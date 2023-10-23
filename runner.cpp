#include "runner.hpp"

#include "msg.hpp"
#include "ui.hpp"

namespace runner {
ULONGLONG num[3];
HANDLE hThread;
volatile bool isAborted = false;

void begin() {
  app::isRunning = true;

  // Disables things that are not available while calculating
  EnableWindow(app::hBtnOK, FALSE);
  SendMessageW(app::hEdi0, EM_SETREADONLY, (WPARAM)TRUE, (LPARAM)NULL);
  if (app::mode == app::MODE_PE) {
    SendMessageW(app::hEdi1, EM_SETREADONLY, (WPARAM)TRUE, (LPARAM)NULL);
    SendMessageW(app::hEdi2, EM_SETREADONLY, (WPARAM)TRUE, (LPARAM)NULL);
  }
  EnableWindow(app::hBtnAbort, TRUE);

  EnableMenuItem(app::hMenu, 2, MF_BYPOSITION | MF_GRAYED);
  DrawMenuBar(app::hWnd);

  wchar_t wcTemp[MAX_INPUT_LENGTH];
  SendMessageW(app::hEdi0, WM_GETTEXT, MAX_INPUT_LENGTH, (LPARAM)wcTemp);  // Gets input
  errno = 0;
  num[0] = wcstoull(wcTemp, NULL, 10);
  bool isErange = (errno == ERANGE);

  if (app::mode == app::MODE_PE) {
    SendMessageW(app::hEdi1, WM_GETTEXT, MAX_INPUT_LENGTH, (LPARAM)wcTemp);
    errno = 0;
    num[1] = wcstoull(wcTemp, NULL, 10);
    isErange = isErange || (errno == ERANGE);  // Records a wraparound with logical disjunction

    SendMessageW(app::hEdi2, WM_GETTEXT, MAX_INPUT_LENGTH, (LPARAM)wcTemp);
    errno = 0;
    num[2] = wcstoull(wcTemp, NULL, 10);
    isErange = isErange || (errno == ERANGE);
  }

  if (isErange)
    util::messageBox(app::hWnd, app::hInst, app::wcMes[IDS_WLARGE], app::wcMes[IDS_ETHREAD],
                     MB_OK | MB_ICONINFORMATION);

  SetWindowTextW(app::hWnd, app::wcMes[IDS_RUNNING_TITLE]);

  // Starts the calculation thread
  hThread =
      myCreateThread(NULL, 0, app::mode == app::MODE_PF ? runner::primeFactor : runner::primeEnumerator, NULL, 0, NULL);
  if (hThread) {
    SetThreadPriority(hThread, THREAD_PRIORITY_BELOW_NORMAL);  // Prevents to stress the system
  } else {
    util::messageBox(app::hWnd, app::hInst, app::wcMes[IDS_ETHREAD], app::wcMes[IDS_ERROR], MB_OK | MB_ICONWARNING);

    ui::appendOutput(app::wcMes[IDS_ETHREAD_OUT]);

    SetWindowTextW(app::hWnd, app::wcMes[IDS_APPNAME]);

    if (app::mode == app::MODE_PE) {
      SendMessageW(app::hEdi1, EM_SETREADONLY, (WPARAM)FALSE, (LPARAM)NULL);
      if (!app::countOnly) SendMessageW(app::hEdi2, EM_SETREADONLY, (WPARAM)FALSE, (LPARAM)NULL);
    }
    EnableWindow(app::hBtnOK, TRUE);
    SendMessageW(app::hEdi0, EM_SETREADONLY, (WPARAM)FALSE, (LPARAM)NULL);
    EnableWindow(app::hBtnAbort, FALSE);

    EnableMenuItem(app::hMenu, 2, MF_BYPOSITION | MF_ENABLED);  // Re-enables the Options menu
    DrawMenuBar(app::hWnd);

    msg::Paint(app::hWnd);

    SetFocus(app::hFocused);  // Restores the focus to the previous place

    app::isRunning = false;
  }
}

tret_t WINAPI primeFactor(void *lpParameter) {
  UNREFERENCED_PARAMETER(lpParameter);

  ULONGLONG N = num[0];  // Dividend (initialized with an input value)
  ULONGLONG cnt = 0;     // Number of prime factors
  ULONGLONG i = 2;       // Divisor (Candidate for a prime factor)
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
      if (N % 2 == 0 && N != 2) {  // Found a prime factor
        chk = true;
        goto after;
      }

      if (N / 2 < 2 || N == 2) {  // N is a prime number
        chk = false;
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
      wsprintfW(wcStr2, L"%I64ux", i);  // Converts the found prime factor to a string and appends "x"
      lstrcatW(wcStr1, wcStr2);         // Appends to the result
    } else {                            // N is a prime number
      wsprintfW(wcStr2, L"%I64u", N);   // Converts itself to a string
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

  wsprintfW(wcStr2, L"%s%I64u = %s", app::wcMes[IDS_PFRESULT], num[0], wcStr1);  // Constructs the result string
  ui::appendOutput(wcStr2);
  SendMessageW(app::hEdiOut, EM_REPLACESEL, 0, (WPARAM)L"\r\n");

  wsprintfW(wcStr1, L" - %s", app::wcMes[IDS_APPNAME]);  // Constructs the window title
  lstrcatW(wcStr2, wcStr1);
  SetWindowTextW(app::hWnd, wcStr2);

  PostMessageW(app::hWnd, WM_APP_THREADEND, 0, 0);
  return IDE_SUCCESS;
}

tret_t WINAPI primeEnumerator(void *lpParameter) {
  UNREFERENCED_PARAMETER(lpParameter);

  if (num[1] == 0) num[1] = ULLONG_MAX;
  if (num[2] == 0 || app::countOnly) num[2] = ULLONG_MAX;
  if (num[0] > num[1]) {
    PostMessageW(app::hWnd, WM_APP_THREADEND, 0, 0);
    return IDE_INVALID;
  }

  ULONGLONG lowerBound = num[0], upperBound = num[1], maxCount = num[2];
  HANDLE hFile = NULL;

  // Prepares for text file output
  if (!app::countOnly && app::useFile) {
#ifndef UNICODE
    util::messageBox(app::hWnd, app::hInst, app::wcMes[IDS_ANSI], app::wcMes[IDS_INFO], MB_OK | MB_ICONINFORMATION);
#endif

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
#ifdef UNICODE
    mbLen = WideCharToMultiByte(app::charset == IDM_OPT_CHARSET_SJIS ? 932 : 65001, 0, app::wcMes[IDS_PFRESULT], -1,
                                NULL, 0, NULL, NULL);
    WideCharToMultiByte(app::charset == IDM_OPT_CHARSET_SJIS ? 932 : 65001, 0, app::wcMes[IDS_PFRESULT], -1, mbStr,
                        mbLen, NULL, NULL);
    if (!WriteFile(hFile, mbStr, (mbLen - 1) * sizeof(char), &dwTemp, NULL)) {
#else
    if (!WriteFile(hFile, app::wcMes[IDS_PFRESULT], lstrlenA(app::wcMes[IDS_RUNNING]) * sizeof(char), &dwTemp, NULL)) {
#endif
      PostMessageW(app::hWnd, WM_APP_THREADEND, 0, 0);
      return IDE_CANNOTWRITEFILE;
    }
    ui::appendOutput(app::wcMes[IDS_RUNFILE]);
  }

  ULONGLONG cnt = 0;  // Number of prime numbers

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
  for (ULONGLONG i = lowerBound; i <= upperBound && cnt < maxCount; i += 2) {  // Prime number candidate
    for (ULONGLONG j = 3; j <= i && !isAborted; j += 2) {                      // Prime factor candidate
      if (i % j == 0 && i != j) break;                                         // Not a prime number
      if (i / j < j || i == j) {                                               // Prime number
        if (!app::countOnly) {                                                 // Does not output in count only mode
          if (cnt == 0) {                                                      // First output
            wsprintfW(wcStr1, L"%I64u", i);                                    // For edit box output
            if (app::useFile) wsprintfA(mbStr, "%I64u", i);                    // For file output
          } else {
            wsprintfW(wcStr1, L", %I64u", i);
            if (app::useFile) wsprintfA(mbStr, ", %I64u", i);
          }

          if (app::useFile) {
            WriteFile(hFile, mbStr, lstrlenA(mbStr) * sizeof(char), &dwTemp, NULL);
          } else {
            ui::appendOutput(wcStr1);
          }
        }
        ++cnt;
        break;
      }
    }
    if (isAborted || i == ULLONG_MAX) break;
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
#ifdef UNICODE
    mbLen = WideCharToMultiByte(app::charset == IDM_OPT_CHARSET_SJIS ? 932 : 65001, 0, wcStr2, lstrlenW(wcStr2), NULL,
                                0, NULL, NULL);
    WideCharToMultiByte(app::charset == IDM_OPT_CHARSET_SJIS ? 932 : 65001, 0, wcStr2, lstrlenW(wcStr2), mbStr, mbLen,
                        NULL, NULL);
    WriteFile(hFile, mbStr, mbLen * sizeof(char), &dwTemp, NULL);
#else
    WriteFile(hfile, tcStr2, lstrlenA(tcStr2) * sizeof(char), &dwtemp, NULL);
#endif
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
}  // namespace runner
