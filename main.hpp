#ifndef MAIN_HPP_
#define MAIN_HPP_

#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef UNICODE
#define UNICODE
#endif

// Microsoft provided min/max macros conflict with C++ STL and even some Windows SDKs lack them.
// So we disable them here and redefine ourselves.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define mymin(a, b) (((a) < (b)) ? (a) : (b))
#define mymax(a, b) (((a) > (b)) ? (a) : (b))

#include <windows.h>

#ifdef UNDER_CE
#include <commctrl.h>
#include <commdlg.h>
#else
#include <process.h>
#endif

#include <errno.h>
#include <wchar.h>

#include "resource.h"
#include "util.hpp"

// Workaround for wrong macro definitions in CeGCC.
#ifdef UNDER_CE
#if SW_MAXIMIZE != 12
#undef SW_MAXIMIZE
#define SW_MAXIMIZE 12
#endif
#if SW_MINIMIZE != 6
#undef SW_MINIMIZE
#define SW_MINIMIZE 6
#endif
#if WS_MINIMIZEBOX != 0x00010000L
#undef WS_MINIMIZEBOX
#define WS_MINIMIZEBOX 0x00010000L
#endif
#if WS_MAXIMIZEBOX != 0x00020000L
#undef WS_MAXIMIZEBOX
#define WS_MAXIMIZEBOX 0x00020000L
#endif

// Expands to _beginthreadex() on Windows PC and CreateThread() on Windows CE.
#define myCreateThread(lpsa, cbStack, lpStartAddr, lpvThreadParam, fdwCreate, lpIDThread) \
  CreateThread(lpsa, cbStack, lpStartAddr, lpvThreadParam, fdwCreate, lpIDThread)

// WinMain is already unicode on Windows CE.
#define wWinMain WinMain
#else
// Expands to _beginthreadex() on Windows PC and CreateThread() on Windows CE.
#define myCreateThread(lpsa, cbStack, lpStartAddr, lpvThreadParam, fdwCreate, lpIDThread) \
  (HANDLE) _beginthreadex(lpsa, cbStack, lpStartAddr, lpvThreadParam, fdwCreate, lpIDThread)

// We define this enum manually as old SDKs don't have it.
typedef enum MONITOR_DPI_TYPE {
  MDT_EFFECTIVE_DPI = 0,
  MDT_ANGULAR_DPI = 1,
  MDT_RAW_DPI = 2,
  MDT_DEFAULT = MDT_EFFECTIVE_DPI
} MONITOR_DPI_TYPE;

// The function pointer type for GetDpiForMonitor API.
typedef HRESULT(CALLBACK *GetDpiForMonitor_t)(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, unsigned *dpiX,
                                              unsigned *dpiY);

// The function pointer type for MonitorFromWindow API.
typedef HMONITOR(CALLBACK *MonitorFromWindow_t)(HWND hWnd, DWORD dwFlags);
#endif

// Making SetWindowLongW and GetWindowLongW compatible for both 32-bit and 64-bit system.
#ifdef _WIN64
#define mySetWindowLongW(hWnd, index, data) SetWindowLongPtrW(hWnd, index, (LRESULT)(data))
#define myGetWindowLongW(hWnd, index) GetWindowLongPtrW(hWnd, index)
#ifndef GWL_WNDPROC
#define GWL_WNDPROC GWLP_WNDPROC
#endif
#ifndef GWL_USERDATA
#define GWL_USERDATA GWLP_USERDATA
#endif
#ifndef GWL_STYLE
#define GWL_STYLE GWLP_STYLE
#endif
#else
#define mySetWindowLongW(hWnd, index, data) SetWindowLongW(hWnd, index, (LONG)(data))
#define myGetWindowLongW(hWnd, index) GetWindowLongW(hWnd, index)
#endif

#ifndef WS_OVERLAPPEDWINDOW
#define WS_OVERLAPPEDWINDOW WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX
#endif
#define WM_APP_THREADEND WM_APP

namespace app {
enum mode_t {
  MODE_PF,  // Prime factorization
  MODE_PE   // Prime enumeration
};

extern HWND hWnd, hBtnOK, hBtnAbort, hBtnClear, hEdi0, hEdi1, hEdi2, hEdiOut, hFocused;
extern HINSTANCE hInst;
extern HMENU hMenu;
extern enum mode_t mode;
extern bool useFile, countOnly, isRunning;
extern int charset;
extern wchar_t wcMes[SIZE_OF_STRING_TABLE][MAX_BUFFER];  // String Table receiver
}  // namespace app

#endif
