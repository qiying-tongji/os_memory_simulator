#ifndef GUI_HELPERS_H
#define GUI_HELPERS_H

#include <graphics.h>
#include <tchar.h>
#include <windows.h>
#include <string>

// ============================================================
// EasyX GUI 公共工具：颜色、字体、按钮、面板
// ============================================================

namespace Gui {

const COLORREF COLOR_BG       = RGB(245, 247, 250);
const COLORREF COLOR_PANEL    = RGB(255, 255, 255);
const COLORREF COLOR_BORDER   = RGB(200, 210, 220);
const COLORREF COLOR_TITLE    = RGB(44, 62, 80);
const COLORREF COLOR_TEXT     = RGB(52, 73, 94);
const COLORREF COLOR_ACCENT   = RGB(52, 152, 219);
const COLORREF COLOR_SUCCESS  = RGB(46, 204, 113);
const COLORREF COLOR_DANGER   = RGB(231, 76, 60);
const COLORREF COLOR_WARNING  = RGB(241, 196, 15);
const COLORREF COLOR_FREE     = RGB(236, 240, 241);

inline COLORREF getProcessColor(int index) {
    static const COLORREF palette[] = {
        RGB(231, 76, 60),  RGB(52, 152, 219), RGB(46, 204, 113),
        RGB(155, 89, 182), RGB(241, 196, 15), RGB(230, 126, 34),
        RGB(26, 188, 156), RGB(149, 165, 166)
    };
    return palette[index % 8];
}

inline void setChineseFont(int height, bool bold = false) {
    LOGFONT font = {0};
    font.lfHeight = -height;
    font.lfWeight = bold ? FW_BOLD : FW_NORMAL;
    _tcscpy_s(font.lfFaceName, _T("微软雅黑"));
    settextstyle(&font);
}

inline void drawText(int x, int y, const TCHAR* text, COLORREF color = COLOR_TEXT) {
    settextcolor(color);
    setbkmode(TRANSPARENT);
    outtextxy(x, y, text);
}

inline std::wstring toWide(const std::string& text) {
    if (text.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    if (len <= 0) return L"";
    std::wstring ws(len - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &ws[0], len);
    return ws;
}

inline void drawTextA(int x, int y, const std::string& text, COLORREF color = COLOR_TEXT) {
    std::wstring ws = toWide(text);
    settextcolor(color);
    setbkmode(TRANSPARENT);
    outtextxy(x, y, ws.c_str());
}

inline void drawPanel(int x, int y, int w, int h, const TCHAR* title = nullptr) {
    setfillcolor(COLOR_PANEL);
    setlinecolor(COLOR_BORDER);
    setlinestyle(PS_SOLID, 2);
    fillroundrect(x, y, x + w, y + h, 12, 12);
    roundrect(x, y, x + w, y + h, 12, 12);
    if (title) {
        setChineseFont(18, true);
        drawText(x + 16, y + 12, title, COLOR_TITLE);
    }
}

struct Button {
    int x, y, w, h;
    TCHAR label[48];
    COLORREF bg;
    COLORREF fg;
    bool enabled;

    Button() : x(0), y(0), w(0), h(0), bg(COLOR_ACCENT), fg(WHITE), enabled(true) {
        label[0] = _T('\0');
    }

    void set(int px, int py, int pw, int ph, const TCHAR* text,
             COLORREF background = COLOR_ACCENT) {
        x = px; y = py; w = pw; h = ph;
        _tcscpy_s(label, text);
        bg = background;
    }

    bool contains(int mx, int my) const {
        return mx >= x && mx <= x + w && my >= y && my <= y + h;
    }

    void draw(bool hover = false) const {
        COLORREF fill = enabled ? (hover ? RGB(
            GetRValue(bg) + 20, GetGValue(bg) + 20, GetBValue(bg) + 20) : bg)
                                : RGB(180, 180, 180);
        setfillcolor(fill);
        setlinecolor(COLOR_BORDER);
        fillroundrect(x, y, x + w, y + h, 8, 8);
        roundrect(x, y, x + w, y + h, 8, 8);
        setChineseFont(16);
        settextcolor(enabled ? fg : RGB(120, 120, 120));
        setbkmode(TRANSPARENT);
        int tw = textwidth(label);
        int th = textheight(label);
        outtextxy(x + (w - tw) / 2, y + (h - th) / 2, label);
    }
};

inline int hitButton(Button* buttons, int count, int mx, int my) {
    for (int i = 0; i < count; i++) {
        if (buttons[i].enabled && buttons[i].contains(mx, my)) return i;
    }
    return -1;
}

// 绘制底部按钮栏背景
inline void drawButtonBar(int y, int height = 70) {
    setfillcolor(RGB(235, 238, 242));
    setlinecolor(COLOR_BORDER);
    solidrectangle(0, y - 10, getwidth(), getheight());
    line(0, y - 10, getwidth(), y - 10);
}

typedef void (*RedrawCallback)();

// 检测是否从最小化恢复，若是则重建双缓冲
inline bool checkMinimizeRestore() {
    static bool wasMinimized = false;
    HWND hwnd = GetHWnd();
    bool minimized = (hwnd && IsIconic(hwnd));
    bool restored = wasMinimized && !minimized;
    wasMinimized = minimized;
    if (restored) {
        EndBatchDraw();
        BeginBatchDraw();
    }
    return restored;
}

// 阻塞等待按钮点击（用于菜单）
inline int waitClick(Button* buttons, int count, RedrawCallback onRedraw = nullptr) {
    int hover = -1;
    bool needDraw = true;

    while (true) {
        if (checkMinimizeRestore()) needDraw = true;

        ExMessage msg;
        while (peekmessage(&msg, EX_MOUSE | EX_KEY)) {
            if (msg.message == WM_MOUSEMOVE) {
                int h = hitButton(buttons, count, msg.x, msg.y);
                if (h != hover) { hover = h; needDraw = true; }
            }
            if (msg.message == WM_LBUTTONDOWN) {
                int idx = hitButton(buttons, count, msg.x, msg.y);
                if (idx >= 0) return idx;
            }
            if (msg.message == WM_KEYDOWN && msg.vkcode == VK_ESCAPE) return -1;
        }

        if (needDraw && !(GetHWnd() && IsIconic(GetHWnd()))) {
            if (onRedraw) onRedraw();
            for (int i = 0; i < count; i++) buttons[i].draw(i == hover);
            FlushBatchDraw();
            needDraw = false;
        }
        Sleep(10);
    }
}

// 非阻塞读取一次输入，与 waitClick 使用相同的消息处理方式
// 返回：>=0 按钮下标，-1 ESC，-2 无输入
inline int pollInput(Button* buttons, int count) {
    int click = -2;
    ExMessage msg;
    while (peekmessage(&msg, EX_MOUSE | EX_KEY)) {
        if (msg.message == WM_LBUTTONDOWN) {
            int idx = hitButton(buttons, count, msg.x, msg.y);
            if (idx >= 0) click = idx;
        }
        if (msg.message == WM_KEYDOWN && msg.vkcode == VK_ESCAPE) return -1;
    }
    return click;
}

}  // namespace Gui

#endif
