#include "gui_dynamic.h"
#include "dynamic_partition.h"
#include "gui_helpers.h"

#include <graphics.h>
#include <algorithm>
#include <string>
#include <vector>

using namespace std;
using namespace Gui;

// 演示操作序列
struct PartitionOp {
    bool isAlloc;
    string name;
    int size;
};

static vector<PartitionOp> getDemoOps() {
    return {
        {true, "A", 130}, {true, "B", 60}, {true, "C", 100},
        {false, "A", 0},  {true, "D", 50}, {false, "B", 0},
        {true, "E", 300}, {true, "F", 40}
    };
}

// 重置管理器
static void resetManager(DynamicPartitionManager& mgr) {
    mgr = DynamicPartitionManager(640);
}

// 绘制内存条动画
static void drawMemoryBar(const DynamicPartitionManager& mgr,
                           int bx, int by, int bw, int bh,
                           const string& highlightProcess = "") {
    int total = mgr.getTotalMemory();
    const auto& allocated = mgr.getAllocatedList();
    const auto& freeList = mgr.getFreeList();

    // 合并所有段
    struct Seg {
        int start, size;
        bool isFree;
        string name;
    };
    vector<Seg> segs;
    for (const auto& b : allocated)
        segs.push_back({b.start, b.size, false, b.name});
    for (const auto& b : freeList)
        segs.push_back({b.start, b.size, true, ""});
    sort(segs.begin(), segs.end(),
         [](const Seg& a, const Seg& b) { return a.start < b.start; });

    // 外框
    setlinecolor(COLOR_BORDER);
    setlinestyle(PS_SOLID, 2);
    rectangle(bx, by, bx + bw, by + bh);

    // 各分区色块
    int colorIdx = 0;
    for (const auto& seg : segs) {
        int sx = bx + seg.start * bw / total;
        int sw = max(2, seg.size * bw / total);

        if (seg.isFree) {
            setfillcolor(COLOR_FREE);
        } else {
            setfillcolor(getProcessColor(colorIdx++));
        }
        solidrectangle(sx, by + 2, sx + sw, by + bh - 2);

        // 高亮闪烁边框
        if (!seg.isFree && seg.name == highlightProcess) {
            setlinecolor(COLOR_WARNING);
            setlinestyle(PS_SOLID, 3);
            rectangle(sx, by + 1, sx + sw, by + bh - 1);
            setlinestyle(PS_SOLID, 2);
        }

        // 标签
        if (sw > 30) {
            setChineseFont(14, true);
            if (seg.isFree) {
                TCHAR buf[32];
                _stprintf_s(buf, _T("空闲%dKB"), seg.size);
                drawText(sx + 4, by + bh / 2 - 8, buf, COLOR_TEXT);
            } else {
                TCHAR buf[32];
                _stprintf_s(buf, _T("%hs %dKB"), seg.name.c_str(), seg.size);
                drawText(sx + 4, by + bh / 2 - 8, buf, WHITE);
            }
        }
    }

    // 地址刻度
    setChineseFont(12);
    drawText(bx, by + bh + 8, _T("0 KB"), COLOR_TEXT);
    TCHAR endBuf[16];
    _stprintf_s(endBuf, _T("%d KB"), total);
    drawText(bx + bw - 40, by + bh + 8, endBuf, COLOR_TEXT);
}

// 绘制分区表
static void drawTables(const DynamicPartitionManager& mgr,
                        int lx, int ry) {
    const auto& allocated = mgr.getAllocatedList();
    const auto& freeList = mgr.getFreeList();

    // 已分配表
    drawPanel(lx, ry, 360, 40 + (int)allocated.size() * 28 + 20, _T("已分配分区"));
    setChineseFont(14, true);
    drawText(lx + 16, ry + 44, _T("进程    起始    大小"), COLOR_TEXT);
    setChineseFont(14);
    int y = ry + 68;
    for (int i = 0; i < (int)allocated.size(); i++) {
        TCHAR row[64];
        _stprintf_s(row, _T("  %hs      %3dKB   %3dKB"),
                    allocated[i].name.c_str(),
                    allocated[i].start, allocated[i].size);
        drawText(lx + 16, y, row, getProcessColor(i));
        y += 28;
    }
    if (allocated.empty()) {
        drawText(lx + 16, y, _T("  （无）"), COLOR_TEXT);
    }

    // 空闲表
    int fy = ry + 40 + max(80, (int)allocated.size() * 28 + 40);
    drawPanel(lx, fy, 360, 40 + (int)freeList.size() * 28 + 20, _T("空闲分区表"));
    setChineseFont(14, true);
    drawText(lx + 16, fy + 44, _T("起始地址    大小"), COLOR_TEXT);
    setChineseFont(14);
    y = fy + 68;
    for (const auto& b : freeList) {
        TCHAR row[48];
        _stprintf_s(row, _T("  %3dKB       %3dKB"), b.start, b.size);
        drawText(lx + 16, y, row, COLOR_FREE);
        y += 28;
    }
}

// 绘制完整场景
static void drawScene(const DynamicPartitionManager& mgr,
                       AllocAlgorithm algo,
                       const TCHAR* statusMsg,
                       const string& highlight = "",
                       int demoStep = -1, int demoTotal = 0) {
    setbkcolor(COLOR_BG);
    cleardevice();

    // 标题
    setChineseFont(26, true);
    drawText(40, 20, _T("动态分区内存分配模拟"), COLOR_TITLE);
    setChineseFont(16);
    const TCHAR* algoName = (algo == AllocAlgorithm::FirstFit)
                                ? _T("首次适应 First Fit") : _T("最佳适应 Best Fit");
    drawText(40, 55, algoName, COLOR_ACCENT);

    TCHAR info[64];
    _stprintf_s(info, _T("总内存: %d KB"), mgr.getTotalMemory());
    drawText(900, 30, info, COLOR_TEXT);

    if (demoStep >= 0) {
        _stprintf_s(info, _T("演示进度: %d / %d"), demoStep, demoTotal);
        drawText(900, 55, info, COLOR_TEXT);
    }

    // 内存条
    drawPanel(40, 100, 1120, 130, _T("内存可视化"));
    drawMemoryBar(mgr, 60, 145, 1080, 60, highlight);

    // 分区表
    drawTables(mgr, 40, 260);

    // 状态日志
    drawPanel(420, 260, 740, 120, _T("操作日志"));
    setChineseFont(16);
    drawText(436, 300, statusMsg, COLOR_TEXT);

    // 图例
    drawPanel(420, 400, 740, 80, _T("图例"));
    setfillcolor(getProcessColor(0));
    solidrectangle(440, 440, 460, 460);
    drawText(470, 438, _T("已分配"), COLOR_TEXT);
    setfillcolor(COLOR_FREE);
    solidrectangle(560, 440, 580, 460);
    drawText(590, 438, _T("空闲"), COLOR_TEXT);
    setlinecolor(COLOR_WARNING);
    setlinestyle(PS_SOLID, 3);
    rectangle(700, 438, 730, 462);
    drawText(740, 438, _T("当前操作高亮"), COLOR_TEXT);
    setlinestyle(PS_SOLID, 2);
}

// 执行一步演示（单次绘制，不闪烁）
static void playDemoStep(DynamicPartitionManager& mgr, AllocAlgorithm algo,
                          const PartitionOp& op, int step, int total) {
    string highlight = op.isAlloc ? op.name : op.name;
    TCHAR msg[128];

    if (op.isAlloc) {
        mgr.allocate(op.name, op.size, algo, false);
        _stprintf_s(msg, _T(">>> 进程 %hs 申请 %d KB"), op.name.c_str(), op.size);
    } else {
        mgr.release(op.name, false);
        _stprintf_s(msg, _T(">>> 进程 %hs 释放内存"), op.name.c_str());
    }

    drawScene(mgr, algo, msg, highlight, step, total);
}

bool runDynamicPartitionGui() {
  BeginBatchDraw();

  // 算法选择
  Button algoBtns[3];
  algoBtns[0].set(300, 300, 200, 50, _T("首次适应"));
  algoBtns[1].set(520, 300, 200, 50, _T("最佳适应"));
  algoBtns[2].set(740, 300, 200, 50, _T("返回"), RGB(149, 165, 166));

  auto drawAlgoSelect = []() {
    setbkcolor(COLOR_BG);
    cleardevice();
    setChineseFont(28, true);
    drawText(380, 200, _T("选择分配算法"), COLOR_TITLE);
  };

  int choice = waitClick(algoBtns, 3, drawAlgoSelect);
  if (choice < 0 || choice == 2) {
    EndBatchDraw();
    return true;
  }

  AllocAlgorithm algo = (choice == 0) ? AllocAlgorithm::FirstFit
                                       : AllocAlgorithm::BestFit;

  DynamicPartitionManager mgr(640);
  auto demoOps = getDemoOps();
  int demoIndex = 0;
  bool autoPlaying = false;
  TCHAR statusMsg[128];
  _tcscpy_s(statusMsg, _T("就绪。点击「自动演示」或「单步执行」开始。"));

  // 控制按钮（放在底部按钮栏）
  const int barY = 700;
  Button btns[5];
  btns[0].set(40, barY, 140, 44, _T("自动演示"));
  btns[1].set(200, barY, 140, 44, _T("单步执行"));
  btns[2].set(360, barY, 140, 44, _T("重置"));
  btns[3].set(520, barY, 140, 44, _T("暂停"), COLOR_WARNING);
  btns[3].enabled = false;
  btns[4].set(1000, barY, 140, 44, _T("返回"), RGB(149, 165, 166));

  bool needDraw = true;
  DWORD lastAutoTime = GetTickCount();
  const DWORD autoInterval = 900;

  auto redrawAll = [&]() {
    drawScene(mgr, algo, statusMsg);
    drawButtonBar(barY);
    for (int i = 0; i < 5; i++) btns[i].draw();
    FlushBatchDraw();
    needDraw = false;
  };

  while (true) {
    if (checkMinimizeRestore()) needDraw = true;

    int click = pollInput(btns, 5);

    if (click == 0) {
      if (demoIndex >= (int)demoOps.size()) {
        resetManager(mgr);
        demoIndex = 0;
      }
      autoPlaying = true;
      btns[3].enabled = true;
      _tcscpy_s(btns[3].label, _T("暂停"));
      _tcscpy_s(statusMsg, _T("自动演示中..."));
      lastAutoTime = GetTickCount();
      needDraw = true;
    } else if (click == 1) {
      autoPlaying = false;
      btns[3].enabled = false;
      _tcscpy_s(btns[3].label, _T("暂停"));
      if (demoIndex < (int)demoOps.size()) {
        playDemoStep(mgr, algo, demoOps[demoIndex], demoIndex + 1,
                     (int)demoOps.size());
        demoIndex++;
        std::wstring ws = toWide(mgr.getLastMessage());
        _tcscpy_s(statusMsg, ws.c_str());
      } else {
        _tcscpy_s(statusMsg, _T("演示已结束，请点「重置」。"));
      }
      needDraw = true;
    } else if (click == 2) {
      resetManager(mgr);
      demoIndex = 0;
      autoPlaying = false;
      btns[3].enabled = false;
      _tcscpy_s(btns[3].label, _T("暂停"));
      _tcscpy_s(statusMsg, _T("已重置。"));
      needDraw = true;
    } else if (click == 3) {
      autoPlaying = !autoPlaying;
      _tcscpy_s(btns[3].label, autoPlaying ? _T("暂停") : _T("继续"));
      _tcscpy_s(statusMsg, autoPlaying ? _T("自动演示中...") : _T("已暂停。"));
      if (autoPlaying) lastAutoTime = GetTickCount();
      needDraw = true;
    } else if (click == 4 || click == -1) {
      EndBatchDraw();
      return true;
    }

    // 定时自动演示
    if (autoPlaying && demoIndex < (int)demoOps.size()) {
      DWORD now = GetTickCount();
      if (now - lastAutoTime >= autoInterval) {
        playDemoStep(mgr, algo, demoOps[demoIndex], demoIndex + 1,
                     (int)demoOps.size());
        demoIndex++;
        lastAutoTime = now;
        if (demoIndex >= (int)demoOps.size()) {
          autoPlaying = false;
          btns[3].enabled = false;
          _tcscpy_s(btns[3].label, _T("暂停"));
          _tcscpy_s(statusMsg, _T("演示完成！"));
        }
        needDraw = true;
      }
    }

    if (needDraw && !(GetHWnd() && IsIconic(GetHWnd()))) {
      redrawAll();
    }

    Sleep(10);
  }

  EndBatchDraw();
  return true;
}
