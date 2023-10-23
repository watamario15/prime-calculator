#ifndef MSG_HPP_
#define MSG_HPP_

#include "main.hpp"

namespace msg {
void onActivate(HWND hWnd, unsigned int state, HWND hWndActDeact, BOOL fMinimized);
BOOL onCreate(HWND hWnd, CREATESTRUCTW *lpCreateStruct);
void onClose(HWND hWnd);
void onTimer(HWND hWnd, unsigned int id);
void onSize(HWND hWnd, unsigned int state, int cx, int cy);
void onPaint(HWND hWnd);
void onCommand(HWND hWnd, int id, HWND hWndCtl, unsigned int codeNotify);
void Paint(HWND hWnd);
}  // namespace msg

#endif
