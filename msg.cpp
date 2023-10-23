#include "msg.hpp"

#include "runner.hpp"
#include "wproc.hpp"

namespace msg {
static HDC hMemDC;  // Handle of a memory device context for double buffering
static HFONT hFmes = NULL, hFbtn = NULL, hFedi = NULL, hFnote = NULL;
static HBRUSH hBrush = NULL, hBshSys = GetSysColorBrush(COLOR_BTNFACE);
static HPEN hPen = NULL, hPenSys;
static int buttonX, buttonY;
static struct wproc::editorprops_t edit0Props, edit1Props, edit2Props;

void onActivate(HWND hWnd, unsigned int state, HWND hWndActDeact, BOOL fMinimized) {
  UNREFERENCED_PARAMETER(hWnd);
  UNREFERENCED_PARAMETER(hWndActDeact);
  UNREFERENCED_PARAMETER(fMinimized);

  // Re-sets the focus on the most recently focused edit control.
  if (state == WA_ACTIVE || state == WA_CLICKACTIVE) SetFocus(app::hFocused);
}

BOOL onCreate(HWND hWnd, CREATESTRUCTW *lpCreateStruct) {
  int scrx, scry;

  // Initialize menus
  SetMenu(hWnd, app::hMenu);
  if (GetUserDefaultUILanguage() == 0x0411) {  // Japanese platforms
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
  hPenSys = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));

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
  edit0Props.hNextWnd = NULL;
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

  HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
  MONITORINFOEXW mInfo;
  mInfo.cbSize = sizeof(MONITORINFOEXW);
  GetMonitorInfoW(hMonitor, &mInfo);
  if (mInfo.rcWork.bottom * 5 / 3 > mInfo.rcWork.right) {
    scrx = mInfo.rcWork.right * 3 / 4;
    scry = mInfo.rcWork.right * 9 / 20;
  } else {
    scrx = mInfo.rcWork.bottom * 10 / 9;
    scry = mInfo.rcWork.bottom * 2 / 3;
  }
  if (scrx < 800 || scry < 480) {
    scrx = 800;
    scry = 480;
  }
  SetWindowPos(hWnd, NULL, 0, 0, scrx, scry, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);

  return TRUE;
}

void onClose(HWND hWnd) {
  if (IDYES == util::messageBox(hWnd, app::hInst, app::wcMes[IDS_CFMQUIT], app::wcMes[IDS_CFMQUIT_TITLE],
                                MB_YESNO | MB_ICONINFORMATION)) {
    KillTimer(hWnd, 1);
    DestroyWindow(hWnd);
  } else {
    SetFocus(app::hFocused);
  }
}

void onTimer(HWND hWnd, unsigned int id) {
  UNREFERENCED_PARAMETER(id);

  static BYTE r = 0, g = 255, b = 255;  // Background color
  HWND hFocusedNow = GetFocus();

  // Gets the currently focused control.
  if (hFocusedNow && (hFocusedNow == app::hEdi0 || hFocusedNow == app::hEdi1 || hFocusedNow == app::hEdi2 ||
                      hFocusedNow == app::hEdiOut)) {
    app::hFocused = hFocusedNow;
  }

  if (b <= 0 && g < 255) g++;
  if (g >= 255 && r > 0) r--;
  if (r <= 0 && b < 255) b++;
  if (b >= 255 && g > 0) g--;
  if (g <= 0 && r < 255) r++;
  if (r >= 255 && b > 0) b--;

  if (hBrush) DeleteObject(hBrush);             // Delete the previous brush
  if (hPen) DeleteObject(hPen);                 // Delete the previous pen
  hBrush = CreateSolidBrush(RGB(r, g, b));      // Creates the new brush (for filling)
  hPen = CreatePen(PS_SOLID, 1, RGB(r, g, b));  // Creates the new pen (for outline)

  Paint(hWnd);  // Repaint with the updated background color
}

void onSize(HWND hWnd, unsigned int state, int cx, int cy) {
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
  rLogfont.lfQuality = DEFAULT_QUALITY;
  rLogfont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
  lstrcpyW(rLogfont.lfFaceName, L"MS Shell Dlg");  // Font name provided for internationalization
  if (hFmes) DeleteObject(hFmes);                  // Deletes the previous font
  hFmes = CreateFontIndirectW(&rLogfont);          // Creates the new font

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
  rLogfont.lfQuality = DEFAULT_QUALITY;
  rLogfont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
  lstrcpyW(rLogfont.lfFaceName, L"MS Shell Dlg");
  if (hFbtn) DeleteObject(hFbtn);
  hFbtn = CreateFontIndirectW(&rLogfont);

  // Font for notes (with just size modification from the button font)
  if (15 * scrx / 700 < 15 * scry / 400) {
    rLogfont.lfHeight = 15 * scrx / 700;
  } else {
    rLogfont.lfHeight = 15 * scry / 400;
  }
  if (hFnote) DeleteObject(hFnote);
  hFnote = CreateFontIndirectW(&rLogfont);

  // Font for edit boxes
  if (16 * scrx / 700 < 16 * scry / 400) {
    rLogfont.lfHeight = 16 * scrx / 700;
  } else {
    rLogfont.lfHeight = 16 * scry / 400;
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
  rLogfont.lfQuality = DEFAULT_QUALITY;
  rLogfont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
  lstrcpyW(rLogfont.lfFaceName, L"MS Shell Dlg");
  if (hFedi) DeleteObject(hFedi);
  hFedi = CreateFontIndirectW(&rLogfont);

  // Apply the new fonts
  SendMessageW(app::hBtnOK, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
  SendMessageW(app::hBtnAbort, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
  SendMessageW(app::hBtnClear, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
  SendMessageW(app::hEdi0, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
  SendMessageW(app::hEdiOut, WM_SETFONT, (WPARAM)hFedi, MAKELPARAM(FALSE, 0));
  if (app::mode == app::MODE_PE) {
    SendMessageW(app::hEdi1, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
    SendMessageW(app::hEdi2, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
  }

  // Move and resize controls
  buttonX = 64 * scrx / 700;
  buttonY = 32 * scry / 400;
  if (app::mode == app::MODE_PF) {
    MoveWindow(app::hEdi0, buttonX, 0, buttonX * 4, buttonY, TRUE);
    MoveWindow(app::hBtnOK, buttonX * 5, 0, buttonX, buttonY, TRUE);
    MoveWindow(app::hBtnAbort, buttonX * 6, 0, buttonX, buttonY, TRUE);
    MoveWindow(app::hBtnClear, buttonX * 7, 0, buttonX * 5 / 2, buttonY, TRUE);
    MoveWindow(app::hEdiOut, scrx / 20, scry * 9 / 40, scrx * 9 / 10, scry * 29 / 40, TRUE);
  } else {
    MoveWindow(app::hEdi0, buttonX, 0, buttonX * 4, buttonY, TRUE);
    MoveWindow(app::hEdi1, buttonX * 6, 0, buttonX * 4, buttonY, TRUE);
    MoveWindow(app::hEdi2, buttonX, buttonY, buttonX * 2, buttonY, TRUE);
    MoveWindow(app::hBtnOK, buttonX * 3, buttonY, buttonX, buttonY, TRUE);
    MoveWindow(app::hBtnAbort, buttonX * 4, buttonY, buttonX, buttonY, TRUE);
    MoveWindow(app::hBtnClear, buttonX * 5, buttonY, buttonX * 5 / 2, buttonY, TRUE);
    MoveWindow(app::hEdiOut, scrx / 20, scry * 3 / 10, scrx * 9 / 10, scry * 13 / 20, TRUE);
  }

  if (hBitmap) DeleteObject(hBitmap);  // Deletes the previous bitmap
  HDC hdc = GetDC(hWnd);
  if (!(hBitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom))) {  // Creates a bitmap with the new size
    util::messageBox(hWnd, app::hInst, app::wcMes[IDS_ESCRBUF], app::wcMes[IDS_ERROR], MB_OK | MB_ICONERROR);
    PostQuitMessage(1);
    return;
  }
  ReleaseDC(hWnd, hdc);
  SelectObject(hMemDC, hBitmap);  // Set the new bitmap to the memory device context

  Paint(hWnd);  // Repaint with new size
}

void onPaint(HWND hWnd) {
  RECT rect;
  PAINTSTRUCT ps;

  GetClientRect(hWnd, &rect);

  HDC hdc = BeginPaint(hWnd, &ps);
  BitBlt(hdc, 0, 0, rect.right, rect.bottom, hMemDC, 0, 0, SRCCOPY);  // Copies the double bufferred content
  EndPaint(hWnd, &ps);
}

void onCommand(HWND hWnd, int id, HWND hWndCtl, unsigned int codeNotify) {
  UNREFERENCED_PARAMETER(hWndCtl);
  UNREFERENCED_PARAMETER(codeNotify);

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
#ifndef UNICODE
      util::messageBox(hWnd, app::hInst, app::wcMes[IDS_ANSI], app::wcMes[IDS_INFO], MB_OK | MB_ICONINFORMATION);
#endif
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

#ifdef UNICODE
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
#else
      if (WriteFile(hFile, tcEdit, (editlen - 1) * sizeof(char), &dwTemp, NULL)) {  // ANSI ビルドなら変換せずに書き込み
#endif
        util::messageBox(hWnd, app::hInst, app::wcMes[IDS_FILEDONE], app::wcMes[IDS_INFO], MB_OK | MB_ICONINFORMATION);
      } else {
        util::messageBox(hWnd, app::hInst, app::wcMes[IDS_EWRITE], app::wcMes[IDS_ERROR], MB_OK | MB_ICONWARNING);
      }

      CloseHandle(hFile);
      HeapFree(hHeap, 0, ofn->lpstrFile);
      HeapFree(hHeap, 0, ofn);
      HeapFree(hHeap, 0, wcEdit);
#ifdef UNICODE
      HeapFree(hHeap, 0, mbEdit);
#endif
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

    case IDM_OPT_PF: {  // Switch to Prime Factorization
      if (app::mode == app::MODE_PF) break;

      CheckMenuRadioItem(app::hMenu, IDM_OPT_PF, IDM_OPT_PE, IDM_OPT_PF, MF_BYCOMMAND);
      EnableMenuItem(app::hMenu, IDM_OPT_CNTONLY, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(app::hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_GRAYED);

      SetFocus(app::hEdi0);

      DestroyWindow(app::hEdi1);  // Remove unnecessary edit boxes
      DestroyWindow(app::hEdi2);

      edit0Props.hNextWnd = NULL;
      edit0Props.runOnEnter = true;

      app::mode = app::MODE_PF;

      RECT rect;
      GetClientRect(hWnd, &rect);

      PostMessageW(hWnd, WM_SIZE, 0, MAKEWPARAM(rect.right, rect.bottom));  // Applies the new layout
      break;
    }

    case IDM_OPT_PE: {  // Switch to Enumerate Prime Numbers
      if (app::mode == app::MODE_PE) break;
      CheckMenuRadioItem(app::hMenu, IDM_OPT_PF, IDM_OPT_PE, IDM_OPT_PE, MF_BYCOMMAND);
      EnableMenuItem(app::hMenu, IDM_OPT_CNTONLY, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(app::hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_ENABLED);

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

      edit0Props.hNextWnd = app::hEdi1;
      edit1Props.hNextWnd = app::hEdi2;
      edit2Props.hNextWnd = NULL;

      edit0Props.runOnEnter = false;
      edit1Props.runOnEnter = false;
      edit2Props.runOnEnter = true;

      edit1Props.defProc = (WNDPROC)mySetWindowLongW(app::hEdi1, GWL_WNDPROC, (LONG_PTR)wproc::inputProc);
      mySetWindowLongW(app::hEdi1, GWL_USERDATA, (LONG_PTR)&edit1Props);
      edit2Props.defProc = (WNDPROC)mySetWindowLongW(app::hEdi2, GWL_WNDPROC, (LONG_PTR)wproc::inputProc);
      mySetWindowLongW(app::hEdi2, GWL_USERDATA, (LONG_PTR)&edit2Props);

      app::mode = app::MODE_PE;

      RECT rect;
      GetClientRect(hWnd, &rect);

      PostMessageW(hWnd, WM_SIZE, 0, MAKEWPARAM(rect.right, rect.bottom));  // Applies the new layout
      break;
    }

    case IDM_OPT_CNTONLY:
      if (GetMenuState(app::hMenu, IDM_OPT_CNTONLY, MF_BYCOMMAND) & MF_CHECKED) {  // Unchecks if checked
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
      if (GetMenuState(app::hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND) & MF_CHECKED) {  // Unchecks if checked
        CheckMenuItem(app::hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_UNCHECKED);
        app::useFile = false;
      } else {  // Checks if unchecked
        CheckMenuItem(app::hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_CHECKED);
        app::useFile = true;
      }
      break;

    case IDM_OPT_LANG_JA: {
      // Backs up the menu state
      unsigned int menu[5];
      menu[0] = GetMenuState(app::hMenu, IDM_OPT_PF, MF_BYCOMMAND);
      menu[1] = GetMenuState(app::hMenu, IDM_OPT_CNTONLY, MF_BYCOMMAND);
      menu[2] = GetMenuState(app::hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND);
      menu[3] = GetMenuState(app::hMenu, IDM_OPT_CHARSET_UTF8, MF_BYCOMMAND);

      DestroyMenu(app::hMenu);
      app::hMenu = LoadMenuW(app::hInst, L"ResMenu_JA");
      SetMenu(hWnd, app::hMenu);

      CheckMenuRadioItem(app::hMenu, IDM_OPT_LANG_JA, IDM_OPT_LANG_EN, IDM_OPT_LANG_JA, MF_BYCOMMAND);

      // Restores checkboxes and radio buttons
      if (menu[0] & MF_CHECKED) {
        CheckMenuRadioItem(app::hMenu, IDM_OPT_PF, IDM_OPT_PE, IDM_OPT_PF, MF_BYCOMMAND);
      } else {
        CheckMenuRadioItem(app::hMenu, IDM_OPT_PF, IDM_OPT_PE, IDM_OPT_PE, MF_BYCOMMAND);
      }
      if (menu[1] & MF_CHECKED) CheckMenuItem(app::hMenu, IDM_OPT_CNTONLY, MF_BYCOMMAND | MF_CHECKED);
      if (menu[2] & MF_CHECKED) CheckMenuItem(app::hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_CHECKED);
      if (menu[3] & MF_CHECKED) {
        CheckMenuRadioItem(app::hMenu, IDM_OPT_CHARSET_UTF8, IDM_OPT_CHARSET_SJIS, IDM_OPT_CHARSET_UTF8, MF_BYCOMMAND);
      } else {
        CheckMenuRadioItem(app::hMenu, IDM_OPT_CHARSET_UTF8, IDM_OPT_CHARSET_SJIS, IDM_OPT_CHARSET_SJIS, MF_BYCOMMAND);
      }

      // Restores the enablement statuses
      if (menu[1] & MF_GRAYED) EnableMenuItem(app::hMenu, IDM_OPT_CNTONLY, MF_BYCOMMAND | MF_GRAYED);
      if (menu[2] & MF_GRAYED) EnableMenuItem(app::hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_GRAYED);

      for (int i = 0; i < SIZE_OF_STRING_TABLE; i++) {  // Loads the Japanese String Table
        LoadStringW(app::hInst, IDS_JA + i, app::wcMes[i], sizeof(app::wcMes[0]) / sizeof(app::wcMes[0][0]));
      }

      SetWindowTextW(app::hBtnOK, app::wcMes[IDS_OK]);
      SetWindowTextW(app::hBtnAbort, app::wcMes[IDS_ABORT]);
      SetWindowTextW(app::hBtnClear, app::wcMes[IDS_CLRHST]);
      SetWindowTextW(hWnd, app::wcMes[IDS_APPNAME]);
      break;
    }

    case IDM_OPT_LANG_EN: {
      // Backs up the menu state
      unsigned int menu[5];
      menu[0] = GetMenuState(app::hMenu, IDM_OPT_PF, MF_BYCOMMAND);
      menu[1] = GetMenuState(app::hMenu, IDM_OPT_CNTONLY, MF_BYCOMMAND);
      menu[2] = GetMenuState(app::hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND);
      menu[3] = GetMenuState(app::hMenu, IDM_OPT_CHARSET_UTF8, MF_BYCOMMAND);

      DestroyMenu(app::hMenu);
      app::hMenu = LoadMenuW(app::hInst, L"ResMenu_EN");
      SetMenu(hWnd, app::hMenu);

      CheckMenuRadioItem(app::hMenu, IDM_OPT_LANG_JA, IDM_OPT_LANG_EN, IDM_OPT_LANG_EN, MF_BYCOMMAND);

      // Restores checkboxes and radio buttons
      if (menu[0] & MF_CHECKED) {
        CheckMenuRadioItem(app::hMenu, IDM_OPT_PF, IDM_OPT_PE, IDM_OPT_PF, MF_BYCOMMAND);
      } else {
        CheckMenuRadioItem(app::hMenu, IDM_OPT_PF, IDM_OPT_PE, IDM_OPT_PE, MF_BYCOMMAND);
      }
      if (menu[1] & MF_CHECKED) CheckMenuItem(app::hMenu, IDM_OPT_CNTONLY, MF_BYCOMMAND | MF_CHECKED);
      if (menu[2] & MF_CHECKED) CheckMenuItem(app::hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_CHECKED);
      if (menu[3] & MF_CHECKED) {
        CheckMenuRadioItem(app::hMenu, IDM_OPT_CHARSET_UTF8, IDM_OPT_CHARSET_SJIS, IDM_OPT_CHARSET_UTF8, MF_BYCOMMAND);
      } else {
        CheckMenuRadioItem(app::hMenu, IDM_OPT_CHARSET_UTF8, IDM_OPT_CHARSET_SJIS, IDM_OPT_CHARSET_SJIS, MF_BYCOMMAND);
      }

      // Restores the enablement statuses
      if (menu[1] & MF_GRAYED) EnableMenuItem(app::hMenu, IDM_OPT_CNTONLY, MF_BYCOMMAND | MF_GRAYED);
      if (menu[2] & MF_GRAYED) EnableMenuItem(app::hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_GRAYED);

      for (int i = 0; i < SIZE_OF_STRING_TABLE; i++) {  // Loads the English String Table
        LoadStringW(app::hInst, IDS_EN + i, app::wcMes[i], sizeof(app::wcMes[0]) / sizeof(app::wcMes[0][0]));
      }

      SetWindowTextW(app::hBtnOK, app::wcMes[IDS_OK]);
      SetWindowTextW(app::hBtnAbort, app::wcMes[IDS_ABORT]);
      SetWindowTextW(app::hBtnClear, app::wcMes[IDS_CLRHST]);
      SetWindowTextW(hWnd, app::wcMes[IDS_APPNAME]);
      break;
    }

    case IDM_OPT_CHARSET_UTF8:
      if (app::charset == IDM_OPT_CHARSET_UTF8) break;

      CheckMenuRadioItem(app::hMenu, IDM_OPT_CHARSET_UTF8, IDM_OPT_CHARSET_SJIS, IDM_OPT_CHARSET_UTF8, MF_BYCOMMAND);
      app::charset = IDM_OPT_CHARSET_UTF8;
      break;

    case IDM_OPT_CHARSET_SJIS:
      if (app::charset == IDM_OPT_CHARSET_SJIS) break;

      CheckMenuRadioItem(app::hMenu, IDM_OPT_CHARSET_UTF8, IDM_OPT_CHARSET_SJIS, IDM_OPT_CHARSET_SJIS, MF_BYCOMMAND);
      app::charset = IDM_OPT_CHARSET_SJIS;
      break;

    case IDM_HELP_HOWTOUSE:
      util::messageBox(hWnd, app::hInst, app::wcMes[app::mode == app::MODE_PF ? IDS_PFHELP : IDS_PEHELP],
                       app::wcMes[IDS_HELP_TITLE], MB_OK | MB_ICONINFORMATION);
      break;

    case IDM_HELP_ABOUT: {
      wchar_t wcTemp[MAX_BUFFER];
      wsprintfW(wcTemp, L"%s\n\n%s" __DATE__ L" " __TIME__ L"\n\n%s", app::wcMes[IDS_ABOUT], app::wcMes[IDS_BUILD],
                app::wcMes[IDS_COPYRIGHT]);
      util::messageBox(hWnd, app::hInst, wcTemp, app::wcMes[IDS_ABOUT_TITLE], MB_OK | MB_ICONINFORMATION);
      break;
    }
  }

  // Re-focuses on one of the edit boxes when a window or a control other than edit is focused
  if (id < IDC_EDIT_IN1) SetFocus(app::hFocused);
}

// 再描画
void Paint(HWND hWnd) {
  int scrx, scry;
  RECT rect;

  GetClientRect(hWnd, &rect);
  scrx = rect.right;
  scry = rect.bottom;

  SelectObject(hMemDC, hPen);    // Sets a pen to the memory device context
  SelectObject(hMemDC, hBrush);  // Sets a brush to the memory device context
  Rectangle(hMemDC, rect.left, rect.top, rect.right, rect.bottom);

  if (app::mode == app::MODE_PF) {
    // Label background
    SelectObject(hMemDC, hPenSys);
    SelectObject(hMemDC, hBshSys);
    Rectangle(hMemDC, 0, 0, buttonX, buttonY);

    // Label notes
    SetBkMode(hMemDC, TRANSPARENT);
    SetTextColor(hMemDC, RGB(0, 0, 0));
    SelectObject(hMemDC, hFnote);

    rect.right = buttonX;
    rect.bottom = buttonY;
    DrawTextW(hMemDC, app::wcMes[IDS_NUMBER], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  } else {
    // Label background
    SelectObject(hMemDC, hPenSys);
    SelectObject(hMemDC, hBshSys);
    Rectangle(hMemDC, 0, 0, buttonX, buttonY);
    Rectangle(hMemDC, buttonX * 5, 0, buttonX * 6, buttonY);
    Rectangle(hMemDC, 0, buttonY, buttonX, buttonY * 2);

    // Label notes
    SetBkMode(hMemDC, TRANSPARENT);
    SetTextColor(hMemDC, RGB(0, 0, 0));
    SelectObject(hMemDC, hFnote);

    rect.right = buttonX;
    rect.bottom = buttonY;
    DrawTextW(hMemDC, app::wcMes[IDS_LOWERBOUND], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    rect.left = buttonX * 5;
    rect.right = buttonX * 6;
    DrawTextW(hMemDC, app::wcMes[IDS_UPPERBOUND], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    rect.left = 0;
    rect.top = buttonY;
    rect.bottom = buttonY * 2;
    rect.right = buttonX;
    DrawTextW(hMemDC, app::wcMes[IDS_LIMIT], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  }

  SetBkMode(hMemDC, OPAQUE);
  SetBkColor(hMemDC, RGB(255, 255, 0));
  SetTextColor(hMemDC, RGB(0, 0, 255));
  SelectObject(hMemDC, hFmes);

  rect.left = 0;
  rect.right = scrx;
  if (app::mode == app::MODE_PF) {
    rect.top = buttonY;
    rect.bottom = scry * 9 / 40;
  } else {
    rect.top = buttonY * 2;
    rect.bottom = scry * 3 / 10;
  }

  // Central message
  DrawTextW(hMemDC,
            app::wcMes[app::isRunning              ? IDS_RUNNING
                       : app::mode == app::MODE_PF ? IDS_PFMSG
                                                   : IDS_PEMSG],
            -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

  InvalidateRect(hWnd, NULL, FALSE);  // Apply to the actual screen
}
}  // namespace msg
