#ifndef MSG_HPP_
#define MSG_HPP_

#include "main.hpp"

namespace msg {
void onActivate(HWND hWnd, unsigned state, HWND hWndActDeact, BOOL fMinimized);
BOOL onCreate(HWND hWnd, CREATESTRUCTW *lpCreateStruct);
void onClose(HWND hWnd);
void onSize(HWND hWnd, unsigned state, int cx, int cy);
void onPaint(HWND hWnd);
void onCommand(HWND hWnd, int id, HWND hWndCtl, unsigned codeNotify);
void redraw(HWND hWnd);
}  // namespace msg

#endif
