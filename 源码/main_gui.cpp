// EasyX 图形界面入口
// 编译前请先安装 EasyX 图形库：https://easyx.cn

#include "gui_dynamic.h"
#include "gui_paging.h"
#include "gui_helpers.h"

#include <graphics.h>
#include <conio.h>

using namespace Gui;

// 绘制主菜单背景（不含按钮，供 waitClick 每帧调用）
static void drawMainMenuBackground() {
    setbkcolor(COLOR_BG);
    cleardevice();

    setChineseFont(32, true);
    drawText(320, 120, _T("操作系统内存管理模拟器"), COLOR_TITLE);

    setChineseFont(18);
    drawText(380, 170, _T("Operating System Memory Simulator"), COLOR_TEXT);

    drawPanel(120, 230, 440, 160, _T("模块一：动态分区"));
    setChineseFont(15);
    drawText(140, 275, _T("640KB 内存 | First Fit / Best Fit"), COLOR_TEXT);
    drawText(140, 305, _T("可视化内存条 + 分区表动画"), COLOR_TEXT);

    drawPanel(640, 230, 440, 160, _T("模块二：请求分页"));
    drawText(660, 275, _T("320指令 / 32页 / 4页框"), COLOR_TEXT);
    drawText(660, 305, _T("FIFO / LRU 页面置换动画"), COLOR_TEXT);

    setChineseFont(14);
    drawText(400, 560, _T("EasyX 图形界面版  |  按 ESC 退出"), COLOR_TEXT);
}

// 绘制主菜单并等待选择
static int drawMainMenu() {
    Button btns[3];
    btns[0].set(200, 450, 280, 56, _T("动态分区模拟"));
    btns[1].set(520, 450, 280, 56, _T("请求分页模拟"));
    btns[2].set(840, 450, 200, 56, _T("退出"), RGB(149, 165, 166));

    return waitClick(btns, 3, drawMainMenuBackground);
}

int main() {
    initgraph(1200, 800);
    setbkcolor(COLOR_BG);
    cleardevice();
    BeginBatchDraw();

    while (true) {
        int choice = drawMainMenu();

        if (choice == 0) {
            EndBatchDraw();
            runDynamicPartitionGui();
            BeginBatchDraw();
        } else if (choice == 1) {
            EndBatchDraw();
            runPagingGui();
            BeginBatchDraw();
        } else {
            break;  // 退出或 ESC
        }
    }

    EndBatchDraw();
    closegraph();
    return 0;
}
