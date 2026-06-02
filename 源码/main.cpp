#include "dynamic_partition.h"
#include "paging.h"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>

using namespace std;

// 清空输入缓冲区
void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// 读取整数输入
int readInt(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) return value;
        cout << "输入无效，请输入整数。\n";
        clearInput();
    }
}

// ============================================================
// 动态分区模块 — 交互菜单
// ============================================================
void dynamicPartitionMenu() {
    while (true) {
        cout << "\n╔══════════════════════════════════════╗\n";
        cout << "║     动态分区内存分配模拟              ║\n";
        cout << "╠══════════════════════════════════════╣\n";
        cout << "║  1. 首次适应算法演示                  ║\n";
        cout << "║  2. 最佳适应算法演示                  ║\n";
        cout << "║  3. 手动操作（自定义申请/释放）       ║\n";
        cout << "║  0. 返回主菜单                        ║\n";
        cout << "╚══════════════════════════════════════╝\n";
        cout << "请选择: ";

        int choice;
        if (!(cin >> choice)) {
            clearInput();
            continue;
        }

        if (choice == 0) return;

        if (choice == 1) {
            DynamicPartitionManager mgr(640);
            mgr.runDemo(AllocAlgorithm::FirstFit);
        } else if (choice == 2) {
            DynamicPartitionManager mgr(640);
            mgr.runDemo(AllocAlgorithm::BestFit);
        } else if (choice == 3) {
            cout << "\n选择分配算法: 1=首次适应  2=最佳适应\n";
            int algoChoice = readInt("请输入: ");
            AllocAlgorithm algo = (algoChoice == 2) ? AllocAlgorithm::BestFit
                                                     : AllocAlgorithm::FirstFit;

            DynamicPartitionManager mgr(640);
            mgr.printState();

            while (true) {
                cout << "\n  1. 申请内存  2. 释放内存  0. 结束\n";
                int op = readInt("请选择操作: ");
                if (op == 0) break;

                if (op == 1) {
                    string name;
                    cout << "进程名: ";
                    cin >> name;
                    int size = readInt("申请大小(KB): ");
                    mgr.allocate(name, size, algo);
                } else if (op == 2) {
                    string name;
                    cout << "进程名: ";
                    cin >> name;
                    mgr.release(name);
                }
            }
        } else {
            cout << "无效选项。\n";
        }

        cout << "\n按回车继续...";
        clearInput();
        cin.get();
    }
}

// ============================================================
// 请求分页模块 — 交互菜单
// ============================================================
void pagingMenu() {
    while (true) {
        cout << "\n╔══════════════════════════════════════╗\n";
        cout << "║     请求分页存储管理模拟              ║\n";
        cout << "╠══════════════════════════════════════╣\n";
        cout << "║  1. FIFO 页面置换演示                 ║\n";
        cout << "║  2. LRU  页面置换演示                 ║\n";
        cout << "║  3. 对比 FIFO 与 LRU                  ║\n";
        cout << "║  0. 返回主菜单                        ║\n";
        cout << "╚══════════════════════════════════════╝\n";
        cout << "请选择: ";

        int choice;
        if (!(cin >> choice)) {
            clearInput();
            continue;
        }

        if (choice == 0) return;

        if (choice == 1) {
            PagingManager mgr;
            mgr.runSimulation(ReplaceAlgorithm::FIFO);
        } else if (choice == 2) {
            PagingManager mgr;
            mgr.runSimulation(ReplaceAlgorithm::LRU);
        } else if (choice == 3) {
            cout << "\n>>> 使用相同指令序列对比两种算法 <<<\n";
            srand(42);
            vector<int> instructions = PagingManager::generateInstructions();

            PagingManager mgrFifo;
            mgrFifo.runSimulation(ReplaceAlgorithm::FIFO, instructions);

            PagingManager mgrLru;
            mgrLru.runSimulation(ReplaceAlgorithm::LRU, instructions);
        } else {
            cout << "无效选项。\n";
        }

        cout << "\n按回车继续...";
        clearInput();
        cin.get();
    }
}

// ============================================================
// 主函数
// ============================================================
int main() {
    // Windows 控制台 UTF-8 支持
#ifdef _WIN32
    system("chcp 65001 > nul");
#endif

    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║   操作系统内存管理模拟器                      ║\n";
    cout << "║   Operating System Memory Simulator           ║\n";
    cout << "╠══════════════════════════════════════════════╣\n";
    cout << "║  模块一：动态分区内存分配（640KB）            ║\n";
    cout << "║  模块二：请求分页存储管理（320指令/4页框）    ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";

    while (true) {
        cout << "\n【主菜单】\n";
        cout << "  1. 动态分区内存分配\n";
        cout << "  2. 请求分页存储管理\n";
        cout << "  0. 退出\n";
        cout << "请选择: ";

        int choice;
        if (!(cin >> choice)) {
            clearInput();
            continue;
        }

        if (choice == 0) {
            cout << "再见！\n";
            break;
        } else if (choice == 1) {
            dynamicPartitionMenu();
        } else if (choice == 2) {
            pagingMenu();
        } else {
            cout << "无效选项，请重新输入。\n";
        }
    }

    return 0;
}
