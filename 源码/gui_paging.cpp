#include "gui_paging.h"
#include "paging.h"
#include "gui_helpers.h"

#include <graphics.h>
#include <algorithm>
#include <cstdio>
#include <string>

using namespace Gui;

// 绘制 4 个页框
static void drawFrames(const PagingManager& mgr, int fx, int fy,
                        int highlightFrame = -1, bool faultFlash = false) {
    const auto& frames = mgr.getFrames();

    for (int i = 0; i < FRAME_COUNT; i++) {
        int x = fx + i * 200;
        int y = fy;

        bool isHighlight = (i == highlightFrame);
        bool isHit = (!mgr.getLastFault() && frames[i].pageNum == mgr.getLastPageNum());

        COLORREF border = COLOR_BORDER;
        if (isHighlight && faultFlash) border = COLOR_DANGER;
        else if (isHit) border = COLOR_SUCCESS;
        else if (isHighlight) border = COLOR_WARNING;

        setlinecolor(border);
        setlinestyle(PS_SOLID, isHighlight || isHit ? 4 : 2);

        if (frames[i].pageNum == -1) {
            setfillcolor(RGB(250, 250, 250));
        } else {
            setfillcolor(getProcessColor(frames[i].pageNum));
        }
        fillroundrect(x, y, x + 180, y + 120, 10, 10);
        roundrect(x, y, x + 180, y + 120, 10, 10);
        setlinestyle(PS_SOLID, 2);

        setChineseFont(16, true);
        TCHAR title[32];
        _stprintf_s(title, _T("页框 %d"), i);
        drawText(x + 50, y + 15, title, COLOR_TITLE);

        setChineseFont(28, true);
        if (frames[i].pageNum == -1) {
            drawText(x + 60, y + 55, _T("空"), COLOR_TEXT);
        } else {
            TCHAR pg[16];
            _stprintf_s(pg, _T("页 %d"), frames[i].pageNum);
            drawText(x + 45, y + 50, pg, WHITE);
        }
    }
}

// 绘制 32 页逻辑地址空间
static void drawPageMap(const PagingManager& mgr, int mx, int my) {
    int currentPage = mgr.getLastPageNum();

    for (int p = 0; p < TOTAL_PAGES; p++) {
        int col = p % 16;
        int row = p / 16;
        int x = mx + col * 42;
        int y = my + row * 42;

        bool inMemory = false;
        for (const auto& f : mgr.getFrames()) {
            if (f.pageNum == p) { inMemory = true; break; }
        }

        if (p == currentPage) {
            setfillcolor(mgr.getLastFault() ? COLOR_DANGER : COLOR_SUCCESS);
        } else if (inMemory) {
            setfillcolor(getProcessColor(p));
        } else {
            setfillcolor(RGB(230, 230, 230));
        }

        setlinecolor(COLOR_BORDER);
        fillrectangle(x, y, x + 36, y + 36);

        setChineseFont(12);
        TCHAR num[8];
        _stprintf_s(num, _T("%d"), p);
        drawText(x + 8, y + 10, num, p == currentPage ? WHITE : COLOR_TEXT);
    }
}

// 绘制进度条
static void drawProgress(int step, int total, int px, int py, int pw) {
    setfillcolor(RGB(220, 220, 220));
    solidroundrect(px, py, px + pw, py + 20, 10, 10);

    int fillW = (step * pw) / total;
    if (fillW > 0) {
        setfillcolor(COLOR_ACCENT);
        solidroundrect(px, py, px + fillW, py + 20, 10, 10);
    }

    setChineseFont(12);
    TCHAR buf[48];
    _stprintf_s(buf, _T("%d / %d"), step, total);
    drawText(px + pw + 10, py + 2, buf, COLOR_TEXT);
}

// 绘制操作日志
static void drawLogPanel(const std::vector<std::string>& logs, int lx, int ly, int lw, int lh) {
    drawPanel(lx, ly, lw, lh, _T("操作日志"));
    setChineseFont(12);
    int y = ly + 35;
    int maxLines = (lh - 40) / 20;
    int startIdx = max(0, (int)logs.size() - maxLines);
    
    for (int i = startIdx; i < (int)logs.size(); i++) {
        // 使用 MultiByteToWideChar 正确转换 UTF-8 到宽字符
        int len = MultiByteToWideChar(CP_UTF8, 0, logs[i].c_str(), -1, NULL, 0);
        if (len > 0) {
            std::vector<wchar_t> ws(len);
            MultiByteToWideChar(CP_UTF8, 0, logs[i].c_str(), -1, ws.data(), len);
            // 截断过长的文本以适应面板宽度
            if (len > 35) {
                ws[32] = L'.';
                ws[33] = L'.';
                ws[34] = L'.';
                ws[35] = L'\0';
            }
            drawText(lx + 10, y, ws.data(), COLOR_TEXT);
        }
        y += 20;
    }
}

// 绘制完整场景
static void drawPagingScene(const PagingManager& mgr, ReplaceAlgorithm algo,
                             const TCHAR* statusMsg, bool faultFlash = false,
                             const std::vector<std::string>& logs = {}) {
    setbkcolor(COLOR_BG);
    cleardevice();

    setChineseFont(26, true);
    drawText(40, 20, _T("请求分页存储管理模拟"), COLOR_TITLE);

    setChineseFont(16);
    const TCHAR* algoName = (algo == ReplaceAlgorithm::FIFO)
                                ? _T("FIFO 先进先出") : _T("LRU 最近最少使用");
    drawText(40, 55, algoName, COLOR_ACCENT);

    // 统计信息
    drawPanel(900, 15, 260, 90, _T("统计"));
    setChineseFont(15);
    TCHAR stat[80];
    _stprintf_s(stat, _T("访问: %d"), mgr.getAccessCount());
    drawText(920, 48, stat, COLOR_TEXT);
    _stprintf_s(stat, _T("缺页: %d"), mgr.getPageFaultCount());
    drawText(1020, 48, stat, COLOR_DANGER);

    if (mgr.getAccessCount() > 0) {
        double rate = 100.0 * mgr.getPageFaultCount() / mgr.getAccessCount();
        _stprintf_s(stat, _T("缺页率: %.1f%%"), rate);
        drawText(920, 72, stat, COLOR_DANGER);
    }

    // 页框
    drawPanel(40, 100, 840, 180, _T("物理内存（4 个页框）"));
    int hlFrame = mgr.getLastFault() ? mgr.getLastVictimFrame()
                                      : mgr.getFrames().empty() ? -1 : 0;
    if (!mgr.getLastFault() && mgr.getLastPageNum() >= 0) {
        for (int i = 0; i < FRAME_COUNT; i++) {
            if (mgr.getFrames()[i].pageNum == mgr.getLastPageNum()) {
                hlFrame = i;
                break;
            }
        }
    }
    drawFrames(mgr, 70, 145, hlFrame, faultFlash);

    // 逻辑页映射
    drawPanel(40, 300, 840, 130, _T("逻辑地址空间（32 页）"));
    drawPageMap(mgr, 60, 340);

    // 当前访问信息
    drawPanel(40, 450, 840, 100, _T("当前访问"));
    setChineseFont(16);
    if (mgr.getLastInstruction() >= 0) {
        TCHAR info[128];
        _stprintf_s(info, _T("访问指令: %d    页号: %d"),
                    mgr.getLastInstruction(), mgr.getLastPageNum());
        drawText(60, 490, info, COLOR_TEXT);

        if (mgr.getLastFault()) {
            drawText(60, 515, _T(">> 缺页中断！正在调入页面..."), COLOR_DANGER);
            if (mgr.getLastVictimFrame() >= 0) {
                _stprintf_s(info, _T("    淘汰页框 %d"), mgr.getLastVictimFrame());
                drawText(350, 515, info, COLOR_WARNING);
            }
        } else {
            drawText(60, 515, _T(">> 页面命中"), COLOR_SUCCESS);
        }
    } else {
        drawText(60, 500, statusMsg, COLOR_TEXT);
    }

    // 进度
    drawPanel(40, 570, 840, 60, _T("执行进度"));
    drawProgress(mgr.getCurrentStep(), TOTAL_INSTRUCTIONS, 60, 605, 700);

    // 操作日志（右侧面板）
    drawLogPanel(logs, 900, 120, 280, 510);
}

bool runPagingGui() {
    BeginBatchDraw();

    // 算法选择
    Button algoBtns[3];
    algoBtns[0].set(300, 300, 200, 50, _T("FIFO"));
    algoBtns[1].set(520, 300, 200, 50, _T("LRU"));
    algoBtns[2].set(740, 300, 200, 50, _T("返回"), RGB(149, 165, 166));

    auto drawAlgoSelect = []() {
        setbkcolor(COLOR_BG);
        cleardevice();
        setChineseFont(28, true);
        drawText(400, 200, _T("选择置换算法"), COLOR_TITLE);
    };

    int choice = waitClick(algoBtns, 3, drawAlgoSelect);
    if (choice < 0 || choice == 2) {
        EndBatchDraw();
        return true;
    }

    ReplaceAlgorithm algo = (choice == 0) ? ReplaceAlgorithm::FIFO
                                           : ReplaceAlgorithm::LRU;

    PagingManager mgr;
    mgr.reset();

    bool autoPlaying = false;
    bool paused = false;
    int speedMs = 150;
    TCHAR statusMsg[128];
    _tcscpy_s(statusMsg, _T("就绪。点击「自动播放」开始动画演示。"));
    
    std::vector<std::string> logs;
    auto addLog = [&](const std::string& msg) {
        logs.push_back(msg);
        if (logs.size() > 20) logs.erase(logs.begin());
    };
    addLog("模拟器已就绪");

    const int barY = 700;
    Button btns[6];
    btns[0].set(40, barY, 120, 44, _T("自动播放"));
    btns[1].set(180, barY, 120, 44, _T("单步"));
    btns[2].set(320, barY, 120, 44, _T("重置"));
    btns[3].set(460, barY, 120, 44, _T("暂停"), COLOR_WARNING);
    btns[3].enabled = false;
    btns[4].set(600, barY, 120, 44, _T("加速"));
    btns[5].set(1000, barY, 140, 44, _T("返回"), RGB(149, 165, 166));

    bool needDraw = true;
    DWORD lastStepTime = GetTickCount();

    auto redrawAll = [&]() {
        drawPagingScene(mgr, algo, statusMsg, false, logs);
        drawButtonBar(barY);
        for (int i = 0; i < 6; i++) btns[i].draw();
        FlushBatchDraw();
        needDraw = false;
    };

    while (true) {
        if (checkMinimizeRestore()) needDraw = true;

        int click = pollInput(btns, 6);

        if (click == 0) {
            if (mgr.isFinished()) {
                mgr.reset();
                logs.clear();
                addLog("模拟已重置");
            }
            autoPlaying = true;
            paused = false;
            btns[3].enabled = true;
            _tcscpy_s(btns[3].label, _T("暂停"));
            _tcscpy_s(statusMsg, _T("自动播放中..."));
            addLog("开始自动播放");
            lastStepTime = GetTickCount();
            needDraw = true;
        } else if (click == 1) {
            autoPlaying = false;
            paused = false;
            btns[3].enabled = false;
            _tcscpy_s(btns[3].label, _T("暂停"));
            if (!mgr.isFinished()) {
                mgr.step(algo);
                char logMsg[128];
                if (mgr.getLastFault()) {
                    sprintf_s(logMsg, "指令%d: 访问页%d - 缺页(淘汰页框%d)",
                              mgr.getLastInstruction(), mgr.getLastPageNum(), mgr.getLastVictimFrame());
                } else {
                    sprintf_s(logMsg, "指令%d: 访问页%d - 命中",
                              mgr.getLastInstruction(), mgr.getLastPageNum());
                }
                addLog(logMsg);
                _tcscpy_s(statusMsg, mgr.getLastFault() ? _T("缺页") : _T("命中"));
            }
            needDraw = true;
        } else if (click == 2) {
            mgr.reset();
            logs.clear();
            addLog("模拟已重置");
            autoPlaying = false;
            paused = false;
            btns[3].enabled = false;
            _tcscpy_s(btns[3].label, _T("暂停"));
            _tcscpy_s(statusMsg, _T("已重置。"));
            needDraw = true;
        } else if (click == 3) {
            paused = !paused;
            _tcscpy_s(btns[3].label, paused ? _T("继续") : _T("暂停"));
            _tcscpy_s(statusMsg, paused ? _T("已暂停。") : _T("自动播放中..."));
            addLog(paused ? "已暂停" : "继续播放");
            if (!paused) lastStepTime = GetTickCount();
            needDraw = true;
        } else if (click == 4) {
            speedMs = max(30, speedMs - 50);
            char logMsg[64];
            sprintf_s(logMsg, "加速: %dms/步", speedMs);
            addLog(logMsg);
            needDraw = true;
        } else if (click == 5 || click == -1) {
            EndBatchDraw();
            return true;
        }

        // 定时自动步进
        if (autoPlaying && !paused && !mgr.isFinished()) {
            DWORD now = GetTickCount();
            if (now - lastStepTime >= (DWORD)speedMs) {
                mgr.step(algo);
                char logMsg[128];
                if (mgr.getLastFault()) {
                    sprintf_s(logMsg, "指令%d: 访问页%d - 缺页(淘汰页框%d)",
                              mgr.getLastInstruction(), mgr.getLastPageNum(), mgr.getLastVictimFrame());
                } else {
                    sprintf_s(logMsg, "指令%d: 访问页%d - 命中",
                              mgr.getLastInstruction(), mgr.getLastPageNum());
                }
                addLog(logMsg);
                _tcscpy_s(statusMsg,
                    mgr.getLastFault() ? _T("缺页中断") : _T("页面命中"));
                lastStepTime = now;
                needDraw = true;

                if (mgr.isFinished()) {
                    autoPlaying = false;
                    btns[3].enabled = false;
                    double rate = 100.0 * mgr.getPageFaultCount() / mgr.getAccessCount();
                    char finishMsg[128];
                    sprintf_s(finishMsg, "模拟完成！缺页率: %.1f%%", rate);
                    addLog(finishMsg);
                    _stprintf_s(statusMsg, _T("模拟完成！缺页率: %.1f%%"), rate);
                }
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
