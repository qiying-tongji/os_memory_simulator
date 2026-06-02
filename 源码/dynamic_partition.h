#ifndef DYNAMIC_PARTITION_H
#define DYNAMIC_PARTITION_H

#include <string>
#include <vector>

// ============================================================
// 第一部分：动态分区内存分配模拟 —— 数据结构
// ============================================================

// 空闲分区：记录一段可用内存的起始地址和大小
struct FreeBlock {
    int start;  // 起始地址（单位：KB）
    int size;   // 分区大小（单位：KB）
};

// 已分配分区：记录某个进程占用的内存
struct AllocatedBlock {
    std::string name;  // 进程名（如 A、B、C）
    int start;         // 起始地址（KB）
    int size;          // 占用大小（KB）
};

// 分配算法类型
enum class AllocAlgorithm {
    FirstFit,  // 首次适应：从头找第一个够大的空闲区
    BestFit    // 最佳适应：找最小的够大的空闲区
};

// 动态分区内存管理器
class DynamicPartitionManager {
public:
    // totalMemory：初始总内存大小，默认 640KB
    explicit DynamicPartitionManager(int totalMemory = 640);

    // 为进程申请内存，成功返回 true（verbose=false 时不输出到控制台）
    bool allocate(const std::string& processName, int size, AllocAlgorithm algo,
                  bool verbose = true);

    // 释放进程占用的内存，成功返回 true
    bool release(const std::string& processName, bool verbose = true);

    // 打印当前内存状态（空闲区 + 已分配区）
    void printState() const;

    // 运行内置演示（A/B/C 申请与释放）
    void runDemo(AllocAlgorithm algo);

    // ---- GUI 接口 ----
    int getTotalMemory() const { return totalMemory_; }
    const std::vector<FreeBlock>& getFreeList() const { return freeList_; }
    const std::vector<AllocatedBlock>& getAllocatedList() const {
        return allocatedList_;
    }
    const std::string& getLastMessage() const { return lastMessage_; }

private:
    int totalMemory_;
    std::vector<FreeBlock> freeList_;
    std::vector<AllocatedBlock> allocatedList_;
    std::string lastMessage_;

    // 释放后合并相邻空闲区
    void mergeFreeBlocks();

    // 按起始地址排序空闲区（便于合并和显示）
    void sortFreeList();
};

#endif
