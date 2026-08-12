#include "runner.hpp"

#include <limits.h>
#ifndef LLONG_MAX
#define LLONG_MAX MAXLONGLONG
#endif

#ifdef __MINGW32CE__  // CeGCC
#define _wtoi64(text) wcstoll(text, NULL, 10)
#endif

#if defined UNDER_CE && __GNUC__ == 3  // Pocket GCC
#define lstrcatW wcscat
#define lstrlenW wcslen
#else
#include <cwchar>
#endif

#include "msg.hpp"
#include "ui.hpp"

namespace runner {
static LONGLONG num[3];
HANDLE hThread;
volatile bool isAborted = false;

static tret_t WINAPI primeFactor(void *lpParameter) {
  UNREFERENCED_PARAMETER(lpParameter);

  LONGLONG N = num[0];  // Dividend (initialized with an input value)
  LONGLONG cnt = 0;     // Number of prime factors
  LONGLONG i = 2;       // Divisor (Candidate for a prime factor)
  bool chk = false;
  wchar_t wcStr1[MAX_BUFFER] = L"", wcStr2[MAX_BUFFER] = L"";

  if (N <= 0) {
    // Notifies that this thread finished without waiting for being processed
    PostMessageW(app::hWnd, WM_APP_THREADEND, 0, 0);
    return IDE_INVALID;
  }

  // Prime factorization with the trial division algorithm
  while (true) {
    if (i == 2) {
      if (N == 2) {  // N is a prime number
        chk = false;
        goto after;
      }
      if (N % 2 == 0) {  // Found a prime factor
        chk = true;
        goto after;
      }
      i = 3;
    }
    for (; i <= N && !isAborted; i += 2) {
      if (N % i == 0 && N != i) {  // Found a prime factor
        chk = true;
        break;
      }
      if (N / i < i || N == i) {  // N is a prime number
        chk = false;
        break;
      }
    }
    if (isAborted) break;

  after:
    if (chk) {                          // Found a prime factor
      wsprintfW(wcStr2, L"%I64dx", i);  // Converts the found prime factor to a string and appends "x"
      lstrcatW(wcStr1, wcStr2);         // Appends to the result
    } else {                            // N is a prime number
      wsprintfW(wcStr2, L"%I64d", N);   // Converts itself to a string
      lstrcatW(wcStr1, wcStr2);         // Appends to the result
      break;
    }

    // Divides N with the found prime factor and continue (We don't need to initialize `i` since it never gets smaller)
    N /= i;
    ++cnt;
  }

  if (cnt == 0 && N > 1) lstrcatW(wcStr1, app::wcMes[IDS_PFPRIME]);  // Appends that the input is prime number

  if (isAborted) {
    PostMessageW(app::hWnd, WM_APP_THREADEND, 0, 0);
    return IDE_ABORT;
  }

  wsprintfW(wcStr2, L"%s%I64d = %s", app::wcMes[IDS_PFRESULT], num[0], wcStr1);  // Constructs the result string
  ui::appendOutput(wcStr2);
  SendMessageW(app::hEdiOut, EM_REPLACESEL, 0, (WPARAM)L"\r\n");

  wsprintfW(wcStr1, L" - %s", app::wcMes[IDS_APPNAME]);  // Constructs the window title
  lstrcatW(wcStr2, wcStr1);
  SetWindowTextW(app::hWnd, wcStr2);

  PostMessageW(app::hWnd, WM_APP_THREADEND, 0, 0);
  return IDE_SUCCESS;
}

static tret_t WINAPI primeEnumerator(void *lpParameter) {
  UNREFERENCED_PARAMETER(lpParameter);

  if (num[1] == 0) num[1] = LLONG_MAX;
  if (num[2] == 0 || app::countOnly) num[2] = LLONG_MAX;
  if (num[0] > num[1]) {
    PostMessageW(app::hWnd, WM_APP_THREADEND, 0, 0);
    return IDE_INVALID;
  }

  LONGLONG lowerBound = num[0], upperBound = num[1], maxCount = num[2];
  HANDLE hFile = NULL;

  // Prepares for text file output
  if (!app::countOnly && app::useFile) {
    wchar_t wcFile[MAX_PATH] = {0};
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = app::hWnd;
    ofn.lpstrFilter = L"Text File (*.txt)\0*.txt\0All files (*.*)\0*.*\0";
    ofn.lpstrFile = wcFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrDefExt = L".txt";
    ofn.lpstrTitle = app::wcMes[IDS_OUT_TITLE];
    ofn.Flags = OFN_OVERWRITEPROMPT;
    if (!GetSaveFileNameW(&ofn)) {
      PostMessageW(app::hWnd, WM_APP_THREADEND, 0, 0);
      return IDE_CANCEL;
    }

    hFile = CreateFileW(wcFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
      PostMessageW(app::hWnd, WM_APP_THREADEND, 0, 0);
      return IDE_CANNOTOPENFILE;
    }
  }

  DWORD dwTemp;
  int mbLen;               // Length of a charset converted string including a null terminator
  char mbStr[MAX_BUFFER];  // Charset converted string

  if (!app::countOnly && !app::useFile) {
    ui::appendOutput(app::wcMes[IDS_PFRESULT]);
  } else if (!app::countOnly && app::useFile) {
    mbLen = WideCharToMultiByte(app::charset == IDM_OPT_CHARSET_SJIS ? 932 : 65001, 0, app::wcMes[IDS_PFRESULT], -1,
                                NULL, 0, NULL, NULL);
    WideCharToMultiByte(app::charset == IDM_OPT_CHARSET_SJIS ? 932 : 65001, 0, app::wcMes[IDS_PFRESULT], -1, mbStr,
                        mbLen, NULL, NULL);
    if (!WriteFile(hFile, mbStr, (mbLen - 1) * sizeof(char), &dwTemp, NULL)) {
      PostMessageW(app::hWnd, WM_APP_THREADEND, 0, 0);
      return IDE_CANNOTWRITEFILE;
    }
    ui::appendOutput(app::wcMes[IDS_RUNFILE]);
  }

  LONGLONG cnt = 0;  // Number of prime numbers

  // Adjusts inputs (handles the cases of blank or 0)
  if (lowerBound <= 2 && upperBound >= 2) {
    if (!app::countOnly) {
      if (app::useFile) {
        WriteFile(hFile, "2", 1, &dwTemp, NULL);
      } else {
        ui::appendOutput(L"2");
      }
    }
    ++cnt;
  }
  if (lowerBound <= 2) lowerBound = 3;
  if (lowerBound % 2 == 0) ++lowerBound;

  wchar_t wcStr1[MAX_BUFFER], wcStr2[MAX_BUFFER];

  // Prime enumeration with the trial division algorithm
  LONGLONG i, j;
  for (i = lowerBound; i <= upperBound && cnt < maxCount; i += 2) {  // Prime number candidate
    for (j = 3; j <= i && !isAborted; j += 2) {                      // Prime factor candidate
      if (i % j == 0 && i != j) break;                               // Not a prime number
      if (i / j < j || i == j) {                                     // Prime number
        if (!app::countOnly) {                                       // Does not output in count only mode
          wsprintfW(wcStr1, cnt ? L", %I64d" : L"%I64d", i);
          if (app::useFile) {
            mbLen = WideCharToMultiByte(app::charset == IDM_OPT_CHARSET_SJIS ? 932 : 65001, 0, wcStr1, lstrlenW(wcStr1),
                                        NULL, 0, NULL, NULL);
            WideCharToMultiByte(app::charset == IDM_OPT_CHARSET_SJIS ? 932 : 65001, 0, wcStr1, lstrlenW(wcStr1), mbStr,
                                mbLen, NULL, NULL);
            WriteFile(hFile, mbStr, mbLen, &dwTemp, NULL);
          } else {
            ui::appendOutput(wcStr1);
          }
        }
        ++cnt;
        break;
      }
    }
    if (isAborted || i == LLONG_MAX) break;
  }

  if (!app::countOnly && !app::useFile) {
    ui::appendOutput(L"\r\n");
  } else if (!app::countOnly && app::useFile) {
    WriteFile(hFile, "\r\n", 2 * sizeof(char), &dwTemp, NULL);
  }

  wsprintfW(wcStr2, app::wcMes[isAborted ? IDS_PEABORT : IDS_PERESULT], cnt, num[0], num[1],
            num[2]);  // Constructs the result
  ui::appendOutput(wcStr2);
  SendMessageW(app::hEdiOut, EM_REPLACESEL, 0, (WPARAM)L"\r\n");

  if (!app::countOnly && app::useFile) {
    mbLen = WideCharToMultiByte(app::charset == IDM_OPT_CHARSET_SJIS ? 932 : 65001, 0, wcStr2, lstrlenW(wcStr2), NULL,
                                0, NULL, NULL);
    WideCharToMultiByte(app::charset == IDM_OPT_CHARSET_SJIS ? 932 : 65001, 0, wcStr2, lstrlenW(wcStr2), mbStr, mbLen,
                        NULL, NULL);
    WriteFile(hFile, mbStr, mbLen * sizeof(char), &dwTemp, NULL);
    CloseHandle(hFile);
  }

  if (isAborted) {
    PostMessageW(app::hWnd, WM_APP_THREADEND, 0, 0);
    return IDE_ABORT;
  }

  wsprintfW(wcStr1, L" - %s", app::wcMes[IDS_APPNAME]);
  lstrcatW(wcStr2, wcStr1);
  SetWindowTextW(app::hWnd, wcStr2);

  PostMessageW(app::hWnd, WM_APP_THREADEND, 0, 0);
  return IDE_SUCCESS;
}

void begin() {
  wchar_t wcTemp[MAX_INPUT_LENGTH];
  SendMessageW(app::hEdi0, WM_GETTEXT, MAX_INPUT_LENGTH, (LPARAM)wcTemp);  // Gets input
  num[0] = _wtoi64(wcTemp);
  if (app::mode == app::MODE_PE) {
    SendMessageW(app::hEdi1, WM_GETTEXT, MAX_INPUT_LENGTH, (LPARAM)wcTemp);
    num[1] = _wtoi64(wcTemp);
    SendMessageW(app::hEdi2, WM_GETTEXT, MAX_INPUT_LENGTH, (LPARAM)wcTemp);
    num[2] = _wtoi64(wcTemp);
  }

  // Starts the calculation thread
  hThread =
      myCreateThread(NULL, 0, app::mode == app::MODE_PF ? runner::primeFactor : runner::primeEnumerator, NULL, 0, NULL);
  if (hThread) {
    SetThreadPriority(hThread, THREAD_PRIORITY_BELOW_NORMAL);  // Prevents system freeze

    // Disables things that are not available while calculating
    EnableWindow(app::hBtnOK, FALSE);
    SendMessageW(app::hEdi0, EM_SETREADONLY, (WPARAM)TRUE, (LPARAM)NULL);
    if (app::mode == app::MODE_PE) {
      SendMessageW(app::hEdi1, EM_SETREADONLY, (WPARAM)TRUE, (LPARAM)NULL);
      SendMessageW(app::hEdi2, EM_SETREADONLY, (WPARAM)TRUE, (LPARAM)NULL);
    }
    EnableWindow(app::hBtnAbort, TRUE);
    EnableMenuItem(app::hMenu, 2, MF_BYPOSITION | MF_GRAYED);
#ifdef UNDER_CE
    CommandBar_DrawMenuBar(app::hCmdBar, 1);
#else
    DrawMenuBar(app::hWnd);
#endif
    SetWindowTextW(app::hWnd, app::wcMes[IDS_RUNNING_TITLE]);
    app::isRunning = true;
    msg::redraw(app::hWnd);
  } else {
    util::messageBox(app::hWnd, app::hInst, app::wcMes[IDS_ETHREAD], app::wcMes[IDS_ERROR], MB_OK | MB_ICONWARNING);
    ui::appendOutput(app::wcMes[IDS_ETHREAD_OUT]);
  }
}
}  // namespace runner
