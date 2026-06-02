#include "paging.h"

#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>

using namespace std;

// ============================================================
// 生成 320 条指令（模拟程序局部性）
//
// 规律：50% 顺序执行，25% 均匀分布在前地址部分，25% 均匀分布在后地址部分
// 算法步骤：
//   1. 随机生成起始点 m（0~319）
//   2. 顺序执行：m+1
//   3. 跳转到前地址部分 [0, m-1] -> m1，顺序执行 m1+1
//   4. 跳转到后地址部分 [m1+2, 319] -> m2，顺序执行 m2+1
//   5. 重复：跳转前、顺序、跳转后、顺序
//   6. 直到凑满 320 条，避免连续重复
// ============================================================
vector<int> PagingManager::generateInstructions() {
    vector<int> instructions;
    srand(static_cast<unsigned>(time(nullptr)));

    int m = rand() % TOTAL_INSTRUCTIONS;  // 随机起始指令 0~319

    while ((int)instructions.size() < TOTAL_INSTRUCTIONS) {
        // 顺序执行：m+1
        int next = m + 1;
        if (next >= TOTAL_INSTRUCTIONS) next = 0;
        instructions.push_back(next);
        if ((int)instructions.size() >= TOTAL_INSTRUCTIONS) break;
        m = next;

        // 跳转到前地址部分 [0, m-1]
        if (m > 0) {
            int m1 = rand() % m;  // 0 ~ m-1
            instructions.push_back(m1);
            if ((int)instructions.size() >= TOTAL_INSTRUCTIONS) break;

            // 顺序执行：m1+1
            int next1 = m1 + 1;
            if (next1 >= TOTAL_INSTRUCTIONS) next1 = 0;
            instructions.push_back(next1);
            if ((int)instructions.size() >= TOTAL_INSTRUCTIONS) break;
            m = next1;
        }

        // 跳转到后地址部分 [m+1, 319]
        int backStart = m + 1;
        if (backStart < TOTAL_INSTRUCTIONS) {
            int range = TOTAL_INSTRUCTIONS - backStart;
            int m2 = backStart + rand() % range;  // m+1 ~ 319
            instructions.push_back(m2);
            if ((int)instructions.size() >= TOTAL_INSTRUCTIONS) break;

            // 顺序执行：m2+1
            int next2 = m2 + 1;
            if (next2 >= TOTAL_INSTRUCTIONS) next2 = 0;
            instructions.push_back(next2);
            if ((int)instructions.size() >= TOTAL_INSTRUCTIONS) break;
            m = next2;
        } else {
            m = rand() % TOTAL_INSTRUCTIONS;
        }
    }

    // 确保恰好 320 条
    instructions.resize(TOTAL_INSTRUCTIONS);
    return instructions;
}

// ============================================================
// GUI：重置模拟
// ============================================================
void PagingManager::reset(const vector<int>& customInstructions) {
    frames_.resize(FRAME_COUNT);
    for (auto& frame : frames_) {
        frame.pageNum = -1;
        frame.loadOrder = -1;
        frame.lastAccess = -1;
    }

    accessCount_ = 0;
    pageFaultCount_ = 0;
    timeStamp_ = 0;
    currentIndex_ = 0;
    finished_ = false;
    lastInstruction_ = -1;
    lastPageNum_ = -1;
    lastFault_ = false;
    lastVictimFrame_ = -1;

    instructions_ = customInstructions.empty() ? generateInstructions()
                                                : customInstructions;
}

// ============================================================
// GUI：单步执行
// ============================================================
bool PagingManager::step(ReplaceAlgorithm algo) {
    if (finished_ || currentIndex_ >= TOTAL_INSTRUCTIONS) {
        finished_ = true;
        return false;
    }

    lastInstruction_ = instructions_[currentIndex_];
    lastPageNum_ = lastInstruction_ / PAGE_SIZE;
    lastVictimFrame_ = -1;

    accessCount_++;
    timeStamp_++;

    int frameIdx = findFrame(lastPageNum_);
    if (frameIdx != -1) {
        frames_[frameIdx].lastAccess = timeStamp_;
        lastFault_ = false;
    } else {
        pageFaultCount_++;
        lastFault_ = true;
        lastVictimFrame_ = loadPage(lastPageNum_, algo);
    }

    currentIndex_++;
    if (currentIndex_ >= TOTAL_INSTRUCTIONS) {
        finished_ = true;
        return false;
    }
    return true;
}

// ============================================================
// 运行完整分页模拟
// ============================================================
void PagingManager::runSimulation(ReplaceAlgorithm algo,
                                   const vector<int>& customInstructions) {
    string algoName = (algo == ReplaceAlgorithm::FIFO) ? "FIFO（先进先出）"
                                                        : "LRU（最近最少使用）";

    cout << "\n========================================\n";
    cout << "  请求分页存储管理演示 — " << algoName << "\n";
    cout << "  指令总数: " << TOTAL_INSTRUCTIONS
         << "  页大小: " << PAGE_SIZE
         << "  总页数: " << TOTAL_PAGES
         << "  页框数: " << FRAME_COUNT << "\n";
    cout << "========================================\n";

    // 初始化并重置状态
    vector<int> instructions = customInstructions.empty() ? generateInstructions()
                                                           : customInstructions;
    reset(instructions);

    cout << "\n--- 开始模拟（显示前 20 步，之后省略）---\n";

    for (int i = 0; i < TOTAL_INSTRUCTIONS; i++) {
        int instruction = instructions_[i];
        bool fault = accessInstruction(instruction, algo);

        bool showDetail = (i < 20);
        bool showSummary = (i >= 20 && i % 50 == 0);

        if (showDetail || showSummary) {
            int pageNum = instruction / PAGE_SIZE;
            cout << "\n第 " << setw(3) << (i + 1) << " 次访问\n";
            cout << "  访问指令: " << instruction << "\n";
            cout << "  页号: " << pageNum << "\n";

            if (fault) {
                cout << "  >> 缺页！调入页 " << pageNum << "\n";
            } else {
                int frameIdx = findFrame(pageNum);
                int physAddr = frameIdx * PAGE_SIZE + (instruction % PAGE_SIZE);
                cout << "  >> 页面命中，物理地址: " << physAddr << "\n";
            }
            cout << "  当前内存页: ";
            printFrames();
        }
    }

    // 最终统计
    printSeparator();
    cout << "\n╔══════════════════════════════════╗\n";
    cout << "║         模拟结果统计              ║\n";
    cout << "╠══════════════════════════════════╣\n";
    cout << "║  置换算法: " << setw(22) << left << algoName << "║\n";
    cout << "║  总访问次数: " << setw(20) << right << accessCount_ << " ║\n";
    cout << "║  缺页次数:   " << setw(20) << right << pageFaultCount_ << " ║\n";
    double faultRate = (accessCount_ > 0)
                           ? (double)pageFaultCount_ / accessCount_ * 100.0
                           : 0.0;
    cout << "║  缺页率:     " << setw(19) << right << fixed
         << setprecision(2) << faultRate << "% ║\n";
    cout << "╚══════════════════════════════════╝\n";
}

// ============================================================
// 访问一条指令
// ============================================================
bool PagingManager::accessInstruction(int instruction, ReplaceAlgorithm algo) {
    accessCount_++;
    timeStamp_++;

    int pageNum = instruction / PAGE_SIZE;

    int frameIdx = findFrame(pageNum);
    if (frameIdx != -1) {
        // 页面命中，更新 LRU 时间戳
        frames_[frameIdx].lastAccess = timeStamp_;
        return false;
    }

    // 缺页
    pageFaultCount_++;
    return loadPage(pageNum, algo);
}

// ============================================================
// 在页框中查找页号
// ============================================================
int PagingManager::findFrame(int pageNum) const {
    for (int i = 0; i < FRAME_COUNT; i++) {
        if (frames_[i].pageNum == pageNum) {
            return i;
        }
    }
    return -1;
}

// ============================================================
// 将页面调入内存（必要时置换）
// ============================================================
int PagingManager::loadPage(int pageNum, ReplaceAlgorithm algo) {
    for (int i = 0; i < FRAME_COUNT; i++) {
        if (frames_[i].pageNum == -1) {
            frames_[i].pageNum = pageNum;
            frames_[i].loadOrder = timeStamp_;
            frames_[i].lastAccess = timeStamp_;
            return -1;  // 无淘汰
        }
    }

    int victimIdx = 0;

    if (algo == ReplaceAlgorithm::FIFO) {
        int minOrder = frames_[0].loadOrder;
        for (int i = 1; i < FRAME_COUNT; i++) {
            if (frames_[i].loadOrder < minOrder) {
                minOrder = frames_[i].loadOrder;
                victimIdx = i;
            }
        }
    } else {
        int minAccess = frames_[0].lastAccess;
        for (int i = 1; i < FRAME_COUNT; i++) {
            if (frames_[i].lastAccess < minAccess) {
                minAccess = frames_[i].lastAccess;
                victimIdx = i;
            }
        }
    }

    frames_[victimIdx].pageNum = pageNum;
    frames_[victimIdx].loadOrder = timeStamp_;
    frames_[victimIdx].lastAccess = timeStamp_;
    return victimIdx;
}

// ============================================================
// 打印当前内存中的页面
// ============================================================
void PagingManager::printFrames() const {
    cout << "[";
    for (int i = 0; i < FRAME_COUNT; i++) {
        if (i > 0) cout << ", ";
        if (frames_[i].pageNum == -1) {
            cout << "空";
        } else {
            cout << frames_[i].pageNum;
        }
    }
    cout << "]\n";
}

void PagingManager::printSeparator() {
    cout << "\n----------------------------------------\n";
}
