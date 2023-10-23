#include "wproc.hpp"

#include <windowsx.h>

#include "msg.hpp"
#include "runner.hpp"
#include "ui.hpp"

namespace wproc {
LRESULT CALLBACK wndProc(HWND hWnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    HANDLE_MSG(hWnd, WM_ACTIVATE, msg::onActivate);  // On window activation
    HANDLE_MSG(hWnd, WM_CREATE, msg::onCreate);      // On window creation
    HANDLE_MSG(hWnd, WM_CLOSE, msg::onClose);        // On window close
    HANDLE_MSG(hWnd, WM_TIMER, msg::onTimer);        // On timer ring
    HANDLE_MSG(hWnd, WM_SIZE, msg::onSize);          // On window size change
    HANDLE_MSG(hWnd, WM_PAINT, msg::onPaint);        // On window repaint request
    HANDLE_MSG(hWnd, WM_COMMAND, msg::onCommand);    // On user interrtuption

#ifndef UNDER_CE
    case 0x02e0:  // On DPI change (WM_DPICHANGED)
      MoveWindow(hWnd, ((RECT *)lParam)->left, ((RECT *)lParam)->top, ((RECT *)lParam)->right - ((RECT *)lParam)->left,
                 ((RECT *)lParam)->bottom - ((RECT *)lParam)->top, FALSE);
      return 0;
#endif

    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;

    case WM_APP_THREADEND: {  // On runner thread exit (user defined message)
      DWORD exitCode;
      WaitForSingleObject(runner::hThread, INFINITE);  // Waits for the thread to end
      GetExitCodeThread(runner::hThread, &exitCode);
      CloseHandle(runner::hThread);

      switch (exitCode) {
        case IDE_INVALID:  // The input was invalid
          if (app::mode == app::MODE_PF) {
            MessageBoxW(hWnd, app::wcMes[IDS_PFINVALID], app::wcMes[IDS_ERROR], MB_OK | MB_ICONWARNING);
            ui::appendOutput(app::wcMes[IDS_PFINVALID_OUT]);
          } else {
            MessageBoxW(hWnd, app::wcMes[IDS_PEINVALID], app::wcMes[IDS_ERROR], MB_OK | MB_ICONWARNING);
            ui::appendOutput(app::wcMes[IDS_PEINVALID_OUT]);
          }
          break;

        case IDE_ABORT:  // Calculation terminated (after starting)
          runner::isAborted = false;
          break;

        case IDE_CANCEL:  // Calculation canceled (before starting)
          break;

        case IDE_CANNOTOPENFILE:  // Couldn't open the output file
          MessageBoxW(hWnd, app::wcMes[IDS_EOPEN], app::wcMes[IDS_ERROR], MB_OK | MB_ICONWARNING);
          ui::appendOutput(app::wcMes[IDS_EOPEN_OUT]);
          break;

        case IDE_CANNOTWRITEFILE:  // Couldn't write on the output file
          MessageBoxW(hWnd, app::wcMes[IDS_EWRITE], app::wcMes[IDS_ERROR], MB_OK | MB_ICONWARNING);
          ui::appendOutput(app::wcMes[IDS_OUT_TITLE]);
      }

      // Sets the default window title on failure (all nonzero exit codes are error)
      if (exitCode) SetWindowTextW(hWnd, app::wcMes[IDS_APPNAME]);

      if (app::mode == app::MODE_PE) {
        SendMessageW(app::hEdi1, EM_SETREADONLY, (WPARAM)FALSE, (LPARAM)NULL);
        if (!app::countOnly) SendMessageW(app::hEdi2, EM_SETREADONLY, (WPARAM)FALSE, (LPARAM)NULL);
      }

      EnableWindow(app::hBtnOK, TRUE);
      SendMessageW(app::hEdi0, EM_SETREADONLY, (WPARAM)FALSE, (LPARAM)NULL);
      EnableWindow(app::hBtnAbort, FALSE);

      EnableMenuItem(app::hMenu, 2, MF_BYPOSITION | MF_ENABLED);  // Re-enable the "Options" menu
      DrawMenuBar(hWnd);

      msg::Paint(hWnd);

      SetFocus(app::hFocused);  // Sets a focus on the previously focused control

      app::isRunning = false;
      return 0;
    }
  }

  return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK inputProc(HWND hWnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam) {
  struct editorprops_t *props = (struct editorprops_t *)myGetWindowLongW(hWnd, GWL_USERDATA);
  LONG_PTR style = myGetWindowLongW(hWnd, GWL_STYLE);

  switch (uMsg) {
    case WM_CHAR:
      if ((style & ES_READONLY) == 0) {
        switch ((wchar_t)wParam) {
          case VK_RETURN:  // Enter
            if (props->runOnEnter) {
              runner::begin();
            } else if (props->hNextWnd) {
              SetFocus(props->hNextWnd);
            }
            return 0;

          case L'q':
            SendMessageW(hWnd, EM_REPLACESEL, 0, (WPARAM)L"1");
            return 0;

          case L'w':
            SendMessageW(hWnd, EM_REPLACESEL, 0, (WPARAM)L"2");
            return 0;

          case L'e':
            SendMessageW(hWnd, EM_REPLACESEL, 0, (WPARAM)L"3");
            return 0;

          case L'r':
            SendMessageW(hWnd, EM_REPLACESEL, 0, (WPARAM)L"4");
            return 0;

          case L't':
            SendMessageW(hWnd, EM_REPLACESEL, 0, (WPARAM)L"5");
            return 0;

          case L'y':
            SendMessageW(hWnd, EM_REPLACESEL, 0, (WPARAM)L"6");
            return 0;

          case L'u':
            SendMessageW(hWnd, EM_REPLACESEL, 0, (WPARAM)L"7");
            return 0;

          case L'i':
            SendMessageW(hWnd, EM_REPLACESEL, 0, (WPARAM)L"8");
            return 0;

          case L'o':
            SendMessageW(hWnd, EM_REPLACESEL, 0, (WPARAM)L"9");
            return 0;

          case L'p':
            SendMessageW(hWnd, EM_REPLACESEL, 0, (WPARAM)L"0");
            return 0;
        }
      }
      break;

    case WM_DESTROY:
      mySetWindowLongW(hWnd, GWL_WNDPROC, (LONG_PTR)props->defProc);  // Restore the default window procedure
      return 0;
  }

  return CallWindowProcW(props->defProc, hWnd, uMsg, wParam, lParam);
}
}  // namespace wproc
