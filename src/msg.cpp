#include "msg.hpp"

#include "runner.hpp"
#include "wproc.hpp"

#if defined UNDER_CE && __GNUC__ == 3  // Pocket GCC
#define lstrcpyW wcscpy
#endif

namespace msg {
static HDC hMemDC;  // Handle of a memory device context for double buffering
static HFONT hFmes = NULL, hFbtn = NULL, hFedi = NULL;
static HBRUSH hBshSys, BGDark;
static HPEN hPenSys;
static int buttonX, buttonY, CmdBar_Height = 0;
static struct wproc::editorprops_t edit0Props, edit1Props, edit2Props;

void onActivate(HWND hWnd, unsigned state, HWND hWndActDeact, BOOL fMinimized) {
  UNREFERENCED_PARAMETER(hWnd);
  UNREFERENCED_PARAMETER(hWndActDeact);
  UNREFERENCED_PARAMETER(fMinimized);

  // Re-sets the focus on the most recently focused edit control.
  if (state == WA_ACTIVE || state == WA_CLICKACTIVE) SetFocus(app::hFocused);
}

BOOL onCreate(HWND hWnd, CREATESTRUCTW *lpCreateStruct) {
#ifdef UNDER_CE
  InitCommonControls();
  app::hCmdBar = CommandBar_Create(app::hInst, hWnd, 1);
  if (app::langid == 0x0411) {  // Japanese platforms
    wchar_t tmp[] = L"ResMenu_JA";
    CommandBar_InsertMenubarEx(app::hCmdBar, app::hInst, tmp, 0);
  } else {
    wchar_t tmp[] = L"ResMenu_EN";
    CommandBar_InsertMenubarEx(app::hCmdBar, app::hInst, tmp, 0);
  }
  CommandBar_Show(app::hCmdBar, TRUE);
  CmdBar_Height = CommandBar_Height(app::hCmdBar);
  app::hMenu = CommandBar_GetMenu(app::hCmdBar, 0);
#else
  if (app::langid == 0x0411) {  // Japanese platforms
    app::hMenu = LoadMenuW(app::hInst, L"ResMenu_JA");
  } else {
    app::hMenu = LoadMenuW(app::hInst, L"ResMenu_EN");
  }
  SetMenu(hWnd, app::hMenu);
#endif
  if (app::langid == 0x0411) {  // Japanese platforms
    CheckMenuRadioItem(app::hMenu, IDM_OPT_LANG_JA, IDM_OPT_LANG_EN, IDM_OPT_LANG_JA, MF_BYCOMMAND);
  } else {
    CheckMenuRadioItem(app::hMenu, IDM_OPT_LANG_JA, IDM_OPT_LANG_EN, IDM_OPT_LANG_EN, MF_BYCOMMAND);
  }
  CheckMenuRadioItem(app::hMenu, IDM_OPT_PF, IDM_OPT_PE, IDM_OPT_PF, MF_BYCOMMAND);  // Prime factorization
  CheckMenuRadioItem(app::hMenu, IDM_OPT_CHARSET_UTF8, IDM_OPT_CHARSET_SJIS, IDM_OPT_CHARSET_UTF8,
                     MF_BYCOMMAND);                                       // UTF-8
  EnableMenuItem(app::hMenu, IDM_OPT_CNTONLY, MF_BYCOMMAND | MF_GRAYED);  // Disables options for prime enumeration
  EnableMenuItem(app::hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_GRAYED);  // Same here

  hMemDC = CreateCompatibleDC(NULL);
  hBshSys = GetSysColorBrush(COLOR_BTNFACE);
  hPenSys = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
  BGDark = CreateSolidBrush(0x3f3936);

  // Using a dummy position and size here as we will set the actual value at `onSize`.
  // Input box
  app::hEdi0 =
      CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL, 0, 0,
                      0, 0, hWnd, (HMENU)IDC_EDIT_IN1, lpCreateStruct->hInstance, NULL);
  if (!app::hEdi0) {
    util::messageBox(hWnd, app::hInst, app::wcMes[IDS_ECRTWND], app::wcMes[IDS_ERROR], MB_OK | MB_ICONERROR);
    return 1;
  }
  SendMessageW(app::hEdi0, EM_SETLIMITTEXT, (WPARAM)(MAX_INPUT_LENGTH - 1), 0);
  edit0Props.defProc = (WNDPROC)mySetWindowLongW(app::hEdi0, GWL_WNDPROC, (LONG_PTR)wproc::inputProc);  // Subclassing
  edit0Props.hPrevWnd = app::hEdi0;
  edit0Props.hNextWnd = app::hEdi0;
  edit0Props.runOnEnter = true;
  mySetWindowLongW(app::hEdi0, GWL_USERDATA, (LONG_PTR)&edit0Props);
  SetFocus(app::hFocused = app::hEdi0);  // Focus and update the log

  // Result box
  app::hEdiOut = CreateWindowExW(
      0, L"EDIT", L"",
      WS_CHILD | WS_VISIBLE | ES_READONLY | ES_LEFT | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL, 0, 0, 0,
      0, hWnd, (HMENU)IDC_EDIT_OUT, lpCreateStruct->hInstance, NULL);
  if (!app::hEdiOut) {
    util::messageBox(hWnd, app::hInst, app::wcMes[IDS_ECRTWND], app::wcMes[IDS_ERROR], MB_OK | MB_ICONERROR);
    return 1;
  }
  SendMessageW(app::hEdiOut, EM_SETLIMITTEXT, (WPARAM)(MAX_OUTPUT_BUFFER - 1), 0);

  // OK button
  app::hBtnOK = CreateWindowExW(0, L"BUTTON", app::wcMes[IDS_OK], WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0,
                                hWnd, (HMENU)IDC_BUTTON_OK, app::hInst, NULL);
  // Abort button
  app::hBtnAbort =
      CreateWindowExW(0, L"BUTTON", app::wcMes[IDS_ABORT], WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_DISABLED, 0, 0, 0,
                      0, hWnd, (HMENU)IDC_BUTTON_ABORT, app::hInst, NULL);
  // Clear History button
  app::hBtnClear = CreateWindowExW(0, L"BUTTON", app::wcMes[IDS_CLRHST], WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0,
                                   0, hWnd, (HMENU)IDC_BUTTON_CLEAR, app::hInst, NULL);
  // 0
  app::hBtn0 = CreateWindowExW(0, L"BUTTON", L"0", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hWnd,
                               (HMENU)IDC_BUTTON_0, app::hInst, NULL);
  // 1
  app::hBtn1 = CreateWindowExW(0, L"BUTTON", L"1", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hWnd,
                               (HMENU)IDC_BUTTON_1, app::hInst, NULL);
  // 2
  app::hBtn2 = CreateWindowExW(0, L"BUTTON", L"2", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hWnd,
                               (HMENU)IDC_BUTTON_2, app::hInst, NULL);
  // 3
  app::hBtn3 = CreateWindowExW(0, L"BUTTON", L"3", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hWnd,
                               (HMENU)IDC_BUTTON_3, app::hInst, NULL);
  // 4
  app::hBtn4 = CreateWindowExW(0, L"BUTTON", L"4", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hWnd,
                               (HMENU)IDC_BUTTON_4, app::hInst, NULL);
  // 5
  app::hBtn5 = CreateWindowExW(0, L"BUTTON", L"5", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hWnd,
                               (HMENU)IDC_BUTTON_5, app::hInst, NULL);
  // 6
  app::hBtn6 = CreateWindowExW(0, L"BUTTON", L"6", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hWnd,
                               (HMENU)IDC_BUTTON_6, app::hInst, NULL);
  // 7
  app::hBtn7 = CreateWindowExW(0, L"BUTTON", L"7", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hWnd,
                               (HMENU)IDC_BUTTON_7, app::hInst, NULL);
  // 8
  app::hBtn8 = CreateWindowExW(0, L"BUTTON", L"8", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hWnd,
                               (HMENU)IDC_BUTTON_8, app::hInst, NULL);
  // 9
  app::hBtn9 = CreateWindowExW(0, L"BUTTON", L"9", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hWnd,
                               (HMENU)IDC_BUTTON_9, app::hInst, NULL);
  // Back Space
  app::hBtnBS = CreateWindowExW(0, L"BUTTON", app::wcMes[IDS_BS], WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0,
                                hWnd, (HMENU)IDC_BUTTON_BS, app::hInst, NULL);
  // Clear Entry
  app::hBtnCE = CreateWindowExW(0, L"BUTTON", app::wcMes[IDS_CE], WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0,
                                hWnd, (HMENU)IDC_BUTTON_CE, app::hInst, NULL);

#ifndef UNDER_CE
  // Obtains the "system DPI" value. We use this as the fallback value on older Windows versions and to calculate the
  // appropriate font height value for ChooseFontW.
  HDC hDC = GetDC(hWnd);
  int dpi = GetDeviceCaps(hDC, LOGPIXELSX);
  ReleaseDC(hWnd, hDC);

  // Tries to load Monitor APIs avoiding a direct call to make this app runnable on old devices.
  //
  // Microsoft recommends the use of GetDpiForWindow API instead of this API according to their documentation. However,
  // it requires Windows 10 1607 or later, which makes this compatibility keeping code more complicated, and
  // GetDpiForMonitor API still works for programs that only use the process-wide DPI awareness. Here, as we only use
  // the process-wide DPI awareness, we are going to use GetDpiForMonitor API.
  //
  // References:
  // https://learn.microsoft.com/en-us/windows/win32/api/shellscalingapi/nf-shellscalingapi-getdpiformonitor
  // https://mariusbancila.ro/blog/2021/05/19/how-to-build-high-dpi-aware-native-desktop-applications/
  HMODULE shcore = LoadLibraryW(L"Shcore.dll"), user32 = LoadLibraryW(L"User32.dll");
  GetDpiForMonitor_t getDpiForMonitor = NULL;
  if (shcore) getDpiForMonitor = (GetDpiForMonitor_t)(void *)GetProcAddress(shcore, "GetDpiForMonitor");
  MonitorFromWindow_t monitorFromWindow = NULL;
  if (user32) monitorFromWindow = (MonitorFromWindow_t)(void *)GetProcAddress(user32, "MonitorFromWindow");

  // Tests whether it successfully got the APIs.
  if (getDpiForMonitor && monitorFromWindow) {  // It got (the system is presumably Windows 8.1 or later).
    unsigned tmpX, tmpY;
    getDpiForMonitor(monitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), 0, &tmpX, &tmpY);
    dpi = tmpX;
  }
  if (shcore) FreeLibrary(shcore);
  if (user32) FreeLibrary(user32);

  // Adjusts the window size according to the DPI value.
  SetWindowPos(hWnd, NULL, 0, 0, 640 * dpi / 96, 480 * dpi / 96,
               SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
#endif
  return TRUE;
}

void onClose(HWND hWnd) {
  DeleteObject(hPenSys);
  DeleteObject(BGDark);
  DestroyWindow(hWnd);
}

HBRUSH onCtlColor(HWND hWnd, HDC hdc, HWND hWndChild, int type) {
  UNREFERENCED_PARAMETER(hWnd);
  UNREFERENCED_PARAMETER(hWndChild);
  UNREFERENCED_PARAMETER(type);

  SetTextColor(hdc, 0x00ff00);
  SetBkColor(hdc, 0x3f3936);
  return BGDark;
}

void onDestroy(HWND hWnd) {
  UNREFERENCED_PARAMETER(hWnd);

  PostQuitMessage(0);
}

void onSize(HWND hWnd, unsigned state, int cx, int cy) {
  UNREFERENCED_PARAMETER(state);
  UNREFERENCED_PARAMETER(cx);
  UNREFERENCED_PARAMETER(cy);

  int scrx, scry;
  RECT rect;
  LOGFONTW rLogfont;              // Font configuration
  static HBITMAP hBitmap = NULL;  // Handle of a bitmap, which is used for double buffering

  GetClientRect(hWnd, &rect);
  scrx = rect.right;
  scry = rect.bottom;

  // Font for the main window
  if (scrx / 24 < scry / 12) {
    rLogfont.lfHeight = scrx / 24;
  } else {
    rLogfont.lfHeight = scry / 12;
  }
  rLogfont.lfWidth = 0;
  rLogfont.lfEscapement = 0;
  rLogfont.lfOrientation = 0;
  rLogfont.lfWeight = FW_EXTRABOLD;
  rLogfont.lfItalic = TRUE;
  rLogfont.lfUnderline = TRUE;
  rLogfont.lfStrikeOut = FALSE;
  rLogfont.lfCharSet = SHIFTJIS_CHARSET;
  rLogfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
  rLogfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
  rLogfont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
#ifdef UNDER_CE
  // Sets a pre-installed font on Windows CE, as it doesn't have "MS Shell Dlg".
  lstrcpyW(rLogfont.lfFaceName, L"Tahoma");
  rLogfont.lfQuality = ANTIALIASED_QUALITY;
#else
  // Sets a logical font face name for localization.
  // It maps to a default shell font associated with the current culture/locale.
  lstrcpyW(rLogfont.lfFaceName, L"MS Shell Dlg");
  rLogfont.lfQuality = DEFAULT_QUALITY;
#endif
  if (hFmes) DeleteObject(hFmes);          // Deletes the previous font
  hFmes = CreateFontIndirectW(&rLogfont);  // Creates the new font

  // Font for buttons
  if (24 * scrx / 700 < 24 * scry / 400) {
    rLogfont.lfHeight = 24 * scrx / 700;
  } else {
    rLogfont.lfHeight = 24 * scry / 400;
  }
  rLogfont.lfWidth = 0;
  rLogfont.lfEscapement = 0;
  rLogfont.lfOrientation = 0;
  rLogfont.lfWeight = FW_NORMAL;
  rLogfont.lfItalic = FALSE;
  rLogfont.lfUnderline = FALSE;
  rLogfont.lfStrikeOut = FALSE;
  rLogfont.lfCharSet = SHIFTJIS_CHARSET;
  rLogfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
  rLogfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
  rLogfont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
#ifdef UNDER_CE
  // Sets a pre-installed font on Windows CE, as it doesn't have "MS Shell Dlg".
  lstrcpyW(rLogfont.lfFaceName, L"Tahoma");
  rLogfont.lfQuality = ANTIALIASED_QUALITY;
#else
  // Sets a logical font face name for localization.
  // It maps to a default shell font associated with the current culture/locale.
  lstrcpyW(rLogfont.lfFaceName, L"MS Shell Dlg");
  rLogfont.lfQuality = DEFAULT_QUALITY;
#endif
  if (hFbtn) DeleteObject(hFbtn);
  hFbtn = CreateFontIndirectW(&rLogfont);

  // Font for the output box
  if (20 * scrx / 700 < 20 * scry / 400) {
    rLogfont.lfHeight = 20 * scrx / 700;
  } else {
    rLogfont.lfHeight = 20 * scry / 400;
  }
  if (rLogfont.lfHeight < 12) rLogfont.lfHeight = 12;
  rLogfont.lfWidth = 0;
  rLogfont.lfEscapement = 0;
  rLogfont.lfOrientation = 0;
  rLogfont.lfWeight = FW_NORMAL;
  rLogfont.lfItalic = FALSE;
  rLogfont.lfUnderline = FALSE;
  rLogfont.lfStrikeOut = FALSE;
  rLogfont.lfCharSet = SHIFTJIS_CHARSET;
  rLogfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
  rLogfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
  rLogfont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
#ifdef UNDER_CE
  // Sets a pre-installed font on Windows CE, as it doesn't have "MS Shell Dlg".
  lstrcpyW(rLogfont.lfFaceName, L"Tahoma");
  rLogfont.lfQuality = ANTIALIASED_QUALITY;
#else
  // Sets a logical font face name for localization.
  // It maps to a default shell font associated with the current culture/locale.
  lstrcpyW(rLogfont.lfFaceName, L"MS Shell Dlg");
  rLogfont.lfQuality = DEFAULT_QUALITY;
#endif
  if (hFedi) DeleteObject(hFedi);
  hFedi = CreateFontIndirectW(&rLogfont);

  // Apply the new fonts
  SendMessageW(app::hBtnOK, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
  SendMessageW(app::hBtnAbort, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
  SendMessageW(app::hBtnClear, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
  SendMessageW(app::hBtn0, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
  SendMessageW(app::hBtn1, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
  SendMessageW(app::hBtn2, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
  SendMessageW(app::hBtn3, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
  SendMessageW(app::hBtn4, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
  SendMessageW(app::hBtn5, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
  SendMessageW(app::hBtn6, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
  SendMessageW(app::hBtn7, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
  SendMessageW(app::hBtn8, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
  SendMessageW(app::hBtn9, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
  SendMessageW(app::hBtnBS, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
  SendMessageW(app::hBtnCE, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
  SendMessageW(app::hEdi0, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
  SendMessageW(app::hEdiOut, WM_SETFONT, (WPARAM)hFedi, MAKELPARAM(FALSE, 0));
  if (app::mode == app::MODE_PE) {
    SendMessageW(app::hEdi1, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
    SendMessageW(app::hEdi2, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
  }

  // Move and resize controls
#ifdef UNDER_CE
  MoveWindow(app::hCmdBar, 0, 0, 0, 0, TRUE);
#endif
  buttonX = 96 * scrx / 700;
  buttonY = 32 * scry / 400;
  int nbX = scrx / 20, nbY = CmdBar_Height + (app::mode == app::MODE_PF ? scry * 9 / 40 : scry * 3 / 10),
      nbW = scrx * 9 / 120;
  MoveWindow(app::hBtn0, nbX, nbY, nbW, buttonY, TRUE);
  nbX += nbW;
  MoveWindow(app::hBtn1, nbX, nbY, nbW, buttonY, TRUE);
  nbX += nbW;
  MoveWindow(app::hBtn2, nbX, nbY, nbW, buttonY, TRUE);
  nbX += nbW;
  MoveWindow(app::hBtn3, nbX, nbY, nbW, buttonY, TRUE);
  nbX += nbW;
  MoveWindow(app::hBtn4, nbX, nbY, nbW, buttonY, TRUE);
  nbX += nbW;
  MoveWindow(app::hBtn5, nbX, nbY, nbW, buttonY, TRUE);
  nbX += nbW;
  MoveWindow(app::hBtn6, nbX, nbY, nbW, buttonY, TRUE);
  nbX += nbW;
  MoveWindow(app::hBtn7, nbX, nbY, nbW, buttonY, TRUE);
  nbX += nbW;
  MoveWindow(app::hBtn8, nbX, nbY, nbW, buttonY, TRUE);
  nbX += nbW;
  MoveWindow(app::hBtn9, nbX, nbY, nbW, buttonY, TRUE);
  nbX += nbW;
  MoveWindow(app::hBtnBS, nbX, nbY, nbW, buttonY, TRUE);
  nbX += nbW;
  MoveWindow(app::hBtnCE, nbX, nbY, nbW, buttonY, TRUE);
  if (app::mode == app::MODE_PF) {
    MoveWindow(app::hEdi0, buttonX, CmdBar_Height, buttonX * 3, buttonY, TRUE);
    MoveWindow(app::hBtnOK, buttonX * 4, CmdBar_Height, buttonX * 2 / 3, buttonY, TRUE);
    MoveWindow(app::hBtnAbort, buttonX * 14 / 3, CmdBar_Height, buttonX * 2 / 3, buttonY, TRUE);
    MoveWindow(app::hBtnClear, buttonX * 16 / 3, CmdBar_Height, buttonX * 5 / 3, buttonY, TRUE);
    MoveWindow(app::hEdiOut, scrx / 20, CmdBar_Height + scry * 9 / 40 + buttonY, scrx * 9 / 10,
               scry * 29 / 40 - buttonY - CmdBar_Height, TRUE);
  } else {
    MoveWindow(app::hEdi0, buttonX, CmdBar_Height, buttonX * 5 / 2, buttonY, TRUE);
    MoveWindow(app::hEdi1, buttonX * 9 / 2, CmdBar_Height, buttonX * 5 / 2, buttonY, TRUE);
    MoveWindow(app::hEdi2, buttonX, CmdBar_Height + buttonY, buttonX * 2, buttonY, TRUE);
    MoveWindow(app::hBtnOK, buttonX * 3, CmdBar_Height + buttonY, buttonX * 2 / 3, buttonY, TRUE);
    MoveWindow(app::hBtnAbort, buttonX * 11 / 3, CmdBar_Height + buttonY, buttonX * 2 / 3, buttonY, TRUE);
    MoveWindow(app::hBtnClear, buttonX * 13 / 3, CmdBar_Height + buttonY, buttonX * 5 / 3, buttonY, TRUE);
    MoveWindow(app::hEdiOut, scrx / 20, CmdBar_Height + scry * 3 / 10 + buttonY, scrx * 9 / 10,
               scry * 13 / 20 - buttonY - CmdBar_Height, TRUE);
  }

  if (hBitmap) DeleteObject(hBitmap);  // Deletes the previous bitmap
  HDC hdc = GetDC(hWnd);
  hBitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
  if (!hBitmap) {  // Creates a bitmap with the new size
    util::messageBox(hWnd, app::hInst, app::wcMes[IDS_ESCRBUF], app::wcMes[IDS_ERROR], MB_OK | MB_ICONERROR);
    PostQuitMessage(1);
    return;
  }
  ReleaseDC(hWnd, hdc);
  SelectObject(hMemDC, hBitmap);  // Set the new bitmap to the memory device context

  redraw(hWnd);  // Repaint with new size
}

void onPaint(HWND hWnd) {
  RECT rect;
  PAINTSTRUCT ps;

  GetClientRect(hWnd, &rect);

  HDC hdc = BeginPaint(hWnd, &ps);
  BitBlt(hdc, 0, 0, rect.right, rect.bottom, hMemDC, 0, 0, SRCCOPY);  // Copies the double bufferred content
  EndPaint(hWnd, &ps);
}

void onCommand(HWND hWnd, int id, HWND hWndCtl, unsigned codeNotify) {
  UNREFERENCED_PARAMETER(codeNotify);

  if (hWndCtl == app::hEdi0 || hWndCtl == app::hEdiOut ||
      (app::mode == app::MODE_PE && (hWndCtl == app::hEdi1 || hWndCtl == app::hEdi2))) {
    app::hFocused = hWndCtl;
  } else {
    SetFocus(app::hFocused);
  }
  EnableMenuItem(app::hMenu, IDM_EDIT_CUT, MF_BYCOMMAND | (app::hFocused == app::hEdiOut ? MF_GRAYED : MF_ENABLED));
  EnableMenuItem(app::hMenu, IDM_EDIT_PASTE, MF_BYCOMMAND | (app::hFocused == app::hEdiOut ? MF_GRAYED : MF_ENABLED));

  switch (id) {
    case IDC_BUTTON_OK:
      if (!app::isRunning) runner::begin();
      break;

    case IDC_BUTTON_ABORT:
      runner::isAborted = true;
      break;

    case IDC_BUTTON_CLEAR:
      SetWindowTextW(app::hEdiOut, L"");
      break;

    case IDM_FILE_SAVE_AS: {
      HANDLE hHeap = GetProcessHeap();
      OPENFILENAMEW *ofn = (OPENFILENAMEW *)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(OPENFILENAMEW));
      if (!ofn) {
        util::messageBox(hWnd, app::hInst, app::wcMes[IDS_EALLOC], app::wcMes[IDS_ERROR], MB_OK | MB_ICONWARNING);
        break;
      }

      ofn->lStructSize = sizeof(OPENFILENAMEW);
      ofn->hwndOwner = hWnd;
      ofn->lpstrFilter = L"Text File (*.txt)\0*.txt\0All files (*.*)\0*.*\0";
      ofn->lpstrFile = (wchar_t *)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, MAX_PATH * sizeof(wchar_t));
      if (!ofn->lpstrFile) {
        util::messageBox(hWnd, app::hInst, app::wcMes[IDS_EALLOC], app::wcMes[IDS_ERROR], MB_OK | MB_ICONWARNING);
        HeapFree(hHeap, 0, ofn);
        break;
      }
      ofn->nMaxFile = MAX_PATH;
      ofn->lpstrDefExt = L".txt";
      ofn->lpstrTitle = app::wcMes[IDS_SAVE_TITLE];
      ofn->Flags = OFN_OVERWRITEPROMPT;
      if (!GetSaveFileNameW(ofn)) {
        HeapFree(hHeap, 0, ofn->lpstrFile);
        HeapFree(hHeap, 0, ofn);
        break;
      }

      // File creation (overwrite if exists)
      HANDLE hFile =
          CreateFileW(ofn->lpstrFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
      if (hFile == INVALID_HANDLE_VALUE) {
        util::messageBox(hWnd, app::hInst, app::wcMes[IDS_EOPEN], app::wcMes[IDS_ERROR], MB_OK | MB_ICONWARNING);
        HeapFree(hHeap, 0, ofn->lpstrFile);
        HeapFree(hHeap, 0, ofn);
        break;
      }

      int editlen = GetWindowTextLengthW(app::hEdiOut) + 1;  // Includes a null terminator
      wchar_t *wcEdit = (wchar_t *)HeapAlloc(hHeap, 0, editlen * sizeof(wchar_t));
      if (!wcEdit) {
        util::messageBox(hWnd, app::hInst, app::wcMes[IDS_EALLOC], app::wcMes[IDS_ERROR], MB_OK | MB_ICONWARNING);
        HeapFree(hHeap, 0, ofn->lpstrFile);
        HeapFree(hHeap, 0, ofn);
        break;
      }

      GetWindowTextW(app::hEdiOut, wcEdit, editlen);  // Gets the content of the output box

      int mblen = WideCharToMultiByte(app::charset == IDM_OPT_CHARSET_SJIS ? 932 : 65001, 0, wcEdit, editlen, NULL, 0,
                                      NULL, NULL);
      char *mbEdit = (char *)HeapAlloc(hHeap, 0, mblen * sizeof(char));
      if (!mbEdit) {
        util::messageBox(hWnd, app::hInst, app::wcMes[IDS_EALLOC], app::wcMes[IDS_ERROR], MB_OK | MB_ICONWARNING);
        HeapFree(hHeap, 0, ofn->lpstrFile);
        HeapFree(hHeap, 0, ofn);
        HeapFree(hHeap, 0, wcEdit);
        break;
      }

      DWORD dwTemp;
      WideCharToMultiByte(app::charset == IDM_OPT_CHARSET_SJIS ? 932 : 65001, 0, wcEdit, editlen, mbEdit, mblen, NULL,
                          NULL);

      if (WriteFile(hFile, mbEdit, (mblen - 1) * sizeof(char), &dwTemp, NULL)) {
        util::messageBox(hWnd, app::hInst, app::wcMes[IDS_FILEDONE], app::wcMes[IDS_INFO], MB_OK | MB_ICONINFORMATION);
      } else {
        util::messageBox(hWnd, app::hInst, app::wcMes[IDS_EWRITE], app::wcMes[IDS_ERROR], MB_OK | MB_ICONWARNING);
      }

      CloseHandle(hFile);
      HeapFree(hHeap, 0, ofn->lpstrFile);
      HeapFree(hHeap, 0, ofn);
      HeapFree(hHeap, 0, wcEdit);
      HeapFree(hHeap, 0, mbEdit);
      break;
    }

    case IDM_FILE_EXIT:
      SendMessageW(hWnd, WM_CLOSE, 0, 0);
      break;

    case IDM_EDIT_CUT:
      SendMessageW(app::hFocused, WM_CUT, 0, 0);
      break;

    case IDM_EDIT_COPY:
      SendMessageW(app::hFocused, WM_COPY, 0, 0);
      break;

    case IDM_EDIT_PASTE:
      SendMessageW(app::hFocused, WM_PASTE, 0, 0);
      break;

    case IDM_EDIT_SELECT_ALL:
      SendMessageW(app::hFocused, EM_SETSEL, 0, SendMessageW(app::hFocused, WM_GETTEXTLENGTH, 0, 0));
      break;

    case IDC_BUTTON_BS: {
      if (app::hFocused == app::hEdiOut) break;
      LRESULT editlen = SendMessageW(app::hFocused, EM_GETSEL, 0, 0);
      if (LOWORD(editlen) == HIWORD(editlen)) {
        SendMessageW(app::hFocused, EM_SETSEL, LOWORD(editlen) - 1, LOWORD(editlen));
        SendMessageW(app::hFocused, EM_REPLACESEL, 0, (WPARAM)L"");
      } else {
        SendMessageW(app::hFocused, EM_REPLACESEL, 0, (WPARAM)L"");
      }
      break;
    }

    case IDC_BUTTON_CE: {
      if (app::hFocused == app::hEdiOut) break;
      LRESULT editlen = SendMessageW(app::hFocused, WM_GETTEXTLENGTH, 0, 0);
      SendMessageW(app::hFocused, EM_SETSEL, 0, editlen);
      SendMessageW(app::hFocused, EM_REPLACESEL, 0, (WPARAM)L"");
      break;
    }

    case IDM_OPT_PF: {  // Switch to Prime Factorization
      if (app::mode == app::MODE_PF) break;
      app::mode = app::MODE_PF;

      CheckMenuRadioItem(app::hMenu, IDM_OPT_PF, IDM_OPT_PE, IDM_OPT_PF, MF_BYCOMMAND);
      EnableMenuItem(app::hMenu, IDM_OPT_CNTONLY, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(app::hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(app::hMenu, IDM_EDIT_CUT, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(app::hMenu, IDM_EDIT_PASTE, MF_BYCOMMAND | MF_ENABLED);
      SetFocus(app::hEdi0);

      DestroyWindow(app::hEdi1);  // Remove unnecessary edit boxes
      DestroyWindow(app::hEdi2);

      edit0Props.hPrevWnd = app::hEdi0;
      edit0Props.hNextWnd = app::hEdi0;
      edit0Props.runOnEnter = true;

      onSize(hWnd);
      break;
    }

    case IDM_OPT_PE: {  // Switch to Enumerate Prime Numbers
      if (app::mode == app::MODE_PE) break;
      app::mode = app::MODE_PE;

      CheckMenuRadioItem(app::hMenu, IDM_OPT_PF, IDM_OPT_PE, IDM_OPT_PE, MF_BYCOMMAND);
      EnableMenuItem(app::hMenu, IDM_OPT_CNTONLY, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(app::hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(app::hMenu, IDM_EDIT_CUT, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(app::hMenu, IDM_EDIT_PASTE, MF_BYCOMMAND | MF_ENABLED);
      SetFocus(app::hEdi0);

      app::hEdi1 = CreateWindowExW(  // Input box
          0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd,
          (HMENU)IDC_EDIT_IN2, app::hInst, NULL);
      if (!app::hEdi1) {
        util::messageBox(hWnd, app::hInst, app::wcMes[IDS_ECRTWND], app::wcMes[IDS_ERROR], MB_OK | MB_ICONERROR);
        PostQuitMessage(1);
        break;
      }
      SendMessageW(app::hEdi1, EM_SETLIMITTEXT, (WPARAM)(MAX_INPUT_LENGTH - 1), 0);

      app::hEdi2 = CreateWindowExW(  // Input box
          0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd,
          (HMENU)IDC_EDIT_IN3, app::hInst, NULL);
      if (!app::hEdi2) {
        util::messageBox(hWnd, app::hInst, app::wcMes[IDS_ECRTWND], app::wcMes[IDS_ERROR], MB_OK | MB_ICONERROR);
        PostQuitMessage(1);
        break;
      }
      SendMessageW(app::hEdi2, EM_SETLIMITTEXT, (WPARAM)(MAX_INPUT_LENGTH - 1), 0);

      edit0Props.hPrevWnd = app::hEdi2;
      edit1Props.hPrevWnd = app::hEdi0;
      edit2Props.hPrevWnd = app::hEdi1;
      edit0Props.hNextWnd = app::hEdi1;
      edit1Props.hNextWnd = app::hEdi2;
      edit2Props.hNextWnd = app::hEdi0;
      edit0Props.runOnEnter = false;
      edit1Props.runOnEnter = false;
      edit2Props.runOnEnter = true;
      edit1Props.defProc = (WNDPROC)mySetWindowLongW(app::hEdi1, GWL_WNDPROC, (LONG_PTR)wproc::inputProc);
      edit2Props.defProc = (WNDPROC)mySetWindowLongW(app::hEdi2, GWL_WNDPROC, (LONG_PTR)wproc::inputProc);
      mySetWindowLongW(app::hEdi1, GWL_USERDATA, (LONG_PTR)&edit1Props);
      mySetWindowLongW(app::hEdi2, GWL_USERDATA, (LONG_PTR)&edit2Props);

      onSize(hWnd);
      break;
    }

    case IDM_OPT_CNTONLY:
      if (app::countOnly) {  // Unchecks if checked
        CheckMenuItem(app::hMenu, IDM_OPT_CNTONLY, MF_BYCOMMAND | MF_UNCHECKED);
        EnableMenuItem(app::hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_ENABLED);
        SendMessageW(app::hEdi2, EM_SETREADONLY, (WPARAM)FALSE, (LPARAM)NULL);
        app::countOnly = false;
      } else {  // Checks if unchecked
        CheckMenuItem(app::hMenu, IDM_OPT_CNTONLY, MF_BYCOMMAND | MF_CHECKED);
        EnableMenuItem(app::hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_GRAYED);
        SendMessageW(app::hEdi2, EM_SETREADONLY, (WPARAM)TRUE, (LPARAM)NULL);
        app::countOnly = true;
      }
      break;

    case IDM_OPT_OUTFILE:
      if (app::useFile) {  // Unchecks if checked
        app::useFile = false;
        CheckMenuItem(app::hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_UNCHECKED);
      } else {  // Checks if unchecked
        app::useFile = true;
        CheckMenuItem(app::hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_CHECKED);
      }
      break;

    case IDM_OPT_LANG_JA: {
      if (app::langid == 0x0411) break;
      app::langid = 0x0411;

#ifdef UNDER_CE
      CommandBar_Destroy(app::hCmdBar);
      app::hCmdBar = CommandBar_Create(app::hInst, hWnd, 1);
      wchar_t tmp[] = L"ResMenu_JA";
      CommandBar_InsertMenubarEx(app::hCmdBar, app::hInst, tmp, 0);
      CommandBar_Show(app::hCmdBar, TRUE);
      CmdBar_Height = CommandBar_Height(app::hCmdBar);
      app::hMenu = CommandBar_GetMenu(app::hCmdBar, 0);
#else
      DestroyMenu(app::hMenu);
      app::hMenu = LoadMenuW(app::hInst, L"ResMenu_JA");
      SetMenu(hWnd, app::hMenu);
#endif

      // Restores checkboxes and radio buttons
      CheckMenuRadioItem(app::hMenu, IDM_OPT_PF, IDM_OPT_PE, app::mode == app::MODE_PF ? IDM_OPT_PF : IDM_OPT_PE,
                         MF_BYCOMMAND);
      if (app::countOnly) CheckMenuItem(app::hMenu, IDM_OPT_CNTONLY, MF_BYCOMMAND | MF_CHECKED);
      if (app::useFile) CheckMenuItem(app::hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_CHECKED);
      CheckMenuRadioItem(app::hMenu, IDM_OPT_LANG_JA, IDM_OPT_LANG_EN, IDM_OPT_LANG_JA, MF_BYCOMMAND);
      CheckMenuRadioItem(app::hMenu, IDM_OPT_CHARSET_UTF8, IDM_OPT_CHARSET_SJIS, app::charset, MF_BYCOMMAND);

      // Restores the enablement statuses
      if (app::mode == app::MODE_PF) EnableMenuItem(app::hMenu, IDM_OPT_CNTONLY, MF_BYCOMMAND | MF_GRAYED);
      if (app::mode == app::MODE_PF || app::countOnly) {
        EnableMenuItem(app::hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_GRAYED);
      }

      int i;
      for (i = 0; i < SIZE_OF_STRING_TABLE; i++) {  // Loads the Japanese String Table
        LoadStringW(app::hInst, IDS_JA + i, app::wcMes[i], sizeof(app::wcMes[0]) / sizeof(app::wcMes[0][0]));
      }
      SetWindowTextW(app::hBtnOK, app::wcMes[IDS_OK]);
      SetWindowTextW(app::hBtnAbort, app::wcMes[IDS_ABORT]);
      SetWindowTextW(app::hBtnClear, app::wcMes[IDS_CLRHST]);
      SetWindowTextW(app::hBtnBS, app::wcMes[IDS_BS]);
      SetWindowTextW(app::hBtnCE, app::wcMes[IDS_CE]);
      SetWindowTextW(hWnd, app::wcMes[IDS_APPNAME]);
      redraw(hWnd);
      break;
    }

    case IDM_OPT_LANG_EN: {
      if (app::langid == 0x0409) break;
      app::langid = 0x0409;

#ifdef UNDER_CE
      CommandBar_Destroy(app::hCmdBar);
      app::hCmdBar = CommandBar_Create(app::hInst, hWnd, 1);
      wchar_t tmp[] = L"ResMenu_EN";
      CommandBar_InsertMenubarEx(app::hCmdBar, app::hInst, tmp, 0);
      CommandBar_Show(app::hCmdBar, TRUE);
      CmdBar_Height = CommandBar_Height(app::hCmdBar);
      app::hMenu = CommandBar_GetMenu(app::hCmdBar, 0);
#else
      DestroyMenu(app::hMenu);
      app::hMenu = LoadMenuW(app::hInst, L"ResMenu_EN");
      SetMenu(hWnd, app::hMenu);
#endif

      // Restores checkboxes and radio buttons
      CheckMenuRadioItem(app::hMenu, IDM_OPT_PF, IDM_OPT_PE, app::mode == app::MODE_PF ? IDM_OPT_PF : IDM_OPT_PE,
                         MF_BYCOMMAND);
      if (app::countOnly) CheckMenuItem(app::hMenu, IDM_OPT_CNTONLY, MF_BYCOMMAND | MF_CHECKED);
      if (app::useFile) CheckMenuItem(app::hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_CHECKED);
      CheckMenuRadioItem(app::hMenu, IDM_OPT_LANG_JA, IDM_OPT_LANG_EN, IDM_OPT_LANG_EN, MF_BYCOMMAND);
      CheckMenuRadioItem(app::hMenu, IDM_OPT_CHARSET_UTF8, IDM_OPT_CHARSET_SJIS, app::charset, MF_BYCOMMAND);

      // Restores the enablement statuses
      if (app::mode == app::MODE_PF) EnableMenuItem(app::hMenu, IDM_OPT_CNTONLY, MF_BYCOMMAND | MF_GRAYED);
      if (app::mode == app::MODE_PF || app::countOnly) {
        EnableMenuItem(app::hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_GRAYED);
      }

      int i;
      for (i = 0; i < SIZE_OF_STRING_TABLE; i++) {  // Loads the English String Table
        LoadStringW(app::hInst, IDS_EN + i, app::wcMes[i], sizeof(app::wcMes[0]) / sizeof(app::wcMes[0][0]));
      }
      SetWindowTextW(app::hBtnOK, app::wcMes[IDS_OK]);
      SetWindowTextW(app::hBtnAbort, app::wcMes[IDS_ABORT]);
      SetWindowTextW(app::hBtnClear, app::wcMes[IDS_CLRHST]);
      SetWindowTextW(app::hBtnBS, app::wcMes[IDS_BS]);
      SetWindowTextW(app::hBtnCE, app::wcMes[IDS_CE]);
      SetWindowTextW(hWnd, app::wcMes[IDS_APPNAME]);
      redraw(hWnd);
      break;
    }

    case IDM_OPT_CHARSET_UTF8:
      if (app::charset == IDM_OPT_CHARSET_UTF8) break;
      app::charset = IDM_OPT_CHARSET_UTF8;

      CheckMenuRadioItem(app::hMenu, IDM_OPT_CHARSET_UTF8, IDM_OPT_CHARSET_SJIS, IDM_OPT_CHARSET_UTF8, MF_BYCOMMAND);
      break;

    case IDM_OPT_CHARSET_SJIS:
      if (app::charset == IDM_OPT_CHARSET_SJIS) break;
      app::charset = IDM_OPT_CHARSET_SJIS;

      CheckMenuRadioItem(app::hMenu, IDM_OPT_CHARSET_UTF8, IDM_OPT_CHARSET_SJIS, IDM_OPT_CHARSET_SJIS, MF_BYCOMMAND);
      break;

    case IDM_HELP_HOWTOUSE:
      util::messageBox(hWnd, app::hInst, app::wcMes[app::mode == app::MODE_PF ? IDS_PFHELP : IDS_PEHELP],
                       app::wcMes[IDS_HELP_TITLE], MB_OK | MB_ICONINFORMATION);
      break;

    case IDM_HELP_ABOUT: {
      wchar_t wcTemp[MAX_BUFFER];
      wsprintfW(wcTemp, L"%s\n\n%s" TEXT(__DATE__) L"\n\n%s", app::wcMes[IDS_ABOUT],
                app::wcMes[IDS_BUILD], app::wcMes[IDS_COPYRIGHT]);
      util::messageBox(hWnd, app::hInst, wcTemp, app::wcMes[IDS_ABOUT_TITLE], MB_OK | MB_ICONINFORMATION);
      break;
    }
  }

  if (id >= IDC_BUTTON_0 && id <= IDC_BUTTON_9 && app::hFocused != app::hEdiOut) {  // Screen Keyboard
    wchar_t num[2] = {(wchar_t)(L'0' + (id - IDC_BUTTON_0)), 0};
    SendMessageW(app::hFocused, EM_REPLACESEL, 0, (WPARAM)num);
  }
}

// 再描画
void redraw(HWND hWnd) {
  int scrx, scry;
  RECT rect;

  GetClientRect(hWnd, &rect);
  scrx = rect.right;
  scry = rect.bottom;

  SelectObject(hMemDC, GetStockObject(BLACK_PEN));    // Sets a pen to the memory device context
  SelectObject(hMemDC, GetStockObject(BLACK_BRUSH));  // Sets a brush to the memory device context
  Rectangle(hMemDC, rect.left, rect.top, rect.right, rect.bottom);
  rect.top = CmdBar_Height;

  if (app::mode == app::MODE_PF) {
    // Label background
    SelectObject(hMemDC, hPenSys);
    SelectObject(hMemDC, hBshSys);
    Rectangle(hMemDC, 0, CmdBar_Height, buttonX, CmdBar_Height + buttonY);

    // Label notes
    SetBkMode(hMemDC, TRANSPARENT);
    SetTextColor(hMemDC, RGB(0, 0, 0));
    SelectObject(hMemDC, hFbtn);
    rect.right = buttonX;
    rect.bottom = CmdBar_Height + buttonY;
    DrawTextW(hMemDC, app::wcMes[IDS_NUMBER], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  } else {
    // Label background
    SelectObject(hMemDC, hPenSys);
    SelectObject(hMemDC, hBshSys);
    Rectangle(hMemDC, 0, CmdBar_Height, buttonX, CmdBar_Height + buttonY);
    Rectangle(hMemDC, buttonX * 7 / 2, CmdBar_Height, buttonX * 9 / 2, CmdBar_Height + buttonY);
    Rectangle(hMemDC, 0, CmdBar_Height + buttonY, buttonX, CmdBar_Height + buttonY * 2);

    // Label notes
    SetBkMode(hMemDC, TRANSPARENT);
    SetTextColor(hMemDC, RGB(0, 0, 0));
    SelectObject(hMemDC, hFbtn);
    rect.right = buttonX;
    rect.bottom = CmdBar_Height + buttonY;
    DrawTextW(hMemDC, app::wcMes[IDS_LOWERBOUND], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    rect.left = buttonX * 7 / 2;
    rect.right = buttonX * 9 / 2;
    DrawTextW(hMemDC, app::wcMes[IDS_UPPERBOUND], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    rect.left = 0;
    rect.top = CmdBar_Height + buttonY;
    rect.bottom = CmdBar_Height + buttonY * 2;
    rect.right = buttonX;
    DrawTextW(hMemDC, app::wcMes[IDS_LIMIT], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  }

  // Central message
  SetBkMode(hMemDC, OPAQUE);
  SetBkColor(hMemDC, RGB(255, 255, 0));
  SetTextColor(hMemDC, RGB(0, 0, 255));
  SelectObject(hMemDC, hFmes);
  rect.left = 0;
  rect.right = scrx;
  if (app::mode == app::MODE_PF) {
    rect.top = CmdBar_Height + buttonY;
    rect.bottom = CmdBar_Height + scry * 9 / 40;
  } else {
    rect.top = CmdBar_Height + buttonY * 2;
    rect.bottom = CmdBar_Height + scry * 3 / 10;
  }
  DrawTextW(hMemDC,
            app::wcMes[app::isRunning              ? IDS_RUNNING
                       : app::mode == app::MODE_PF ? IDS_PFMSG
                                                   : IDS_PEMSG],
            -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

  InvalidateRect(hWnd, NULL, FALSE);  // Apply to the actual screen
}
}  // namespace msg
