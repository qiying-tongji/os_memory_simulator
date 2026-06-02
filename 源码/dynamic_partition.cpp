#include "dynamic_partition.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

// ============================================================
// 构造函数：初始化一块完整的空闲内存
// ============================================================
DynamicPartitionManager::DynamicPartitionManager(int totalMemory)
    : totalMemory_(totalMemory) {
    freeList_.push_back({0, totalMemory_});
}

// ============================================================
// 内存申请
// ============================================================
bool DynamicPartitionManager::allocate(const string& processName, int size,
                                       AllocAlgorithm algo, bool verbose) {
    lastMessage_.clear();

    for (const auto& block : allocatedList_) {
        if (block.name == processName) {
            lastMessage_ = "进程 " + processName + " 已存在";
            if (verbose) cout << "[错误] " << lastMessage_ << "\n";
            return false;
        }
    }

    if (size <= 0) {
        lastMessage_ = "申请大小必须大于 0";
        if (verbose) cout << "[错误] " << lastMessage_ << "\n";
        return false;
    }

    int bestIndex = -1;  // 选中的空闲区下标

    if (algo == AllocAlgorithm::FirstFit) {
        // 首次适应：从头遍历，找到第一个够大的空闲区
        for (int i = 0; i < (int)freeList_.size(); i++) {
            if (freeList_[i].size >= size) {
                bestIndex = i;
                break;
            }
        }
    } else {
        // 最佳适应：找最小的、但仍够大的空闲区
        int minWaste = totalMemory_ + 1;
        for (int i = 0; i < (int)freeList_.size(); i++) {
            if (freeList_[i].size >= size) {
                int waste = freeList_[i].size - size;
                if (waste < minWaste) {
                    minWaste = waste;
                    bestIndex = i;
                }
            }
        }
    }

    if (bestIndex == -1) {
        lastMessage_ = "进程 " + processName + " 申请 " + to_string(size) + "KB 失败：内存不足";
        if (verbose) cout << "[失败] " << lastMessage_ << "\n";
        return false;
    }

    // 从选中的空闲区中划分出所需大小
    FreeBlock& chosen = freeList_[bestIndex];
    int allocStart = chosen.start;

    allocatedList_.push_back({processName, allocStart, size});

    if (chosen.size == size) {
        // 整块分配，移除该空闲区
        freeList_.erase(freeList_.begin() + bestIndex);
    } else {
        // 部分分配，缩小空闲区
        chosen.start += size;
        chosen.size -= size;
    }

    string algoName = (algo == AllocAlgorithm::FirstFit) ? "首次适应" : "最佳适应";
    lastMessage_ = "进程 " + processName + " 申请 " + to_string(size) + "KB 成功（" + algoName + "）";
    if (verbose) {
        cout << "\n>>> " << lastMessage_ << "\n";
        printState();
    }
    return true;
}

bool DynamicPartitionManager::release(const string& processName, bool verbose) {
    lastMessage_.clear();
    int foundIndex = -1;
    for (int i = 0; i < (int)allocatedList_.size(); i++) {
        if (allocatedList_[i].name == processName) {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex == -1) {
        lastMessage_ = "进程 " + processName + " 不存在";
        if (verbose) cout << "[错误] " << lastMessage_ << "\n";
        return false;
    }

    AllocatedBlock freed = allocatedList_[foundIndex];
    allocatedList_.erase(allocatedList_.begin() + foundIndex);

    // 将释放的区域加入空闲表
    freeList_.push_back({freed.start, freed.size});

    // 合并相邻空闲区
    mergeFreeBlocks();

    lastMessage_ = "进程 " + processName + " 释放 " + to_string(freed.size) + "KB 成功";
    if (verbose) {
        cout << "\n>>> " << lastMessage_ << "\n";
        printState();
    }
    return true;
}

// ============================================================
// 合并相邻空闲区
// ============================================================
void DynamicPartitionManager::mergeFreeBlocks() {
    if (freeList_.size() <= 1) return;

    sortFreeList();

    vector<FreeBlock> merged;
    merged.push_back(freeList_[0]);

    for (int i = 1; i < (int)freeList_.size(); i++) {
        FreeBlock& last = merged.back();
        FreeBlock& curr = freeList_[i];

        // 如果当前块紧接在上一块之后，则合并
        if (last.start + last.size == curr.start) {
            last.size += curr.size;
        } else {
            merged.push_back(curr);
        }
    }

    freeList_ = merged;
}

void DynamicPartitionManager::sortFreeList() {
    sort(freeList_.begin(), freeList_.end(),
         [](const FreeBlock& a, const FreeBlock& b) {
             return a.start < b.start;
         });
}

// ============================================================
// 打印当前内存状态
// ============================================================
void DynamicPartitionManager::printState() const {
    cout << "\n";
    cout << "┌─────────────────────────────────────────┐\n";
    cout << "│           当前内存状态                    │\n";
    cout << "├─────────────────────────────────────────┤\n";

    // 已分配分区
    cout << "│ 【已分配分区】                          │\n";
    if (allocatedList_.empty()) {
        cout << "│   （无）                                │\n";
    } else {
        cout << "│   进程名    起始地址    大小            │\n";
        for (const auto& block : allocatedList_) {
            cout << "│   " << setw(6) << left << block.name
                 << setw(10) << right << block.start << "KB"
                 << setw(10) << right << block.size << "KB"
                 << "          │\n";
        }
    }

    cout << "├─────────────────────────────────────────┤\n";

    // 空闲分区
    cout << "│ 【空闲分区】                            │\n";
    cout << "│   起始地址    大小                      │\n";
    for (const auto& block : freeList_) {
        cout << "│   " << setw(10) << right << block.start << "KB"
             << setw(10) << right << block.size << "KB"
             << "            │\n";
    }

    // 内存可视化条
    cout << "├─────────────────────────────────────────┤\n";
    cout << "│ 【内存条可视化】 总容量: " << totalMemory_ << "KB       │\n";
    cout << "│ 0";
    // 合并所有块按地址排序后画条
    vector<pair<int, int>> segments;  // start, size
    vector<pair<int, string>> labels; // start, name
    for (const auto& b : allocatedList_) {
        segments.push_back({b.start, b.size});
        labels.push_back({b.start, b.name});
    }
    for (const auto& b : freeList_) {
        segments.push_back({b.start, b.size});
        labels.push_back({b.start, "空"});
    }
    sort(segments.begin(), segments.end());
    sort(labels.begin(), labels.end());

    string bar;
    for (const auto& seg : segments) {
        int chars = max(1, seg.second * 40 / totalMemory_);
        bool isFree = true;
        string label = "空";
        for (const auto& b : allocatedList_) {
            if (b.start == seg.first) {
                isFree = false;
                label = b.name;
                break;
            }
        }
        char ch = isFree ? '.' : label[0];
        for (int i = 0; i < chars; i++) bar += ch;
    }
    cout << " [" << bar << "] " << totalMemory_ << "KB\n";
    cout << "└─────────────────────────────────────────┘\n";
}

// ============================================================
// 内置演示流程
// ============================================================
void DynamicPartitionManager::runDemo(AllocAlgorithm algo) {
    string algoName = (algo == AllocAlgorithm::FirstFit) ? "首次适应 (First Fit)"
                                                         : "最佳适应 (Best Fit)";
    cout << "\n========================================\n";
    cout << "  动态分区内存分配演示 — " << algoName << "\n";
    cout << "  初始内存: " << totalMemory_ << "KB\n";
    cout << "========================================\n";
    printState();

    allocate("A", 130, algo);
    allocate("B", 60, algo);
    allocate("C", 100, algo);
    release("A");
    allocate("D", 50, algo);
    release("B");
    allocate("E", 300, algo);
    allocate("F", 40, algo);

    cout << "\n========== 演示结束 ==========\n";
}
