#include "main.hpp"

#include "msg.hpp"
#include "runner.hpp"
#include "wproc.hpp"

#define WND_CLASS_NAME L"prime-main"

namespace app {
HWND hWnd, hBtnOK, hBtnAbort, hBtnClear, hEdi0, hEdi1, hEdi2, hEdiOut, hFocused;
HINSTANCE hInst;
HMENU hMenu;
enum mode_t mode = MODE_PF;
bool useFile = false, countOnly = false, isRunning = false;
int charset = IDM_OPT_CHARSET_UTF8;
wchar_t wcMes[SIZE_OF_STRING_TABLE][MAX_BUFFER];
}  // namespace app

#ifdef BORLAND
extern "C"
#endif
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, wchar_t* lpCmdLine, int nShowCmd) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  app::hInst = hInstance;

  if (GetUserDefaultUILanguage() == 0x0411) {  //  Loads Japanese strings on Japanese platforms
    for (int i = 0; i < SIZE_OF_STRING_TABLE; i++) {
      LoadStringW(hInstance, i + IDS_JA, app::wcMes[i], sizeof(app::wcMes[0]) / sizeof(app::wcMes[0][0]));
    }
    app::hMenu = LoadMenuW(app::hInst, L"ResMenu_JA");
  } else {  // Loads English strings otherwise
    for (int i = 0; i < SIZE_OF_STRING_TABLE; i++) {
      LoadStringW(hInstance, i + IDS_EN, app::wcMes[i], sizeof(app::wcMes[0]) / sizeof(app::wcMes[0][0]));
    }
    app::hMenu = LoadMenuW(app::hInst, L"ResMenu_EN");
  }

  WNDCLASSW wcl;
  wcl.hInstance = hInstance;
  wcl.lpszClassName = WND_CLASS_NAME;
  wcl.lpfnWndProc = wproc::wndProc;
  wcl.style = 0;
  wcl.hIcon = LoadIconW(hInstance, L"ResIcon");
  wcl.hCursor = LoadCursorW(NULL, IDC_ARROW);
  wcl.lpszMenuName = 0;
  wcl.cbClsExtra = 0;
  wcl.cbWndExtra = 0;
  wcl.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
  if (!RegisterClassW(&wcl)) {
    MessageBoxW(NULL, app::wcMes[IDS_EREGCL], app::wcMes[IDS_ERROR], MB_OK | MB_ICONERROR);
    return 1;
  }

  app::hWnd = CreateWindowExW(  // Main window
      0, WND_CLASS_NAME, app::wcMes[IDS_APPNAME],
      WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,  // WS_CLIPCHILDREN prevents a flickering
      CW_USEDEFAULT, CW_USEDEFAULT, 480, 320, NULL, NULL, hInstance, NULL);
  if (!app::hWnd) {
    MessageBoxW(NULL, app::wcMes[IDS_ECRTWND], app::wcMes[IDS_ERROR], MB_OK | MB_ICONERROR);
    return 1;
  }

  ShowWindow(app::hWnd, nShowCmd);
  UpdateWindow(app::hWnd);

  SetTimer(app::hWnd, 1, 16, NULL);  // Used to change the background color

  HACCEL hAccel = LoadAcceleratorsW(hInstance, L"ResAccel");  // Loads keyboard shortcuts

  MSG msg;
  while (GetMessageW(&msg, NULL, 0, 0)) {  // Repeats until `WM_QUIT` is given, which is 0
    if (!TranslateAcceleratorW(app::hWnd, hAccel, &msg)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
  }
  return (int)msg.wParam;
}
