#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include "defproc.h"

#ifdef _WIN64
#define TARGET_PLATFORM TEXT("Win64")
#ifndef _AArch64 // コンパイルオプションで手動定義
#define TARGET_CPU TEXT("AMD64/Intel64(x86_64)")
#else
#define TARGET_CPU TEXT("AArch64(ARM64)")
#endif
#elif defined _WIN32
#define TARGET_PLATFORM TEXT("Win32")
#ifndef _AArch32 // コンパイルオプションで手動定義
#define TARGET_CPU TEXT("IA-32(x86)")
#else
#define TARGET_CPU TEXT("AArch32(ARM32)")
#endif
#endif
#define COMPILER_NAME TEXT("Visual Studio 2019")
#define WND_CLASS_NAME TEXT("Prime_Factorization_Main")
#define APP_THREADEND WM_APP // WM_APP から 0xBFFF までは自作メッセージとして使える

enum {
    IDC_BUTTON_OK = 0,
    IDC_BUTTON_ABORT = 1,
    IDC_BUTTON_CLEAR = 2,
    IDC_EDIT_IN1 = 20,
    IDC_EDIT_IN2 = 21,
    IDC_EDIT_IN3 = 22,
    IDC_EDIT_OUT = 23,
    IDM_FILE_SAVE_AS = 100,
    IDM_FILE_EXIT = 101,
    IDM_EDIT_CUT = 120,
    IDM_EDIT_COPY = 121,
    IDM_EDIT_PASTE = 122,
    IDM_EDIT_SELECT_ALL = 123,
    IDM_OPT_PF = 140,
    IDM_OPT_LCPN = 141,
    IDM_OPT_ONLYNUM = 142,
    IDM_OPT_OUTFILE = 143,
    IDM_OPT_LANG_JA = 145,
    IDM_OPT_LANG_EN = 146,
    IDM_OPT_CHARSET_UTF8 = 147,
    IDM_OPT_CHARSET_SJIS = 148,
    IDM_HELP_HOWTOUSE = 160,
    IDM_HELP_ABOUT = 161,
    IDS_JA = 0,
    IDS_EN = 100,
    IDE_SUCCESS = 0,
    IDE_INVALID = 1,
    IDE_ABORT = 2,
    IDE_CANCEL = 3,
    IDE_CANNOTOPENFILE = 4,
    IDE_CANNOTWRITEFILE = 5,
    SIZE_OF_STRING_TABLE = 48,
    MAX_INPUT_LENGTH = 21,
    MAX_OUTPUT_BUFFER = 65536,
    MAX_BUFFER = 1024
};

struct PFSTATUS {
    unsigned int mode: 1;
    unsigned int onlycnt: 1;
    unsigned int usefile: 1;
    unsigned int charset: 1;
    unsigned int isWorking: 1;
} status = {0};

HWND hBtn_ok, hBtn_abort, hBtn_clr, hEdi0, hEdi1, hEdi2, hEdi_out, hWnd_focused;
HINSTANCE hInst; // Instance Handle のバックアップ
HMENU hMenu;
HDC hMemDC;
HFONT hFmes = NULL, hFbtn = NULL, hFedi = NULL, hFnote = NULL; // 作成するフォント
HBRUSH hBrush = NULL, hBshSys; // 取得するブラシ
HPEN hPen = NULL, hPenSys; // 取得するペン
HANDLE hThread;
INT btnsize[2];
ULONGLONG num[3]; // 入力値受付用変数
volatile bool isAborted = false;
TCHAR tcTemp[MAX_BUFFER], tcMes[SIZE_OF_STRING_TABLE][MAX_BUFFER];

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK Edit0WindowProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK Edit1WindowProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK Edit2WindowProc(HWND, UINT, WPARAM, LPARAM);
BOOL Main_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
void Main_OnClose(HWND hwnd);
void Main_OnTimer(HWND hwnd, UINT id);
void Main_OnSize(HWND hwnd, UINT state, int cx, int cy);
void Main_OnPaint(HWND hwnd);
void Main_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
void Paint(HWND hwnd);
void StartCalc(HWND hwnd);
DWORD WINAPI PrimeFactorization(LPVOID);
DWORD WINAPI ListPrimeNumbers(LPVOID);

void OutputToEditbox(HWND hwnd, LPCTSTR arg) { // エディットボックスの末尾に文字列を追加
    INT editlen = (INT)SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0);
    SendMessage(hwnd, EM_SETSEL, editlen, editlen);
    SendMessage(hwnd, EM_REPLACESEL, 0, (WPARAM)arg);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    hInst = hInstance;

    if(GetUserDefaultUILanguage() == 0x0411) { // UI が日本語なら日本語リソースを読み込む
        for(int i=0; i<SIZE_OF_STRING_TABLE; i++)
            LoadString(hInstance, i+IDS_JA, tcMes[i], sizeof(tcMes[0])/sizeof(tcMes[0][0]));
        hMenu = LoadMenu(hInst, TEXT("ResMenu_JA"));
    } else { // それ以外なら英語リソースを読み込む
        for(int i=0; i<SIZE_OF_STRING_TABLE; i++)
            LoadString(hInstance, i+IDS_EN, tcMes[i], sizeof(tcMes[0])/sizeof(tcMes[0][0]));
        hMenu = LoadMenu(hInst, TEXT("ResMenu_EN"));
    }
    HACCEL hAccel = LoadAccelerators(hInstance, TEXT("ResAccel")); // ショートカットキー読み込み

    wsprintf(tcTemp, TEXT("%s - %s"), tcMes[0], TARGET_CPU); // タイトル生成

    WNDCLASSEX wcl;
    wcl.cbSize = sizeof(WNDCLASSEX);
    wcl.hInstance = hInstance;
    wcl.lpszClassName = WND_CLASS_NAME;
    wcl.lpfnWndProc = WindowProc;
    wcl.style = 0;
    wcl.hIcon = LoadIcon(hInstance, TEXT("ResIcon")); // アイコンを読み込む
    wcl.hIconSm = LoadIcon(hInstance, TEXT("ResIcon")); // 小アイコンを読み込む
    wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcl.lpszMenuName = 0;
    wcl.cbClsExtra = 0;
    wcl.cbWndExtra = 0;
    wcl.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    if(!RegisterClassEx(&wcl)) {
        MessageBox(NULL, tcMes[43], tcMes[19], MB_OK | MB_ICONERROR);
        return 1;
    }

    // WM_SIZE でサイズ調整するので、生成時は適当でいい
    HWND hWnd = CreateWindowEx( // メインウィンドウを生成
        0,
        WND_CLASS_NAME,
        tcTemp,
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, // WS_CLIPCHILDREN で子ウィンドウ領域を描画対象から外す(ちらつき防止)
        CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
        NULL,
        NULL,
        hInstance,
        NULL
    );
    if(!hWnd){
        MessageBox(NULL, tcMes[44], tcMes[19], MB_OK | MB_ICONERROR);
        return 1;
    }

    hBtn_ok = CreateWindowEx( // OKボタン
        0,
        TEXT("BUTTON"),
        tcMes[9],
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 0, 0,
        hWnd,
        (HMENU)IDC_BUTTON_OK,
        hInstance,
        NULL);
    if(!hBtn_ok) {
        MessageBox(hWnd, tcMes[44], tcMes[19], MB_OK | MB_ICONERROR);
        return 1;
    }

    hBtn_abort = CreateWindowEx( // 中断ボタン
        0,
        TEXT("BUTTON"),
        tcMes[10],
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_DISABLED,
        0, 0, 0, 0,
        hWnd,
        (HMENU)IDC_BUTTON_ABORT,
        hInstance,
        NULL);
    if(!hBtn_abort) {
        MessageBox(hWnd, tcMes[44], tcMes[19], MB_OK | MB_ICONERROR);
        return 1;
    }

    hBtn_clr = CreateWindowEx( // 履歴消去ボタン
        0,
        TEXT("BUTTON"),
        tcMes[11],
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 0, 0,
        hWnd,
        (HMENU)IDC_BUTTON_CLEAR,
        hInstance,
        NULL);
    if(!hBtn_clr) {
        MessageBox(hWnd, tcMes[44], tcMes[19], MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(hWnd, nShowCmd);
    UpdateWindow(hWnd);

    MSG msg;
    SetTimer(hWnd, 1, 16, NULL);

    while(GetMessage(&msg, NULL, 0, 0)) { // Window Message が WM_QUIT(=0) でない限りループ
        if(!TranslateAccelerator(hWnd, hAccel, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int)msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    DWORD dwTemp;

    switch(uMsg) {
        HANDLE_MSG(hwnd, WM_CREATE, Main_OnCreate); // ウィンドウ生成時
        HANDLE_MSG(hwnd, WM_CLOSE, Main_OnClose); // 終了時
        HANDLE_MSG(hwnd, WM_TIMER, Main_OnTimer);
        HANDLE_MSG(hwnd, WM_SIZE, Main_OnSize); // ウィンドウサイズ変更時
        HANDLE_MSG(hwnd, WM_PAINT, Main_OnPaint);
        HANDLE_MSG(hwnd, WM_COMMAND, Main_OnCommand);

        case WM_ACTIVATE:
            if(LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
                SetFocus(hWnd_focused); // 以前フォーカスが当たっていたエディットにフォーカスを戻す     
            break;

        case 0x02e0: // WM_DPICHANGED (The SDK for WinXP doesn't have this macro)
            MoveWindow(hwnd, ((PRECT)lParam)->left, ((PRECT)lParam)->top, ((PRECT)lParam)->right - ((PRECT)lParam)->left,
                ((PRECT)lParam)->bottom - ((PRECT)lParam)->top, FALSE);
            break;

        case APP_THREADEND:
            WaitForSingleObject(hThread, INFINITE);
            GetExitCodeThread(hThread, &dwTemp);
            CloseHandle(hThread);
            switch(dwTemp) {
                case IDE_INVALID:
                    if(!status.mode) {
                        MessageBox(hwnd, tcMes[31], tcMes[19], MB_OK | MB_ICONWARNING);
                        OutputToEditbox(hEdi_out, tcMes[32]);
                    }else{
                        MessageBox(hwnd, tcMes[35], tcMes[19], MB_OK | MB_ICONWARNING);
                        OutputToEditbox(hEdi_out, tcMes[36]);
                    }
                    break;

                case IDE_ABORT:
                    isAborted = false;
                    break;

                case IDE_CANCEL:
                    break;

                case IDE_CANNOTOPENFILE:
                    MessageBox(hwnd, tcMes[18], tcMes[19], MB_OK | MB_ICONWARNING);
                    OutputToEditbox(hEdi_out, tcMes[38]);
                    break;

                case IDE_CANNOTWRITEFILE:
                    MessageBox(hwnd, tcMes[22], tcMes[19], MB_OK | MB_ICONWARNING);
                    OutputToEditbox(hEdi_out, tcMes[37]);
            }
            if(dwTemp) {
                    wsprintf(tcTemp, TEXT("%s - %s"), tcMes[0], TARGET_CPU);
                    SetWindowText(hwnd, tcTemp);
            }
            if(status.mode) {
                EnableWindow(hEdi1, TRUE);
                if(!status.onlycnt) EnableWindow(hEdi2, TRUE);
            }
            EnableWindow(hBtn_ok, TRUE);
            EnableWindow(hEdi0, TRUE);
            EnableWindow(hBtn_abort, FALSE);
            EnableMenuItem(hMenu, 2, MF_BYPOSITION | MF_ENABLED); // 「オプション」メニューを再度有効化
            DrawMenuBar(hwnd); // メニュー再描画
            Paint(hwnd);
            SetFocus(hWnd_focused);
            status.isWorking = 0;
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return myDefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// エディットコントロールのサブクラス化用独自 Window Procedure
LRESULT CALLBACK Edit0WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    WNDPROC defProc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch(uMsg) {
        case WM_DESTROY:
            SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)defProc); // 元の Window Procedure に戻す
            break;

        case WM_CHAR:
            // Enterで実行or次のエディットに移動, 実行対象でなければdefaultに流す(間に別のメッセージ入れちゃだめ!!)
            switch((CHAR)wParam) {
                case VK_RETURN:
                    if(!status.isWorking && !status.mode) StartCalc(hwnd);
                    else if(status.mode) SetFocus(hEdi1);
                    return 0;
            }
        default:
            return CallWindowProc(defProc, hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK Edit1WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    WNDPROC defProc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch(uMsg) {
        case WM_DESTROY:
            SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)defProc);
            break;

        case WM_CHAR:
            // Enterで次のエディットに移動, 実行対象でなければdefaultに流す(後に別のメッセージ入れちゃだめ!!)
            switch((CHAR)wParam) {
                case VK_RETURN:
                    SetFocus(hEdi2);
                    return 0;
            }
        default:
            return CallWindowProc(defProc, hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK Edit2WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    WNDPROC defProc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch(uMsg) {
        case WM_DESTROY:
            SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)defProc);
            break;

        case WM_CHAR:
            // Enterで計算開始, 実行対象でなければdefaultに流す(間に別のメッセージ入れちゃだめ!!)
            switch((CHAR)wParam) {
                case VK_RETURN:
                    if(!status.isWorking) StartCalc(hwnd);
                    return 0;
            }
        default:
            return CallWindowProc(defProc, hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

BOOL Main_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
    INT scrx, scry;

    // メニューの初期化
    SetMenu(hwnd, hMenu);
    if(GetUserDefaultUILanguage() == 0x0411)
        CheckMenuRadioItem(hMenu, IDM_OPT_LANG_JA, IDM_OPT_LANG_EN, IDM_OPT_LANG_JA, MF_BYCOMMAND);
    else CheckMenuRadioItem(hMenu, IDM_OPT_LANG_JA, IDM_OPT_LANG_EN, IDM_OPT_LANG_EN, MF_BYCOMMAND);
    CheckMenuRadioItem(hMenu, IDM_OPT_PF, IDM_OPT_LCPN, IDM_OPT_PF, MF_BYCOMMAND);
    CheckMenuRadioItem(hMenu, IDM_OPT_CHARSET_UTF8, IDM_OPT_CHARSET_SJIS, IDM_OPT_CHARSET_UTF8, MF_BYCOMMAND);
    EnableMenuItem(hMenu, IDM_OPT_ONLYNUM, MF_BYCOMMAND | MF_GRAYED);
    EnableMenuItem(hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_GRAYED);

    hMemDC = CreateCompatibleDC(NULL);
    hBshSys = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    hPenSys = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));

    hEdi0 = CreateWindowEx( // 入力ボックス
        0,
        TEXT("EDIT"),
        TEXT(""),
        WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
        0, 0, 0, 0,
        hwnd,
        (HMENU)IDC_EDIT_IN1,
        lpCreateStruct->hInstance,
        NULL);
    if(!hEdi0) {
        MessageBox(hwnd, tcMes[44], tcMes[19], MB_OK | MB_ICONERROR);
        return 1;
    }
    SendMessage(hEdi0, EM_SETLIMITTEXT, (WPARAM)(MAX_INPUT_LENGTH-1), 0); // 符号付き64ビットの限界桁数に設定
    WNDPROC wpedi0_old = (WNDPROC)SetWindowLongPtr(hEdi0, GWLP_WNDPROC, (LONG_PTR)Edit0WindowProc); // サブクラス化
    SetWindowLongPtr(hEdi0, GWLP_USERDATA, (LONG_PTR)wpedi0_old);
    SetFocus(hWnd_focused = hEdi0);

    hEdi_out = CreateWindowEx( // 結果出力ボックス
        0,
        TEXT("EDIT"),
        TEXT(""),
        WS_CHILD | WS_VISIBLE | ES_READONLY | ES_LEFT | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
        0, 0, 0, 0,
        hwnd,
        (HMENU)IDC_EDIT_OUT,
        lpCreateStruct->hInstance,
        NULL);
    if(!hEdi_out) {
        MessageBox(hwnd, tcMes[44], tcMes[19], MB_OK | MB_ICONERROR);
        return 1;
    }
    SendMessage(hEdi_out, EM_SETLIMITTEXT, (WPARAM)(MAX_OUTPUT_BUFFER-1), 0); // 文字数制限を最大値に変更

    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFOEX mInfo; mInfo.cbSize = sizeof(MONITORINFOEX);
    GetMonitorInfo(hMonitor, &mInfo);
    if(mInfo.rcWork.bottom*5/3 > mInfo.rcWork.right) {
        scrx = mInfo.rcWork.right*3/4;
        scry = mInfo.rcWork.right*9/20;
    } else {
        scrx = mInfo.rcWork.bottom*10/9;
        scry = mInfo.rcWork.bottom*2/3;
    }
    if(scrx<800 || scry<480) { scrx=800; scry=480; }
    SetWindowPos(hwnd, NULL, 0, 0, scrx, scry, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
    return TRUE;
}

void Main_OnClose(HWND hwnd) {
    if(IDYES == MessageBox(hwnd, tcMes[12], tcMes[13], MB_YESNO | MB_ICONINFORMATION)) {
        KillTimer(hwnd, 1);
        DestroyWindow(hwnd);
    }
    else SetFocus(hWnd_focused); // 直近までフォーカスが当たっていたエディットボックスにフォーカスを戻す
}

void Main_OnTimer(HWND hwnd, UINT id) {
    static BYTE r=0, g=255, b=255;
    HWND hwnd_temp;

    if((hwnd_temp = GetFocus()) && (hwnd_temp == hEdi0 || hwnd_temp == hEdi1 || hwnd_temp == hEdi2 || hwnd_temp == hEdi_out))
        hWnd_focused = hwnd_temp;

    if(b <= 0 && g < 255) g++;
    if(g >= 255 && r > 0) r--;
    if(r <= 0 && b < 255) b++;
    if(b >= 255 && g > 0) g--;
    if(g <= 0 && r < 255) r++;
    if(r >= 255 && b > 0) b--;

    if(r > 255) r = 255;
    if(r < 0) r = 0;
    if(g > 255) g = 255;
    if(g < 0) g = 0;
    if(b > 255) b = 255;
    if(b < 0) b = 0;

    if(hBrush) DeleteObject(hBrush);
    if(hPen) DeleteObject(hPen);
    hBrush = CreateSolidBrush(RGB(r, g, b)); // ブラシを作成(塗りつぶし用)
    hPen = CreatePen(PS_SOLID, 1, RGB(r, g, b)); // ペンを作成(輪郭用)
    Paint(hwnd);
}

void Main_OnSize(HWND hwnd, UINT state, int cx, int cy) {
    INT scrx, scry;
    RECT rect;
    LOGFONT rLogfont; // 作成するフォントの構造体
    static HBITMAP hBitmap = NULL;

    GetClientRect(hwnd, &rect);
    scrx = rect.right; scry = rect.bottom;

    // メインウィンドウ用のフォントを作成
    if(scrx/24 < scry/12) rLogfont.lfHeight = scrx/24;
    else rLogfont.lfHeight = scry/12;
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
    lstrcpy(rLogfont.lfFaceName, TEXT("MS Shell Dlg"));
    if(hFmes) DeleteObject(hFmes);
    hFmes = CreateFontIndirect(&rLogfont); // フォントを作成

    // ボタン用のフォントを作成
    if(24*scrx/700 < 24*scry/400) rLogfont.lfHeight = 24*scrx/700;
    else rLogfont.lfHeight = 24*scry/400;
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
    lstrcpy(rLogfont.lfFaceName, TEXT("MS Shell Dlg"));
    if(hFbtn) DeleteObject(hFbtn);
    hFbtn = CreateFontIndirect(&rLogfont);

    // 注釈用のフォントを作成
    if(15*scrx/700 < 15*scry/400) rLogfont.lfHeight = 15*scrx/700;
    else rLogfont.lfHeight = 15*scry/400;
    if(hFnote) DeleteObject(hFnote);
    hFnote = CreateFontIndirect(&rLogfont);

    // エディットボックス用のフォントを作成
    if(16*scrx/700 < 16*scry/400) rLogfont.lfHeight = 16*scrx/700;
    else rLogfont.lfHeight = 16*scry/400;
    if(rLogfont.lfHeight<12) rLogfont.lfHeight = 12;
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
    lstrcpy(rLogfont.lfFaceName, TEXT("MS Shell Dlg"));
    if(hFedi) DeleteObject(hFedi);
    hFedi = CreateFontIndirect(&rLogfont);

    // 作成したフォントを適用
    SendMessage(hBtn_ok, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
    SendMessage(hBtn_abort, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
    SendMessage(hBtn_clr, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
    SendMessage(hEdi0, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
    SendMessage(hEdi_out, WM_SETFONT, (WPARAM)hFedi, MAKELPARAM(FALSE, 0));
    if(status.mode) {
        SendMessage(hEdi1, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
        SendMessage(hEdi2, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
    }

    // コントロールの移動とサイズ変更
    btnsize[0] = 64*scrx/700; btnsize[1] = 32*scry/400;
    if(!status.mode) {
        MoveWindow(hEdi0, btnsize[0], 0, btnsize[0]*4, btnsize[1], TRUE);
        MoveWindow(hBtn_ok, btnsize[0]*5, 0, btnsize[0], btnsize[1], TRUE);
        MoveWindow(hBtn_abort, btnsize[0]*6, 0, btnsize[0], btnsize[1], TRUE);
        MoveWindow(hBtn_clr, btnsize[0]*7, 0, btnsize[0]*5/2, btnsize[1], TRUE);
        MoveWindow(hEdi_out, scrx/20, scry*9/40, scrx*9/10, scry*29/40, TRUE);
    } else {
        MoveWindow(hEdi0, btnsize[0], 0, btnsize[0]*4, btnsize[1], TRUE);
        MoveWindow(hEdi1, btnsize[0]*6, 0, btnsize[0]*4, btnsize[1], TRUE);
        MoveWindow(hEdi2, btnsize[0], btnsize[1], btnsize[0]*2, btnsize[1], TRUE);
        MoveWindow(hBtn_ok, btnsize[0]*3, btnsize[1], btnsize[0], btnsize[1], TRUE);
        MoveWindow(hBtn_abort, btnsize[0]*4, btnsize[1], btnsize[0], btnsize[1], TRUE);
        MoveWindow(hBtn_clr, btnsize[0]*5, btnsize[1], btnsize[0]*5/2, btnsize[1], TRUE);
        MoveWindow(hEdi_out, scrx/20, scry*3/10, scrx*9/10, scry*13/20, TRUE);
    }

    if(hBitmap) DeleteObject(hBitmap);
    HDC hdc = GetDC(hwnd);
    if(!(hBitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom))) {
        MessageBox(hwnd, tcMes[46], tcMes[19], MB_OK | MB_ICONERROR);
        PostQuitMessage(1);
    }
    ReleaseDC(hwnd, hdc);
    SelectObject(hMemDC, hBitmap); // 新サイズの Bitmap を Memory Device Context に設定
    Paint(hwnd);
}

void Main_OnPaint(HWND hwnd) {
    RECT rect;
    PAINTSTRUCT ps;

    GetClientRect(hwnd, &rect);

    HDC hdc = BeginPaint(hwnd, &ps);
    BitBlt(hdc, 0, 0, rect.right, rect.bottom, hMemDC, 0, 0, SRCCOPY);
    EndPaint(hwnd, &ps);
}

void Main_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
    INT editlen, mblen;
    UINT menu[5];
    PTSTR tcEdit;
    PSTR mbEdit;
    DWORD dwTemp;
    HANDLE hFile, hHeap;
    OPENFILENAME *ofn;
    WNDPROC defProc;

    switch(id) {
        case IDC_BUTTON_OK: // OKボタン
            if(status.isWorking) break;
            StartCalc(hwnd);
            break;

        case IDC_BUTTON_ABORT: // 中断ボタン
            isAborted = true;
            break;

        case IDC_BUTTON_CLEAR: // 履歴消去
            SetWindowText(hEdi_out, TEXT(""));
            break;

        case IDM_FILE_SAVE_AS: // テキストファイルに保存
#ifndef UNICODE
            MessageBox(hwnd, tcMes[23], tcMes[21], MB_OK | MB_ICONINFORMATION);
#endif
            hHeap = GetProcessHeap();
            ofn = (OPENFILENAME*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(OPENFILENAME));
            if(!ofn) {
                MessageBox(hwnd, tcMes[45], tcMes[19], MB_OK | MB_ICONWARNING);
                break;
            }
            ofn->lStructSize = sizeof(OPENFILENAME);
            ofn->hwndOwner = hwnd;
            ofn->lpstrFilter = TEXT("Text File (*.txt)\0*.txt\0")
                               TEXT("All files (*.*)\0*.*\0");
            ofn->lpstrFile = (PTSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, MAX_PATH*sizeof(TCHAR));
            if(!ofn->lpstrFile) {
                MessageBox(hwnd, tcMes[45], tcMes[19], MB_OK | MB_ICONWARNING);
                HeapFree(hHeap, 0, ofn);
                break;
            }
            ofn->nMaxFile = MAX_PATH;
            ofn->lpstrDefExt = TEXT(".txt");
            ofn->lpstrTitle = tcMes[17];
            ofn->Flags = OFN_OVERWRITEPROMPT;
            if(!GetSaveFileName(ofn)) break;

            hFile = CreateFile(ofn->lpstrFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if(hFile == INVALID_HANDLE_VALUE) {
                MessageBox(hwnd, tcMes[18], tcMes[19], MB_OK | MB_ICONWARNING);
                HeapFree(hHeap, 0, ofn->lpstrFile);
                HeapFree(hHeap, 0, ofn);
                break;
            }
            editlen = GetWindowTextLength(hEdi_out) + 1;
            tcEdit = (PTSTR)HeapAlloc(hHeap, 0, editlen*sizeof(TCHAR));
            if(!tcEdit) {
                MessageBox(hwnd, tcMes[45], tcMes[19], MB_OK | MB_ICONWARNING);
                HeapFree(hHeap, 0, ofn->lpstrFile);
                HeapFree(hHeap, 0, ofn);
                break;
            }
            GetWindowText(hEdi_out, tcEdit, editlen);
#ifdef UNICODE
            mblen = WideCharToMultiByte(status.charset?932:65001, NULL, tcEdit, editlen, NULL, 0, NULL, NULL); // 変換後サイズ取得
            mbEdit = (PSTR)HeapAlloc(hHeap, 0, mblen*sizeof(CHAR));
            if(!mbEdit) {
                MessageBox(hwnd, tcMes[45], tcMes[19], MB_OK | MB_ICONWARNING);
                HeapFree(hHeap, 0, ofn->lpstrFile);
                HeapFree(hHeap, 0, ofn);
                HeapFree(hHeap, 0, tcEdit);
                break;
            }
            WideCharToMultiByte(status.charset?932:65001, NULL, tcEdit, editlen, mbEdit, mblen, NULL, NULL);
            if(WriteFile(hFile, mbEdit, (mblen-1)*sizeof(CHAR), &dwTemp, NULL))
#else
            if(WriteFile(hFile, tcEdit, (editlen-1)*sizeof(CHAR), &dwTemp, NULL))
#endif
                MessageBox(hwnd, tcMes[20], tcMes[21], MB_OK | MB_ICONINFORMATION);
            else
                MessageBox(hwnd, tcMes[22], tcMes[19], MB_OK | MB_ICONWARNING);
            CloseHandle(hFile);
            HeapFree(hHeap, 0, ofn->lpstrFile);
            HeapFree(hHeap, 0, ofn);
            HeapFree(hHeap, 0, tcEdit);
            HeapFree(hHeap, 0, mbEdit);
            break;

        case IDM_FILE_EXIT: // 終了
            SendMessage(hwnd, WM_CLOSE, 0, 0);
            break;

        case IDM_EDIT_CUT: // 切り取り
            SendMessage(hWnd_focused, WM_CUT, 0, 0);
            break;

        case IDM_EDIT_COPY: // コピー
            SendMessage(hWnd_focused, WM_COPY, 0, 0);
            break;

        case IDM_EDIT_PASTE: // 貼り付け
            SendMessage(hWnd_focused, WM_PASTE, 0, 0);
            break;

        case IDM_EDIT_SELECT_ALL: // 全選択
            editlen = (INT)SendMessage(hWnd_focused, WM_GETTEXTLENGTH, 0, 0);
            SendMessage(hWnd_focused, EM_SETSEL, 0, editlen);
            break;

        case IDM_OPT_PF: // 素因数分解に変更
            if(!status.mode) break;
            CheckMenuRadioItem(hMenu, IDM_OPT_PF, IDM_OPT_LCPN, IDM_OPT_PF, MF_BYCOMMAND);
            EnableMenuItem(hMenu, IDM_OPT_ONLYNUM, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_GRAYED);
            SetFocus(hEdi0);
            DestroyWindow(hEdi1); // mode1で追加されたエディットボックスを削除
            DestroyWindow(hEdi2); // mode1で追加されたエディットボックスを削除
            status.mode = 0;
            SendMessage(hwnd, WM_SIZE, 0, 0);
            break;

        case IDM_OPT_LCPN: // 素数列挙・数えに変更
            if(status.mode) break;
            CheckMenuRadioItem(hMenu, IDM_OPT_PF, IDM_OPT_LCPN, IDM_OPT_LCPN, MF_BYCOMMAND);
            EnableMenuItem(hMenu, IDM_OPT_ONLYNUM, MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_ENABLED);

            hEdi1 = CreateWindowEx( // 入力ボックス
                0,
                TEXT("EDIT"),
                TEXT(""),
                WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
                0, 0, 0, 0,
                hwnd,
                (HMENU)IDC_EDIT_IN2,
                hInst,
                NULL);
            if(!hEdi1){
                MessageBox(hwnd, tcMes[44], tcMes[19], MB_OK | MB_ICONERROR);
                PostQuitMessage(1);
            }
            SendMessage(hEdi1, EM_SETLIMITTEXT, (WPARAM)(MAX_INPUT_LENGTH-1), 0);
            defProc = (WNDPROC)SetWindowLongPtr(hEdi1, GWLP_WNDPROC, (LONG_PTR)Edit1WindowProc); // サブクラス化
            SetWindowLongPtr(hEdi1, GWLP_USERDATA, (LONG_PTR)defProc);

            hEdi2 = CreateWindowEx( // 入力ボックス
                0,
                TEXT("EDIT"),
                TEXT(""),
                WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
                0, 0, 0, 0,
                hwnd,
                (HMENU)IDC_EDIT_IN3,
                hInst,
                NULL);
            if(!hEdi2) {
                MessageBox(hwnd, tcMes[44], tcMes[19], MB_OK | MB_ICONERROR);
                PostQuitMessage(1);
            }
            SendMessage(hEdi2, EM_SETLIMITTEXT, (WPARAM)(MAX_INPUT_LENGTH-1), 0);
            defProc = (WNDPROC)SetWindowLongPtr(hEdi2, GWLP_WNDPROC, (LONG_PTR)Edit2WindowProc); // サブクラス化
            SetWindowLongPtr(hEdi2, GWLP_USERDATA, (LONG_PTR)defProc);

            status.mode = 1;
            SendMessage(hwnd, WM_SIZE, 0, 0);
            break;

        case IDM_OPT_ONLYNUM: // 「個数のみ表示」
            if(GetMenuState(hMenu, IDM_OPT_ONLYNUM, MF_BYCOMMAND) & MF_CHECKED) { // 既にチェックがついていた場合
                CheckMenuItem(hMenu, IDM_OPT_ONLYNUM, MF_BYCOMMAND | MF_UNCHECKED);
                EnableMenuItem(hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_ENABLED);
                EnableWindow(hEdi2, TRUE);
                status.onlycnt = 0;
            } else {
                CheckMenuItem(hMenu, IDM_OPT_ONLYNUM, MF_BYCOMMAND | MF_CHECKED);
                EnableMenuItem(hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(hEdi2, FALSE);
                status.onlycnt = 1;
            }
            break;

        case IDM_OPT_OUTFILE: // 「テキストファイルに出力」
            if(GetMenuState(hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND) & MF_CHECKED) {
                CheckMenuItem(hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_UNCHECKED);
                status.usefile = 0;
            } else {
                CheckMenuItem(hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_CHECKED);
                status.usefile = 1;
            }
            break;

        case IDM_OPT_LANG_JA: // 日本語に変更
            // 状態のバックアップ
            menu[0] = GetMenuState(hMenu, IDM_OPT_PF, MF_BYCOMMAND);
            menu[1] = GetMenuState(hMenu, IDM_OPT_ONLYNUM, MF_BYCOMMAND);
            menu[2] = GetMenuState(hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND);
            menu[3] = GetMenuState(hMenu, IDM_OPT_CHARSET_UTF8, MF_BYCOMMAND);
            DestroyMenu(hMenu);
            hMenu = LoadMenu(hInst, TEXT("ResMenu_JA"));
            SetMenu(hwnd, hMenu);
            CheckMenuRadioItem(hMenu, IDM_OPT_LANG_JA, IDM_OPT_LANG_EN, IDM_OPT_LANG_JA, MF_BYCOMMAND);

            // チェック・ラジオボタンの復元
            if(menu[0] & MF_CHECKED) CheckMenuRadioItem(hMenu, IDM_OPT_PF, IDM_OPT_LCPN, IDM_OPT_PF, MF_BYCOMMAND);
            else CheckMenuRadioItem(hMenu, IDM_OPT_PF, IDM_OPT_LCPN, IDM_OPT_LCPN, MF_BYCOMMAND);
            if(menu[1] & MF_CHECKED) CheckMenuItem(hMenu, IDM_OPT_ONLYNUM, MF_BYCOMMAND | MF_CHECKED);
            if(menu[2] & MF_CHECKED) CheckMenuItem(hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_CHECKED);
            if(menu[3] & MF_CHECKED) CheckMenuRadioItem(hMenu, IDM_OPT_CHARSET_UTF8, IDM_OPT_CHARSET_SJIS, IDM_OPT_CHARSET_UTF8, MF_BYCOMMAND);
            else CheckMenuRadioItem(hMenu, IDM_OPT_CHARSET_UTF8, IDM_OPT_CHARSET_SJIS, IDM_OPT_CHARSET_SJIS, MF_BYCOMMAND);

            // 有効無効の復元
            if(menu[1] & MF_GRAYED) EnableMenuItem(hMenu, IDM_OPT_ONLYNUM, MF_BYCOMMAND | MF_GRAYED);
            if(menu[2] & MF_GRAYED) EnableMenuItem(hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_GRAYED);

            for(int i = 0; i < SIZE_OF_STRING_TABLE; i++)
                LoadString(hInst, IDS_JA+i, tcMes[i], sizeof(tcMes[0])/sizeof(tcMes[0][0]));
            SetWindowText(hBtn_ok, tcMes[9]);
            SetWindowText(hBtn_abort, tcMes[10]);
            SetWindowText(hBtn_clr, tcMes[11]);
            wsprintf(tcTemp, TEXT("%s - %s"), tcMes[0], TARGET_CPU);
            SetWindowText(hwnd, tcTemp);
            break;

        case IDM_OPT_LANG_EN: // 英語に変更
            // 状態のバックアップ
            menu[0] = GetMenuState(hMenu, IDM_OPT_PF, MF_BYCOMMAND);
            menu[1] = GetMenuState(hMenu, IDM_OPT_ONLYNUM, MF_BYCOMMAND);
            menu[2] = GetMenuState(hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND);
            menu[3] = GetMenuState(hMenu, IDM_OPT_CHARSET_UTF8, MF_BYCOMMAND);
            DestroyMenu(hMenu);
            hMenu = LoadMenu(hInst, TEXT("ResMenu_EN"));
            SetMenu(hwnd, hMenu);
            CheckMenuRadioItem(hMenu, IDM_OPT_LANG_JA, IDM_OPT_LANG_EN, IDM_OPT_LANG_EN, MF_BYCOMMAND);

            // チェック・ラジオボタンの復元
            if(menu[0] & MF_CHECKED) CheckMenuRadioItem(hMenu, IDM_OPT_PF, IDM_OPT_LCPN, IDM_OPT_PF, MF_BYCOMMAND);
            else CheckMenuRadioItem(hMenu, IDM_OPT_PF, IDM_OPT_LCPN, IDM_OPT_LCPN, MF_BYCOMMAND);
            if(menu[1] & MF_CHECKED) CheckMenuItem(hMenu, IDM_OPT_ONLYNUM, MF_BYCOMMAND | MF_CHECKED);
            if(menu[2] & MF_CHECKED) CheckMenuItem(hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_CHECKED);
            if(menu[3] & MF_CHECKED) CheckMenuRadioItem(hMenu, IDM_OPT_CHARSET_UTF8, IDM_OPT_CHARSET_SJIS, IDM_OPT_CHARSET_UTF8, MF_BYCOMMAND);
            else CheckMenuRadioItem(hMenu, IDM_OPT_CHARSET_UTF8, IDM_OPT_CHARSET_SJIS, IDM_OPT_CHARSET_SJIS, MF_BYCOMMAND);

            // 有効無効の復元
            if(menu[1] & MF_GRAYED) EnableMenuItem(hMenu, IDM_OPT_ONLYNUM, MF_BYCOMMAND | MF_GRAYED);
            if(menu[2] & MF_GRAYED) EnableMenuItem(hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_GRAYED);

            for(int i = 0; i < SIZE_OF_STRING_TABLE; i++)
                LoadString(hInst, IDS_EN+i, tcMes[i], sizeof(tcMes[0])/sizeof(tcMes[0][0]));
            SetWindowText(hBtn_ok, tcMes[9]);
            SetWindowText(hBtn_abort, tcMes[10]);
            SetWindowText(hBtn_clr, tcMes[11]);
            wsprintf(tcTemp, TEXT("%s - %s"), tcMes[0], TARGET_CPU);
            SetWindowText(hwnd, tcTemp);
            break;

        case IDM_OPT_CHARSET_UTF8:
            if(!status.charset) break;
            CheckMenuRadioItem(hMenu, IDM_OPT_CHARSET_UTF8, IDM_OPT_CHARSET_SJIS, IDM_OPT_CHARSET_UTF8, MF_BYCOMMAND);
            status.charset = 0;
            break;

        case IDM_OPT_CHARSET_SJIS:
            if(status.charset) break;
            CheckMenuRadioItem(hMenu, IDM_OPT_CHARSET_UTF8, IDM_OPT_CHARSET_SJIS, IDM_OPT_CHARSET_SJIS, MF_BYCOMMAND);
            status.charset = 1;
            break;

        case IDM_HELP_HOWTOUSE: // 使い方
            MessageBox(hwnd, tcMes[status.mode?8:7], tcMes[14], MB_OK | MB_ICONINFORMATION);
            break;

        case IDM_HELP_ABOUT: // このプログラムについて
            wsprintf(tcTemp, TEXT("%s\n\n%s%s\n%s%s Application\n%s%s\n%s") __DATE__ TEXT(" ") __TIME__ TEXT("\n\n%s"),
                tcMes[1], tcMes[2], COMPILER_NAME, tcMes[3], TARGET_PLATFORM, tcMes[4], TARGET_CPU, tcMes[5], tcMes[6]);
            MessageBox(hwnd, tcTemp, tcMes[15], MB_OK | MB_ICONINFORMATION);
            break;
    }
    if(id < IDC_EDIT_IN1) SetFocus(hWnd_focused); // エディット以外のコントロールが押されたとき、エディットにフォーカスを戻す
}

void Paint(HWND hwnd) {
    INT scrx, scry;
    RECT rect;

    GetClientRect(hwnd, &rect);
    scrx = rect.right; scry = rect.bottom;

    SelectObject(hMemDC, hPen); // デバイスコンテキストにペンを設定
    SelectObject(hMemDC, hBrush); // デバイスコンテキストにブラシを設定
    Rectangle(hMemDC, rect.left, rect.top, rect.right, rect.bottom); // 領域いっぱいに四角形を描く

    if(!status.mode) {
        SelectObject(hMemDC, hPenSys);
        SelectObject(hMemDC, hBshSys);
        Rectangle(hMemDC, 0, 0, btnsize[0], btnsize[1]);

        SetBkMode(hMemDC, TRANSPARENT);
        SetTextColor(hMemDC, RGB(0, 0, 0));
        SelectObject(hMemDC, hFnote); // デバイスコンテキストにフォントを設定
        rect.right = btnsize[0]; rect.bottom = btnsize[1];
        DrawText(hMemDC, tcMes[24], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    } else {
        SelectObject(hMemDC, hPenSys);
        SelectObject(hMemDC, hBshSys);
        Rectangle(hMemDC, 0, 0, btnsize[0], btnsize[1]);
        Rectangle(hMemDC, btnsize[0]*5, 0, btnsize[0]*6, btnsize[1]);
        Rectangle(hMemDC, 0, btnsize[1], btnsize[0], btnsize[1]*2);

        SetBkMode(hMemDC, TRANSPARENT);
        SetTextColor(hMemDC, RGB(0, 0, 0));
        SelectObject(hMemDC, hFnote);
        rect.right = btnsize[0]; rect.bottom = btnsize[1];
        DrawText(hMemDC, tcMes[25], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        rect.left = btnsize[0]*5; rect.right = btnsize[0]*6;
        DrawText(hMemDC, tcMes[26], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        rect.left = 0; rect.top = btnsize[1]; rect.bottom = btnsize[1]*2; rect.right = btnsize[0];
        DrawText(hMemDC, tcMes[27], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    SetBkMode(hMemDC, OPAQUE);
    SetBkColor(hMemDC, RGB(255, 255, 0));
    SetTextColor(hMemDC, RGB(0, 0, 255));
    SelectObject(hMemDC, hFmes);
    rect.left = 0; rect.right = scrx;
    if(!status.mode) {
        rect.top = btnsize[1];
        rect.bottom = scry*9/40;
    } else {
        rect.top = btnsize[1]*2;
        rect.bottom = scry*3/10;
    }

    if(status.isWorking) DrawText(hMemDC, tcMes[28], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    else if(!status.mode) DrawText(hMemDC, tcMes[29], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    else if(status.mode) DrawText(hMemDC, tcMes[30], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    InvalidateRect(hwnd, NULL, FALSE);
}

void StartCalc(HWND hwnd) {
    DWORD dwTemp;
    bool isErange = false;

    status.isWorking = 1;
    EnableWindow(hBtn_ok, FALSE);
    EnableWindow(hEdi0, FALSE);
    EnableWindow(hEdi1, FALSE);
    EnableWindow(hEdi2, FALSE);
    EnableWindow(hBtn_abort, TRUE);
    EnableMenuItem(hMenu, 2, MF_BYPOSITION | MF_GRAYED); // 「オプション」メニューをグレーアウト
    DrawMenuBar(hwnd); // メニュー再描画

    SendMessage(hEdi0, WM_GETTEXT, MAX_INPUT_LENGTH, (LPARAM)tcTemp);
    errno = 0;
    num[0] = _tcstoull(tcTemp, NULL, 10);
    isErange = (errno == ERANGE);
    if(status.mode) {
        SendMessage(hEdi1, WM_GETTEXT, MAX_INPUT_LENGTH, (LPARAM)tcTemp);
        errno = 0;
        num[1] = _tcstoull(tcTemp, NULL, 10);
        isErange = isErange || (errno == ERANGE);
        SendMessage(hEdi2, WM_GETTEXT, MAX_INPUT_LENGTH, (LPARAM)tcTemp);
        errno = 0;
        num[2] = _tcstoull(tcTemp, NULL, 10);
        isErange = isErange || (errno == ERANGE);
    }
    if(isErange) MessageBox(hwnd, tcMes[47], tcMes[21], MB_OK | MB_ICONINFORMATION);
    SetWindowText(hwnd, tcMes[16]);

    if(!status.mode) hThread = CreateThread(NULL, 0, PrimeFactorization, (LPVOID)hwnd, 0, &dwTemp);
    else hThread = CreateThread(NULL, 0, ListPrimeNumbers, (LPVOID)hwnd, 0, &dwTemp);
    SetThreadPriority(hThread, THREAD_PRIORITY_BELOW_NORMAL);
}

DWORD WINAPI PrimeFactorization(LPVOID lpParameter) {
    ULONGLONG N = num[0], cnt=0, i=2;
    bool chk = false;
    TCHAR tcStr1[MAX_BUFFER] = TEXT(""), tcStr2[MAX_BUFFER] = TEXT("");
    HWND hwnd = (HWND)lpParameter;

    if(N<=0) {
        PostMessage(hwnd, APP_THREADEND, 0, 0);
        return IDE_INVALID;
    }

    // 計算
    while(1) {
        while(i<=N && !isAborted) {
            if(N%i==0 && N!=i) {
                chk = true;
                break;
            }
            if(N/i<i || N==i) {
                chk = false;
                break;
            }
            if(i==2) i++;
            else i+=2;
        }
        if(isAborted) break;
        if(chk) {
            wsprintf(tcStr2, TEXT("%I64ux"), i); // 見つけた素因数を文字列に変換・「x」を足す
            lstrcat(tcStr1, tcStr2); // 結果文字列に追加
        } else {
            wsprintf(tcStr2, TEXT("%I64u"), N); // その数自身を文字列に変換(素数だった場合)
            lstrcat(tcStr1, tcStr2); // 結果文字列に追加
            break;
        }
        N/=i;
        cnt++;
    }
    if(cnt==0 && N>1) lstrcat(tcStr1, tcMes[33]); // 素数だった場合、その旨を追加

    if(isAborted) {
        PostMessage(hwnd, APP_THREADEND, 0, 0);
        return IDE_ABORT;
    }

    // 結果文字列の生成
    wsprintf(tcStr2, TEXT("%s%I64u = %s"), tcMes[34], num[0], tcStr1);

    // Update the window's title and the result box
    OutputToEditbox(hEdi_out, tcStr2);
    SendMessage(hEdi_out, EM_REPLACESEL, 0, (WPARAM)TEXT("\r\n")); // 続けて書き込むならこの1行だけで良い
    wsprintf(tcStr1, TEXT(" - %s"), tcMes[0]);
    lstrcat(tcStr2, tcStr1);
    SetWindowText(hwnd, tcStr2);

    PostMessage(hwnd, APP_THREADEND, 0, 0);
    return IDE_SUCCESS;
}

DWORD WINAPI ListPrimeNumbers(LPVOID lpParameter) {
    ULONGLONG cnt = 0;
    DWORD dwTemp;
    int mblen;
    TCHAR tcStr1[MAX_BUFFER], tcStr2[MAX_BUFFER], tcFile[MAX_PATH];
    CHAR mbStr[MAX_BUFFER];
    HANDLE hFile = NULL;
    OPENFILENAME ofn = {0};
    HWND hwnd = (HWND)lpParameter;

    // 入力値を調整(主に空or"0"の時の対応)
    if(num[0]<2) num[0] = 2;
    if(num[0]!=2 && !(num[0]%2)) num[0]++;
    if(num[1]<1) num[1] = 0xffffffffffffffff;
    if(num[2]<1) num[2] = 0xffffffffffffffff;
    if(num[0]>num[1]) {
        PostMessage(hwnd, APP_THREADEND, 0, 0);
        return IDE_INVALID;
    }

    // ファイル出力のときの下準備(「個数のみ」のときはテキスト出力しないので外す。以下同様。)
    if(!status.onlycnt && status.usefile) {
#ifndef UNICODE
        MessageBox(hwnd, tcMes[23], tcMes[21], MB_OK | MB_ICONINFORMATION);
#endif
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFilter = TEXT("Text File (*.txt)\0*.txt\0")
                          TEXT("All files (*.*)\0*.*\0");
        ofn.lpstrFile = tcFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrDefExt = TEXT(".txt");
        ofn.lpstrTitle = tcMes[37];
        ofn.Flags = OFN_OVERWRITEPROMPT;
        if(!GetSaveFileName(&ofn)) {
            PostMessage(hwnd, APP_THREADEND, 0, 0);
            return IDE_CANCEL;
        }

        hFile = CreateFile(tcFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if(hFile == INVALID_HANDLE_VALUE) {
            PostMessage(hwnd, APP_THREADEND, 0, 0);
            return IDE_CANNOTOPENFILE;
        }
    }

    if(!status.onlycnt && !status.usefile) OutputToEditbox(hEdi_out, tcMes[34]);
    else if(!status.onlycnt && status.usefile) {
#ifdef UNICODE
        mblen = WideCharToMultiByte(status.charset?932:65001, NULL, tcMes[34], lstrlen(tcMes[34])+1, NULL, 0, NULL, NULL); // 変換後文字数取得
        WideCharToMultiByte(status.charset?932:65001, NULL, tcMes[34], lstrlen(tcMes[34])+1, mbStr, mblen, NULL, NULL);
        if(!WriteFile(hFile, mbStr, (mblen-1)*sizeof(CHAR), &dwTemp, NULL)) {
#else
        if(!WriteFile(hFile, tcMes[34], lstrlenA(tcmes[28])*sizeof(CHAR), &dwTemp, NULL)) {
#endif
            PostMessage(hwnd, APP_THREADEND, 0, 0);
            return IDE_CANNOTWRITEFILE;
        }
        OutputToEditbox(hEdi_out, tcMes[38]);
    }

    // 計算
    for(ULONGLONG i=num[0]; i<=num[1] && cnt<num[2]; i++) {
        for(ULONGLONG j=2; j<=i && !isAborted; j++) {
            if(i%j==0 && i!=j) break;
            if(i/j<j || i==j) {
                if(!status.onlycnt) {
                    if(cnt) {
                        wsprintf(tcStr1, TEXT(", %I64u"), i);
                        if(status.usefile) wsprintfA(mbStr, ", %I64u", i);
                    } else {
                        wsprintf(tcStr1, TEXT("%I64u"), i);
                        if(status.usefile) wsprintfA(mbStr, "%I64u", i);
                    }
                    if(status.usefile) WriteFile(hFile, mbStr, lstrlenA(mbStr)*sizeof(CHAR), &dwTemp, NULL);
                    else OutputToEditbox(hEdi_out, tcStr1);
                }
                cnt++;
                break;
            }
            if(j!=2) j++; // 2以外ならもう1つ更に増やす
        }
        if(isAborted || i==0xffffffffffffffff) break;
        if(i!=2) i++; // 2以外ならもう1つ更に増やす
    }

    // 最後の出力など
    if(!status.onlycnt && !status.usefile) OutputToEditbox(hEdi_out, TEXT("\r\n"));
    else if(!status.onlycnt && status.usefile) WriteFile(hFile, "\r\n", 2*sizeof(CHAR), &dwTemp, NULL);
    wsprintf(tcStr2, tcMes[isAborted?42:41], cnt, num[0], num[1], num[2]);
    if(!status.onlycnt && status.usefile) {
#ifdef UNICODE
        mblen = WideCharToMultiByte(status.charset?932:65001, NULL, tcStr2, lstrlen(tcStr2), NULL, 0, NULL, NULL); // 変換後文字数
        WideCharToMultiByte(status.charset?932:65001, NULL, tcStr2, lstrlen(tcStr2), mbStr, mblen, NULL, NULL);
        WriteFile(hFile, mbStr, mblen*sizeof(CHAR), &dwTemp, NULL);
#else
        WriteFile(hfile, tcStr2, lstrlenA(tcStr2)*sizeof(CHAR), &dwtemp, NULL);
#endif
        CloseHandle(hFile);
    }
    OutputToEditbox(hEdi_out, tcStr2);
    SendMessage(hEdi_out, EM_REPLACESEL, 0, (WPARAM)TEXT("\r\n")); // '\r': CR, '\n': LF
    if(isAborted){
        PostMessage(hwnd, APP_THREADEND, 0, 0);
        return IDE_ABORT;
    }
    wsprintf(tcStr1, TEXT(" - %s"), tcMes[0]);
    lstrcat(tcStr2, tcStr1);
    SetWindowText(hwnd, tcStr2);
    PostMessage(hwnd, APP_THREADEND, 0, 0);
    return IDE_SUCCESS;
}