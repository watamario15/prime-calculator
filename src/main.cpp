#include "main.hpp"

#include "msg.hpp"
#include "runner.hpp"
#include "wproc.hpp"

#define WND_CLASS_NAME L"prime-main"

namespace app {
HWND hWnd, hBtnOK, hBtnAbort, hBtnClear, hBtn0, hBtn1, hBtn2, hBtn3, hBtn4, hBtn5, hBtn6, hBtn7, hBtn8, hBtn9, hBtnCE,
    hBtnBS, hEdi0, hEdi1, hEdi2, hEdiOut, hFocused;
HINSTANCE hInst;
HMENU hMenu;
enum mode_t mode = MODE_PF;
bool useFile = false, countOnly = false, isRunning = false;
int charset = IDM_OPT_CHARSET_UTF8;
unsigned short langid = 0x0409;
wchar_t wcMes[SIZE_OF_STRING_TABLE][MAX_BUFFER];
#ifdef UNDER_CE
HWND hCmdBar;
#endif
}  // namespace app

#ifdef __BORLANDC__
extern "C"
#endif
    int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, wchar_t *lpCmdLine, int nShowCmd) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);
  app::hInst = hInstance;

#ifdef UNDER_CE
  wchar_t lang[16];
  if (GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SABBREVLANGNAME, lang, sizeof(lang) / sizeof(lang[0])) >= 3) {
    if (lang[0] == L'J' && lang[1] == L'P' && lang[2] == L'N') app::langid = 0x0411;
  }
#else
  HMODULE kernel32 = LoadLibraryW(L"Kernel32.dll");
  if (kernel32) {
    SetDllDirectoryW_t setDllDirectoryW = (SetDllDirectoryW_t)(void *)GetProcAddress(kernel32, "SetDllDirectoryW");
    if (setDllDirectoryW) setDllDirectoryW(L"");  // DLL hijacking prevention
    FreeLibrary(kernel32);
  }

  app::langid = GetUserDefaultUILanguage();
#endif
  if (app::langid == 0x0411) {  //  Loads Japanese strings on Japanese platforms
    int i;
    for (i = 0; i < SIZE_OF_STRING_TABLE; i++) {
      LoadStringW(hInstance, i + IDS_JA, app::wcMes[i], sizeof(app::wcMes[0]) / sizeof(app::wcMes[0][0]));
    }
  } else {  // Loads English strings otherwise
    int i;
    for (i = 0; i < SIZE_OF_STRING_TABLE; i++) {
      LoadStringW(hInstance, i + IDS_EN, app::wcMes[i], sizeof(app::wcMes[0]) / sizeof(app::wcMes[0][0]));
    }
  }

  WNDCLASSW wcl;
  ZeroMemory(&wcl, sizeof(wcl));
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

  app::hWnd = CreateWindowExW(  // Main window (WS_CLIPCHILDREN prevents flicker)
      0, WND_CLASS_NAME, app::wcMes[IDS_APPNAME], WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT,
      480, 320, NULL, NULL, hInstance, NULL);
  if (!app::hWnd) {
    MessageBoxW(NULL, app::wcMes[IDS_ECRTWND], app::wcMes[IDS_ERROR], MB_OK | MB_ICONERROR);
    return 1;
  }

  ShowWindow(app::hWnd, nShowCmd);
#ifdef UNDER_CE
  ShowWindow(app::hWnd, SW_MAXIMIZE);
#endif
  UpdateWindow(app::hWnd);

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
