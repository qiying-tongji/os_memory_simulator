#ifndef PAGING_H
#define PAGING_H

#include <string>
#include <vector>

// ============================================================
// 第二部分：请求分页存储管理模拟 —— 数据结构
// ============================================================

// 基本常量
const int TOTAL_INSTRUCTIONS = 320;  // 作业共 320 条指令
const int PAGE_SIZE = 10;            // 每页 10 条指令
const int TOTAL_PAGES = 32;          // 共 32 页（320 / 10）
const int FRAME_COUNT = 4;           // 内存中只有 4 个页框

// 页面置换算法
enum class ReplaceAlgorithm {
    FIFO,  // 先进先出
    LRU    // 最近最少使用
};

// 页框：内存中的一个物理页框
struct PageFrame {
    int pageNum;     // 当前存放的逻辑页号，-1 表示空闲
    int loadOrder;   // 调入顺序（FIFO 用）
    int lastAccess;  // 最近一次访问的时间戳（LRU 用）
};

// 请求分页管理器
class PagingManager {
public:
    // 生成符合局部性规律的 320 条指令序列
    static std::vector<int> generateInstructions();

    // 运行分页模拟，algo 指定置换算法
    // 若 instructions 非空则使用给定序列（便于算法对比）
    void runSimulation(ReplaceAlgorithm algo,
                       const std::vector<int>& instructions = {});

    // ---- GUI 单步模拟接口 ----
    void reset(const std::vector<int>& instructions = {});
    bool step(ReplaceAlgorithm algo);  // 执行一步，返回是否还有下一步
    bool isFinished() const { return finished_; }

    int getAccessCount() const { return accessCount_; }
    int getPageFaultCount() const { return pageFaultCount_; }
    int getCurrentStep() const { return currentIndex_; }
    const std::vector<PageFrame>& getFrames() const { return frames_; }

    int getLastInstruction() const { return lastInstruction_; }
    int getLastPageNum() const { return lastPageNum_; }
    bool getLastFault() const { return lastFault_; }
    int getLastVictimFrame() const { return lastVictimFrame_; }

private:
    std::vector<PageFrame> frames_;
    std::vector<int> instructions_;
    int accessCount_;
    int pageFaultCount_;
    int timeStamp_;
    int currentIndex_;
    bool finished_;

    int lastInstruction_;
    int lastPageNum_;
    bool lastFault_;
    int lastVictimFrame_;

    // 访问一条指令，返回是否缺页
    bool accessInstruction(int instruction, ReplaceAlgorithm algo);

    // 查找页号对应的页框下标，未命中返回 -1
    int findFrame(int pageNum) const;

    // 将页面调入内存（必要时触发置换），返回被淘汰的页框号（-1 表示无淘汰）
    int loadPage(int pageNum, ReplaceAlgorithm algo);

    // 打印当前内存中的页面
    void printFrames() const;

    // 打印分隔线
    static void printSeparator();
};

#endif
