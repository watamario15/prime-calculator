#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <climits>
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
#define WND_CLASS_NAME TEXT("Prime Factorization Main") // ウィンドウクラス名
#define APP_THREADEND WM_APP // スレッド終了メッセージ (WM_APP から 0xBFFF までは独自に使える)

enum { // 定数定義
    IDC_BUTTON_OK = 0,            // -----
    IDC_BUTTON_ABORT = 1,         //
    IDC_BUTTON_CLEAR = 2,         //
    IDC_EDIT_IN1 = 20,            // コントロール ID
    IDC_EDIT_IN2 = 21,            //
    IDC_EDIT_IN3 = 22,            //
    IDC_EDIT_OUT = 23,            // -----
    IDM_FILE_SAVE_AS = 100,       // -----
    IDM_FILE_EXIT = 101,          //
    IDM_EDIT_CUT = 120,           //
    IDM_EDIT_COPY = 121,          //
    IDM_EDIT_PASTE = 122,         //
    IDM_EDIT_SELECT_ALL = 123,    //
    IDM_OPT_PF = 140,             //
    IDM_OPT_LCPN = 141,           // メニュー ID
    IDM_OPT_ONLYNUM = 142,        //
    IDM_OPT_OUTFILE = 143,        //
    IDM_OPT_LANG_JA = 145,        //
    IDM_OPT_LANG_EN = 146,        //
    IDM_OPT_CHARSET_UTF8 = 147,   //
    IDM_OPT_CHARSET_SJIS = 148,   //
    IDM_HELP_HOWTOUSE = 160,      //
    IDM_HELP_ABOUT = 161,         // -----
    IDS_JA = 0,                   // 日本語 String Table ID
    IDS_EN = 100,                 // 英語 String Table ID
    IDE_SUCCESS = 0,              // -----
    IDE_INVALID = 1,              //
    IDE_ABORT = 2,                // 計算スレッドの終了コード
    IDE_CANCEL = 3,               //
    IDE_CANNOTOPENFILE = 4,       //
    IDE_CANNOTWRITEFILE = 5,      // -----
    SIZE_OF_STRING_TABLE = 50,    // String Table の項目数
    MAX_INPUT_LENGTH = 21,        // -----
    MAX_OUTPUT_BUFFER = 65536,    // バッファサイズ(ヌル終端を含む)
    MAX_BUFFER = 1024             // -----
};

struct PFSTATUS { // 真偽値用ビットフィールド
    unsigned int mode: 1; // 素因数分解: 0, 素数列挙: 1
    unsigned int onlycnt: 1; // (素数列挙)個数のみ出力モード
    unsigned int usefile: 1; // (素数列挙)ファイル出力
    unsigned int charset: 1; // UTF-8: 0, Shift_JIS: 1
    unsigned int isWorking: 1; // 計算処理中
} status = {0};

HWND hBtn_ok, hBtn_abort, hBtn_clr, hEdi0, hEdi1, hEdi2, hEdi_out, hWnd_focused; // ウィンドウハンドル
HINSTANCE hInst; // Instance Handle のグローバルバックアップ
HMENU hMenu; // メニューハンドル
HDC hMemDC; // ダブルバッファリング用 device context のハンドル
HFONT hFmes = NULL, hFbtn = NULL, hFedi = NULL, hFnote = NULL; // 作成するフォント
HBRUSH hBrush = NULL, hBshSys; // 取得するブラシ
HPEN hPen = NULL, hPenSys; // 取得するペン
HANDLE hThread; // 計算スレッドのハンドル
INT btnsize[2]; // ボタンサイズ ([0]: x, [1]: y)
ULONGLONG num[3]; // 入力値受付用変数
volatile bool isAborted = false; // 計算中断フラグ (volatile を外すと最適化で中断が効かなくなる)
TCHAR tcTemp[MAX_BUFFER] /* 仮バッファ */, tcMes[SIZE_OF_STRING_TABLE][MAX_BUFFER] /* リソース文字列受け取り用バッファ */;

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

// エディットボックスの末尾に文字列を追加
void OutputToEditbox(HWND hwnd, LPCTSTR arg) {
    INT editlen = (INT)SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0); // 文字列長を取得
    SendMessage(hwnd, EM_SETSEL, editlen, editlen); // 末尾にキャレットを移動 (選択範囲の最初と最後を共に末尾指定)
    SendMessage(hwnd, EM_REPLACESEL, 0, (WPARAM)arg); // キャレット位置に追記
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
    wcl.hIcon = LoadIcon(hInstance, TEXT("ResIcon")); // アイコン
    wcl.hIconSm = LoadIcon(hInstance, TEXT("ResIcon")); // 小アイコン
    wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcl.lpszMenuName = 0;
    wcl.cbClsExtra = 0;
    wcl.cbWndExtra = 0;
    wcl.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    if(!RegisterClassEx(&wcl)) { // クラスの登録
        MessageBox(NULL, tcMes[43], tcMes[19], MB_OK | MB_ICONERROR);
        return 1;
    }

    // サイズは WM_SIZE で調整するので、生成時は適当な値を入れておく
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
    if(!hWnd) {
        MessageBox(NULL, tcMes[44], tcMes[19], MB_OK | MB_ICONERROR);
        return 1;
    }

    hBtn_ok = CreateWindowEx( // OK ボタン
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
    SetTimer(hWnd, 1, 16, NULL); // 背景色変化etc用タイマーをセット

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
        HANDLE_MSG(hwnd, WM_TIMER, Main_OnTimer); // タイマー時
        HANDLE_MSG(hwnd, WM_SIZE, Main_OnSize); // ウィンドウサイズ変更時
        HANDLE_MSG(hwnd, WM_PAINT, Main_OnPaint); // 再描画要求を受けたとき
        HANDLE_MSG(hwnd, WM_COMMAND, Main_OnCommand); // ユーザ操作など

        case WM_ACTIVATE: // アクティブ化
            // 以前フォーカスが当たっていたエディットにフォーカスを戻す
            if(LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE) SetFocus(hWnd_focused);
            break;

        case 0x02e0: // DPI 値変更時 (WM_DPICHANGED マクロが XP 用 SDK に無いため数値指定)
            MoveWindow(hwnd, ((PRECT)lParam)->left, ((PRECT)lParam)->top, ((PRECT)lParam)->right - ((PRECT)lParam)->left,
                ((PRECT)lParam)->bottom - ((PRECT)lParam)->top, FALSE);
            break;

        case APP_THREADEND: // 計算スレッド完了時 (独自メッセージ)
            WaitForSingleObject(hThread, INFINITE); // スレッド終了待ち
            GetExitCodeThread(hThread, &dwTemp); // 終了コード受け取り
            CloseHandle(hThread); // スレッドハンドルを閉じる
            switch(dwTemp) {
                case IDE_INVALID: // 無効入力
                    if(!status.mode) {
                        MessageBox(hwnd, tcMes[31], tcMes[19], MB_OK | MB_ICONWARNING);
                        OutputToEditbox(hEdi_out, tcMes[32]);
                    } else {
                        MessageBox(hwnd, tcMes[35], tcMes[19], MB_OK | MB_ICONWARNING);
                        OutputToEditbox(hEdi_out, tcMes[36]);
                    }
                    break;

                case IDE_ABORT: // 中断
                    isAborted = false;
                    break;

                case IDE_CANCEL: // キャンセル
                    break;

                case IDE_CANNOTOPENFILE: // 出力ファイルを開けない
                    MessageBox(hwnd, tcMes[18], tcMes[19], MB_OK | MB_ICONWARNING);
                    OutputToEditbox(hEdi_out, tcMes[38]);
                    break;

                case IDE_CANNOTWRITEFILE: // 出力ファイルに書き込めない
                    MessageBox(hwnd, tcMes[22], tcMes[19], MB_OK | MB_ICONWARNING);
                    OutputToEditbox(hEdi_out, tcMes[37]);
            }
            if(dwTemp) { // 全エラーで共通の処理 (正常終了の終了コードは0)
                wsprintf(tcTemp, TEXT("%s - %s"), tcMes[0], TARGET_CPU); // 初期タイトル生成
                SetWindowText(hwnd, tcTemp);
            }
            if(status.mode) { // 素数列挙専用の処理
                EnableWindow(hEdi1, TRUE);
                if(!status.onlycnt) EnableWindow(hEdi2, TRUE);
            }
            EnableWindow(hBtn_ok, TRUE);
            EnableWindow(hEdi0, TRUE);
            EnableWindow(hBtn_abort, FALSE);
            EnableMenuItem(hMenu, 2, MF_BYPOSITION | MF_ENABLED); // 「オプション」メニューを再度有効化
            DrawMenuBar(hwnd); // メニュー再描画
            Paint(hwnd); // 再描画
            SetFocus(hWnd_focused); // 以前のフォーカス位置にフォーカスを当てる
            status.isWorking = 0; // 計算中フラグを下ろす
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
    WNDPROC defProc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA); // USERDATA 領域に保存した標準 Window Procedure を取得

    switch(uMsg) {
        case WM_DESTROY:
            SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)defProc); // 元の Window Procedure に戻す
            break;

        case WM_CHAR:
            switch((CHAR)wParam) {
                case VK_RETURN: // Enter
                    if(!status.isWorking && !status.mode) StartCalc(hwnd); // 計算処理実行
                    else if(status.mode) SetFocus(hEdi1); // 次にフォーカスを移動
                    return 0;
            } // 実行対象でなければ default に流す(間に別のメッセージ入れちゃだめ!!)
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
            switch((CHAR)wParam) {
                case VK_RETURN: // Enter
                    SetFocus(hEdi2); // 次にフォーカスを移動
                    return 0;
            } // 実行対象でなければ default に流す(後に別のメッセージ入れちゃだめ!!)
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
            switch((CHAR)wParam) {
                case VK_RETURN: // Enter
                    if(!status.isWorking) StartCalc(hwnd); // 計算処理開始
                    return 0;
            } // 実行対象でなければ default に流す(間に別のメッセージ入れちゃだめ!!)
        default:
            return CallWindowProc(defProc, hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

BOOL Main_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
    INT scrx, scry;

    // メニューの初期化
    SetMenu(hwnd, hMenu);
    if(GetUserDefaultUILanguage() == 0x0411) // UI 言語が日本語
        CheckMenuRadioItem(hMenu, IDM_OPT_LANG_JA, IDM_OPT_LANG_EN, IDM_OPT_LANG_JA, MF_BYCOMMAND);
    else // UI 言語がその他
        CheckMenuRadioItem(hMenu, IDM_OPT_LANG_JA, IDM_OPT_LANG_EN, IDM_OPT_LANG_EN, MF_BYCOMMAND);
    CheckMenuRadioItem(hMenu, IDM_OPT_PF, IDM_OPT_LCPN, IDM_OPT_PF, MF_BYCOMMAND); // 素因数分解
    CheckMenuRadioItem(hMenu, IDM_OPT_CHARSET_UTF8, IDM_OPT_CHARSET_SJIS, IDM_OPT_CHARSET_UTF8, MF_BYCOMMAND); // UTF-8
    EnableMenuItem(hMenu, IDM_OPT_ONLYNUM, MF_BYCOMMAND | MF_GRAYED); // 素因数分解では使わない項目を無効化
    EnableMenuItem(hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_GRAYED); // 同様に無効化

    hMemDC = CreateCompatibleDC(NULL); // ディスプレイに合うデバイスコンテキストを生成
    hBshSys = CreateSolidBrush(GetSysColor(COLOR_BTNFACE)); // ボタン色のブラシを作成
    hPenSys = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE)); // ボタン色のペンを作成

    hEdi0 = CreateWindowEx( // 入力ボックス
        0,
        TEXT("EDIT"),
        TEXT(""),
        WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
        0, 0, 0, 0, // サイズ調整は WM_SIZE で行うので、ここでは適当な値を入れておく
        hwnd,
        (HMENU)IDC_EDIT_IN1,
        lpCreateStruct->hInstance,
        NULL);
    if(!hEdi0) {
        MessageBox(hwnd, tcMes[44], tcMes[19], MB_OK | MB_ICONERROR);
        return 1;
    }
    SendMessage(hEdi0, EM_SETLIMITTEXT, (WPARAM)(MAX_INPUT_LENGTH-1), 0); // 符号なし64ビット整数の最大桁数に設定
    WNDPROC wpedi0_old = (WNDPROC)SetWindowLongPtr(hEdi0, GWLP_WNDPROC, (LONG_PTR)Edit0WindowProc); // サブクラス化
    SetWindowLongPtr(hEdi0, GWLP_USERDATA, (LONG_PTR)wpedi0_old); // USERDATA に標準 Window Procedure を保存
    SetFocus(hWnd_focused = hEdi0); // ここにフォーカスを当て、直近フォーカス位置も更新する

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
    SendMessage(hEdi_out, EM_SETLIMITTEXT, (WPARAM)(MAX_OUTPUT_BUFFER-1), 0);

    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST); // 現在のモニタを取得
    MONITORINFOEX mInfo; mInfo.cbSize = sizeof(MONITORINFOEX); // 構造体初期化
    GetMonitorInfo(hMonitor, &mInfo); // モニタ情報を取得
    // 適切なウィンドウサイズを計算し、そのサイズに変更
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
    // メッセージボックスを出し、「はい」が押されると終了
    if(IDYES == MessageBox(hwnd, tcMes[12], tcMes[13], MB_YESNO | MB_ICONINFORMATION)) {
        KillTimer(hwnd, 1); // 背景色変化etc用のタイマーを止める
        DestroyWindow(hwnd);
    }
    else SetFocus(hWnd_focused); // 「いいえ」なら以前フォーカスが当たっていたエディットボックスにフォーカスを戻して続行
}

void Main_OnTimer(HWND hwnd, UINT id) {
    static BYTE r=0, g=255, b=255; // 背景色
    HWND hwnd_temp;

    if((hwnd_temp = GetFocus()) && (hwnd_temp == hEdi0 || hwnd_temp == hEdi1 || hwnd_temp == hEdi2 || hwnd_temp == hEdi_out))
        hWnd_focused = hwnd_temp; // 現在のフォーカス位置を取得

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

    if(hBrush) DeleteObject(hBrush); // 元のブラシを破棄
    if(hPen) DeleteObject(hPen); // 元のペンを破棄
    hBrush = CreateSolidBrush(RGB(r, g, b)); // 新ブラシを作成(塗りつぶし用)
    hPen = CreatePen(PS_SOLID, 1, RGB(r, g, b)); // 新ペンを作成(輪郭用)
    Paint(hwnd); // 再描画
}

void Main_OnSize(HWND hwnd, UINT state, int cx, int cy) {
    INT scrx, scry;
    RECT rect;
    LOGFONT rLogfont; // 作成するフォントに関する情報
    static HBITMAP hBitmap = NULL; // ダブルバッファリング用 bitmap のハンドル

    GetClientRect(hwnd, &rect); // ウィンドウの描画領域サイズを取得
    scrx = rect.right; scry = rect.bottom; // 幅と高さをバックアップ

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
    lstrcpy(rLogfont.lfFaceName, TEXT("MS Shell Dlg")); // どの言語版 OS でも適切なフォントが選ばれる指定方法
    if(hFmes) DeleteObject(hFmes); // 旧フォントを破棄
    hFmes = CreateFontIndirect(&rLogfont); // 新フォントを作成

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

    // 注釈用のフォントを作成 (ボタン用フォントからサイズだけ変更)
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

    if(hBitmap) DeleteObject(hBitmap); // 旧 bitmap を破棄
    HDC hdc = GetDC(hwnd); // デバイスコンテキストを取得
    if(!(hBitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom))) { // 新サイズの bitmap を作成
        MessageBox(hwnd, tcMes[46], tcMes[19], MB_OK | MB_ICONERROR);
        PostQuitMessage(1);
        return;
    }
    ReleaseDC(hwnd, hdc); // デバイスコンテキストを解放
    SelectObject(hMemDC, hBitmap); // 新 bitmap を Memory Device Context に設定
    Paint(hwnd); //再描画
}

void Main_OnPaint(HWND hwnd) {
    RECT rect;
    PAINTSTRUCT ps;

    GetClientRect(hwnd, &rect); // ウィンドウの描画領域サイズを取得

    HDC hdc = BeginPaint(hwnd, &ps); // 再描画要求に基づく再描画の開始
    BitBlt(hdc, 0, 0, rect.right, rect.bottom, hMemDC, 0, 0, SRCCOPY); // ダブルバッファリングのデータを転送
    EndPaint(hwnd, &ps); // 再描画完了
}

void Main_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
    INT editlen /* 出力ボックス内容の長さ */, mblen /* 文字コード変換後の長さ */;
    UINT menu[5]; // メニュー状態のバックアップ用変数
    PTSTR tcEdit; // 出力ボックスの内容を受け取る領域のポインタ
    PSTR mbEdit; // 文字コード変換した結果を受け取る領域のポインタ
    DWORD dwTemp; // DWORD 型仮変数
    HANDLE hFile /* ファイルハンドル */, hHeap /* ヒープハンドル */;
    OPENFILENAME *ofn; // ファイル選択用構造体
    WNDPROC defProc; // 標準 Window Procedure
    RECT rect;

    switch(id) {
        case IDC_BUTTON_OK: // OK ボタン
            if(status.isWorking) break; // 計算中なら抜ける
            StartCalc(hwnd); // 計算処理開始
            break;

        case IDC_BUTTON_ABORT: // 中断ボタン
            isAborted = true; // 中断フラグを立てる
            break;

        case IDC_BUTTON_CLEAR: // 履歴消去
            SetWindowText(hEdi_out, TEXT("")); // 出力ボックスに空文字列を設定
            break;

        case IDM_FILE_SAVE_AS: // テキストファイルに保存
#ifndef UNICODE
            MessageBox(hwnd, tcMes[23], tcMes[21], MB_OK | MB_ICONINFORMATION);
#endif
            hHeap = GetProcessHeap(); // プロセスヒープ取得
            ofn = (OPENFILENAME *)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(OPENFILENAME)); // 構造体領域確保
            if(!ofn) {
                MessageBox(hwnd, tcMes[45], tcMes[19], MB_OK | MB_ICONWARNING);
                break;
            }
            ofn->lStructSize = sizeof(OPENFILENAME);
            ofn->hwndOwner = hwnd;
            ofn->lpstrFilter = TEXT("Text File (*.txt)\0*.txt\0")
                               TEXT("All files (*.*)\0*.*\0");
            ofn->lpstrFile = (PTSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, MAX_PATH*sizeof(TCHAR)); // ファイル名領域確保
            if(!ofn->lpstrFile) {
                MessageBox(hwnd, tcMes[45], tcMes[19], MB_OK | MB_ICONWARNING);
                HeapFree(hHeap, 0, ofn);
                break;
            }
            ofn->nMaxFile = MAX_PATH;
            ofn->lpstrDefExt = TEXT(".txt");
            ofn->lpstrTitle = tcMes[17];
            ofn->Flags = OFN_OVERWRITEPROMPT;
            if(!GetSaveFileName(ofn)) {
                HeapFree(hHeap, 0, ofn->lpstrFile);
                HeapFree(hHeap, 0, ofn);
                break;
            }
            // ファイル作成 (あれば上書き)
            hFile = CreateFile(ofn->lpstrFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if(hFile == INVALID_HANDLE_VALUE) { // 開けなかった場合
                MessageBox(hwnd, tcMes[18], tcMes[19], MB_OK | MB_ICONWARNING);
                HeapFree(hHeap, 0, ofn->lpstrFile);
                HeapFree(hHeap, 0, ofn);
                break;
            }
            editlen = GetWindowTextLength(hEdi_out) + 1; // ヌル終端込みの長さ
            tcEdit = (PTSTR)HeapAlloc(hHeap, 0, editlen*sizeof(TCHAR)); // 領域確保
            if(!tcEdit) {
                MessageBox(hwnd, tcMes[45], tcMes[19], MB_OK | MB_ICONWARNING);
                HeapFree(hHeap, 0, ofn->lpstrFile);
                HeapFree(hHeap, 0, ofn);
                break;
            }
            GetWindowText(hEdi_out, tcEdit, editlen); // 出力ボックスの内容を取得
#ifdef UNICODE
            mblen = WideCharToMultiByte(status.charset?932:65001, NULL, tcEdit, editlen, NULL, 0, NULL, NULL); // 変換後サイズ取得
            mbEdit = (PSTR)HeapAlloc(hHeap, 0, mblen*sizeof(CHAR)); // 変換後文字列の領域確保
            if(!mbEdit) {
                MessageBox(hwnd, tcMes[45], tcMes[19], MB_OK | MB_ICONWARNING);
                HeapFree(hHeap, 0, ofn->lpstrFile);
                HeapFree(hHeap, 0, ofn);
                HeapFree(hHeap, 0, tcEdit);
                break;
            }
            WideCharToMultiByte(status.charset?932:65001, NULL, tcEdit, editlen, mbEdit, mblen, NULL, NULL); // 文字コード変換
            if(WriteFile(hFile, mbEdit, (mblen-1)*sizeof(CHAR), &dwTemp, NULL)) // 書き込み
#else
            if(WriteFile(hFile, tcEdit, (editlen-1)*sizeof(CHAR), &dwTemp, NULL)) // ANSI ビルドなら変換せずに書き込み
#endif
                MessageBox(hwnd, tcMes[20], tcMes[21], MB_OK | MB_ICONINFORMATION); // 出力成功
            else
                MessageBox(hwnd, tcMes[22], tcMes[19], MB_OK | MB_ICONWARNING); // 出力失敗
            CloseHandle(hFile); // ファイルハンドルを閉じる
            HeapFree(hHeap, 0, ofn->lpstrFile);
            HeapFree(hHeap, 0, ofn);
            HeapFree(hHeap, 0, tcEdit);
#ifdef UNICODE
            HeapFree(hHeap, 0, mbEdit);
#endif
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
            DestroyWindow(hEdi1); // 素数列挙で追加されたエディットボックスを削除
            DestroyWindow(hEdi2); // 同様に削除
            status.mode = 0;
            GetClientRect(hwnd, &rect);
            PostMessage(hwnd, WM_SIZE, 0, MAKEWPARAM(rect.right, rect.bottom)); // 再配置させるために WM_SIZE を発行
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
            if(!hEdi1) {
                MessageBox(hwnd, tcMes[44], tcMes[19], MB_OK | MB_ICONERROR);
                PostQuitMessage(1);
                break;
            }
            SendMessage(hEdi1, EM_SETLIMITTEXT, (WPARAM)(MAX_INPUT_LENGTH-1), 0); // 符号なし64ビット整数の最大桁数に設定
            defProc = (WNDPROC)SetWindowLongPtr(hEdi1, GWLP_WNDPROC, (LONG_PTR)Edit1WindowProc); // サブクラス化
            SetWindowLongPtr(hEdi1, GWLP_USERDATA, (LONG_PTR)defProc); // 元の Window Procedure を USERDATA に保存

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
                break;
            }
            SendMessage(hEdi2, EM_SETLIMITTEXT, (WPARAM)(MAX_INPUT_LENGTH-1), 0);
            defProc = (WNDPROC)SetWindowLongPtr(hEdi2, GWLP_WNDPROC, (LONG_PTR)Edit2WindowProc);
            SetWindowLongPtr(hEdi2, GWLP_USERDATA, (LONG_PTR)defProc);

            status.mode = 1;
            GetClientRect(hwnd, &rect);
            PostMessage(hwnd, WM_SIZE, 0, MAKEWPARAM(rect.right, rect.bottom)); // 再配置させるために WM_SIZE を発行
            break;

        case IDM_OPT_ONLYNUM: // 「個数のみ表示」
            if(GetMenuState(hMenu, IDM_OPT_ONLYNUM, MF_BYCOMMAND) & MF_CHECKED) { // 既にチェックがあれば外す
                CheckMenuItem(hMenu, IDM_OPT_ONLYNUM, MF_BYCOMMAND | MF_UNCHECKED);
                EnableMenuItem(hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_ENABLED);
                EnableWindow(hEdi2, TRUE);
                status.onlycnt = 0;
            } else { // チェックが無ければ付ける
                CheckMenuItem(hMenu, IDM_OPT_ONLYNUM, MF_BYCOMMAND | MF_CHECKED);
                EnableMenuItem(hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(hEdi2, FALSE);
                status.onlycnt = 1;
            }
            break;

        case IDM_OPT_OUTFILE: // 「テキストファイルに出力」
            if(GetMenuState(hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND) & MF_CHECKED) { // 既にチェックがあれば外す
                CheckMenuItem(hMenu, IDM_OPT_OUTFILE, MF_BYCOMMAND | MF_UNCHECKED);
                status.usefile = 0;
            } else { // チェックが無ければ付ける
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

            for(int i=0; i < SIZE_OF_STRING_TABLE; i++) // 日本語 String Table 読み込み
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

            for(int i=0; i < SIZE_OF_STRING_TABLE; i++) // 英語 String Table 読み込み
                LoadString(hInst, IDS_EN+i, tcMes[i], sizeof(tcMes[0])/sizeof(tcMes[0][0]));
            SetWindowText(hBtn_ok, tcMes[9]);
            SetWindowText(hBtn_abort, tcMes[10]);
            SetWindowText(hBtn_clr, tcMes[11]);
            wsprintf(tcTemp, TEXT("%s - %s"), tcMes[0], TARGET_CPU);
            SetWindowText(hwnd, tcTemp);
            break;

        case IDM_OPT_CHARSET_UTF8: // 出力文字コードを UTF-8 に変更
            if(!status.charset) break;
            CheckMenuRadioItem(hMenu, IDM_OPT_CHARSET_UTF8, IDM_OPT_CHARSET_SJIS, IDM_OPT_CHARSET_UTF8, MF_BYCOMMAND);
            status.charset = 0;
            break;

        case IDM_OPT_CHARSET_SJIS: // 出力文字コードを Shift_JIS に変更
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

// 再描画
void Paint(HWND hwnd) {
    INT scrx, scry;
    RECT rect;

    GetClientRect(hwnd, &rect); // ウィンドウの描画領域サイズを取得
    scrx = rect.right; scry = rect.bottom; // 幅・高さのバックアップ

    SelectObject(hMemDC, hPen); // デバイスコンテキストにペンを設定
    SelectObject(hMemDC, hBrush); // デバイスコンテキストにブラシを設定
    Rectangle(hMemDC, rect.left, rect.top, rect.right, rect.bottom); // 領域いっぱいに四角形を描く

    if(!status.mode) { // 素因数分解モード
        // ラベル背景
        SelectObject(hMemDC, hPenSys);
        SelectObject(hMemDC, hBshSys);
        Rectangle(hMemDC, 0, 0, btnsize[0], btnsize[1]);
        // ラベル文字
        SetBkMode(hMemDC, TRANSPARENT);
        SetTextColor(hMemDC, RGB(0, 0, 0));
        SelectObject(hMemDC, hFnote);
        rect.right = btnsize[0]; rect.bottom = btnsize[1];
        DrawText(hMemDC, tcMes[24], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    } else { // 素数列挙モード
        // ラベル背景
        SelectObject(hMemDC, hPenSys);
        SelectObject(hMemDC, hBshSys);
        Rectangle(hMemDC, 0, 0, btnsize[0], btnsize[1]);
        Rectangle(hMemDC, btnsize[0]*5, 0, btnsize[0]*6, btnsize[1]);
        Rectangle(hMemDC, 0, btnsize[1], btnsize[0], btnsize[1]*2);
        // ラベル文字
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

    // 中央のメッセージ出力
    if(status.isWorking) DrawText(hMemDC, tcMes[28], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    else if(!status.mode) DrawText(hMemDC, tcMes[29], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    else if(status.mode) DrawText(hMemDC, tcMes[30], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    InvalidateRect(hwnd, NULL, FALSE); // 実際の画面の再描画要求を発行
}

void StartCalc(HWND hwnd) {
    DWORD dwTemp; // DWORD 型仮変数
    bool isErange = false; // ラップアラウンドの有無

    status.isWorking = 1; // 計算処理中フラグを立てる
    EnableWindow(hBtn_ok, FALSE);    // -----
    EnableWindow(hEdi0, FALSE);      //
    if(status.mode) {                //
        EnableWindow(hEdi1, FALSE);  // 計算処理中は使えないコントロールを無効化
        EnableWindow(hEdi2, FALSE);  //
    }                                //
    EnableWindow(hBtn_abort, TRUE);  // -----
    EnableMenuItem(hMenu, 2, MF_BYPOSITION | MF_GRAYED); // 「オプション」メニューをグレーアウト
    DrawMenuBar(hwnd); // メニュー再描画

    SendMessage(hEdi0, WM_GETTEXT, MAX_INPUT_LENGTH, (LPARAM)tcTemp); // 入力値取得
    errno = 0;
    num[0] = _tcstoull(tcTemp, NULL, 10); // 数値に変換
    isErange = (errno == ERANGE); // ラップアラウンド有無を記録
    if(status.mode) {
        SendMessage(hEdi1, WM_GETTEXT, MAX_INPUT_LENGTH, (LPARAM)tcTemp);
        errno = 0;
        num[1] = _tcstoull(tcTemp, NULL, 10);
        isErange = isErange || (errno == ERANGE); // ラップアラウンド有無を記録 (論理和)
        SendMessage(hEdi2, WM_GETTEXT, MAX_INPUT_LENGTH, (LPARAM)tcTemp);
        errno = 0;
        num[2] = _tcstoull(tcTemp, NULL, 10);
        isErange = isErange || (errno == ERANGE);
    }
    if(isErange) MessageBox(hwnd, tcMes[47], tcMes[21], MB_OK | MB_ICONINFORMATION); // ラップアラウンド検出時
    SetWindowText(hwnd, tcMes[16]); // ウィンドウタイトルを変更

    // 計算スレッド開始
    if(!status.mode) hThread = CreateThread(NULL, 0, PrimeFactorization, (LPVOID)hwnd, 0, &dwTemp);
    else hThread = CreateThread(NULL, 0, ListPrimeNumbers, (LPVOID)hwnd, 0, &dwTemp);
    if(hThread) SetThreadPriority(hThread, THREAD_PRIORITY_BELOW_NORMAL); // システムの動作を重くしないよう、通常未満の優先度に設定
    else { // スレッド生成失敗時
        MessageBox(hwnd, tcMes[48], tcMes[19], MB_OK | MB_ICONWARNING);
        OutputToEditbox(hEdi_out, tcMes[49]);
        wsprintf(tcTemp, TEXT("%s - %s"), tcMes[0], TARGET_CPU); // 初期タイトル生成
        SetWindowText(hwnd, tcTemp);
        if(status.mode) { // 素数列挙専用の処理
            EnableWindow(hEdi1, TRUE);
            if(!status.onlycnt) EnableWindow(hEdi2, TRUE);
        }
        EnableWindow(hBtn_ok, TRUE);
        EnableWindow(hEdi0, TRUE);
        EnableWindow(hBtn_abort, FALSE);
        EnableMenuItem(hMenu, 2, MF_BYPOSITION | MF_ENABLED); // 「オプション」メニューを再度有効化
        DrawMenuBar(hwnd); // メニュー再描画
        Paint(hwnd); // 再描画
        SetFocus(hWnd_focused); // 以前のフォーカス位置にフォーカスを当てる
        status.isWorking = 0; // 計算中フラグを下ろす
    }
}

// 素因数分解スレッド (lpParameter にはウィンドウハンドルを指定)
DWORD WINAPI PrimeFactorization(LPVOID lpParameter) {
    ULONGLONG N = num[0] /* 割られる数(入力値で初期化) */, cnt = 0 /* 素因数の個数 */, i = 2 /* 割る数(素因数候補) */;
    bool chk = false;
    TCHAR tcStr1[MAX_BUFFER] = TEXT(""), tcStr2[MAX_BUFFER] = TEXT("");
    HWND hwnd = (HWND)lpParameter;

    if(N<=0) {
        PostMessage(hwnd, APP_THREADEND, 0, 0); // スレッド終了メッセージを発行(待機無し)
        return IDE_INVALID;
    }

    // 素因数分解 (試し割り法)
    while(1) {
        while(i<=N && !isAborted) {
            if(N%i==0 && N!=i) { // 素因数発見
                chk = true;
                break;
            }
            if(N/i<i || N==i) { // N は素数
                chk = false;
                break;
            }
            if(i==2) i++;
            else i+=2; // 2以外の偶数での割り算はスキップ
        }
        if(isAborted) break; // 中断
        if(chk) { // 素因数発見
            wsprintf(tcStr2, TEXT("%I64ux"), i); // 見つけた素因数を文字列に変換し「x」を足す
            lstrcat(tcStr1, tcStr2); // 結果文字列に追加
        } else { // N は素数
            wsprintf(tcStr2, TEXT("%I64u"), N); // その数自身を文字列に変換
            lstrcat(tcStr1, tcStr2); // 結果文字列に追加
            break;
        }
        N/=i; // 見つけた素因数で N を割り、繰り返す(素因数は大きくなっていくため、割る数 i の初期化は不要)
        cnt++; // 素因数の個数を加算
    }
    if(cnt==0 && N>1) lstrcat(tcStr1, tcMes[33]); // 素数だった場合、その旨を追記

    if(isAborted) { // 中断
        PostMessage(hwnd, APP_THREADEND, 0, 0);
        return IDE_ABORT;
    }

    wsprintf(tcStr2, TEXT("%s%I64u = %s"), tcMes[34], num[0], tcStr1); // 結果文字列の生成
    OutputToEditbox(hEdi_out, tcStr2); // 結果出力
    SendMessage(hEdi_out, EM_REPLACESEL, 0, (WPARAM)TEXT("\r\n")); // キャレットが末尾にあることが確実ならこれだけで良い
    wsprintf(tcStr1, TEXT(" - %s"), tcMes[0]); // ウィンドウタイトル後半を構築
    lstrcat(tcStr2, tcStr1); // 処理結果に結合
    SetWindowText(hwnd, tcStr2); // ウィンドウタイトル更新

    PostMessage(hwnd, APP_THREADEND, 0, 0);
    return IDE_SUCCESS;
}

// 素数列挙スレッド (lpParameter にはウィンドウハンドルを指定)
DWORD WINAPI ListPrimeNumbers(LPVOID lpParameter) {
    ULONGLONG cnt = 0; // 素数の個数
    DWORD dwTemp; // DWORD 型仮変数
    int mblen; // ヌル終端込みの文字コード変換後文字列長
    TCHAR tcStr1[MAX_BUFFER], tcStr2[MAX_BUFFER], tcFile[MAX_PATH] = {0};
    CHAR mbStr[MAX_BUFFER]; // 文字コード変換後文字列
    HANDLE hFile = NULL;
    OPENFILENAME ofn = {0};
    HWND hwnd = (HWND)lpParameter;

    // 入力値を調整(主に空or"0"の時の対応)
    if(num[0]<2) num[0] = 2;
    if(num[0]!=2 && !(num[0]%2)) num[0]++;
    if(num[1]<1) num[1] = ULLONG_MAX;
    if(num[2]<1) num[2] = ULLONG_MAX;
    if(num[0]>num[1]) { // 上端より下端の方が大きいとき
        PostMessage(hwnd, APP_THREADEND, 0, 0); // スレッド終了メッセージを発行(待機無し)
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
        // ファイルを作成 (あれば上書き)
        hFile = CreateFile(tcFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if(hFile == INVALID_HANDLE_VALUE) {
            PostMessage(hwnd, APP_THREADEND, 0, 0);
            return IDE_CANNOTOPENFILE;
        }
    }

    if(!status.onlycnt && !status.usefile) OutputToEditbox(hEdi_out, tcMes[34]);
    else if(!status.onlycnt && status.usefile) {
#ifdef UNICODE
        mblen = WideCharToMultiByte(status.charset?932:65001, NULL, tcMes[34], lstrlen(tcMes[34])+1, NULL, 0, NULL, NULL);
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

    // 計算 (試し割り法)
    for(ULONGLONG i=num[0]; i<=num[1] && cnt<num[2]; i++) { // 素数候補
        for(ULONGLONG j=2; j<=i && !isAborted; j++) { // 素因数候補
            if(i%j==0 && i!=j) break; // 素数でない
            if(i/j<j || i==j) { // 素数
                if(!status.onlycnt) { // 数え上げモードなら出力しない
                    if(cnt) { // 2つ目以降の追記
                        wsprintf(tcStr1, TEXT(", %I64u"), i); // 出力用
                        if(status.usefile) wsprintfA(mbStr, ", %I64u", i); // ファイル出力用 (ASCII 範囲なので変換不要)
                    } else { // 1つ目
                        wsprintf(tcStr1, TEXT("%I64u"), i);
                        if(status.usefile) wsprintfA(mbStr, "%I64u", i);
                    }
                    if(status.usefile) WriteFile(hFile, mbStr, lstrlenA(mbStr)*sizeof(CHAR), &dwTemp, NULL);
                    else OutputToEditbox(hEdi_out, tcStr1);
                }
                cnt++;
                break;
            }
            if(j!=2) j++; // 2以外ならもう1つ更に増やす(2以外の偶数のスキップ)
        }
        if(isAborted || i==ULLONG_MAX) break;
        if(i!=2) i++; // 2以外ならもう1つ更に増やす(2以外の偶数のスキップ)
    }

    // 最後の出力など
    if(!status.onlycnt && !status.usefile) OutputToEditbox(hEdi_out, TEXT("\r\n"));
    else if(!status.onlycnt && status.usefile) WriteFile(hFile, "\r\n", 2*sizeof(CHAR), &dwTemp, NULL);
    wsprintf(tcStr2, tcMes[isAborted?42:41], cnt, num[0], num[1], num[2]); // 結果文字列の生成
    OutputToEditbox(hEdi_out, tcStr2); // 結果出力
    SendMessage(hEdi_out, EM_REPLACESEL, 0, (WPARAM)TEXT("\r\n")); // キャレットが末尾にあることが確実ならこれだけで良い
    if(!status.onlycnt && status.usefile) {
#ifdef UNICODE
        mblen = WideCharToMultiByte(status.charset?932:65001, NULL, tcStr2, lstrlen(tcStr2), NULL, 0, NULL, NULL);
        WideCharToMultiByte(status.charset?932:65001, NULL, tcStr2, lstrlen(tcStr2), mbStr, mblen, NULL, NULL);
        WriteFile(hFile, mbStr, mblen*sizeof(CHAR), &dwTemp, NULL);
#else
        WriteFile(hfile, tcStr2, lstrlenA(tcStr2)*sizeof(CHAR), &dwtemp, NULL);
#endif
        CloseHandle(hFile);
    }
    if(isAborted) {
        PostMessage(hwnd, APP_THREADEND, 0, 0);
        return IDE_ABORT;
    }
    wsprintf(tcStr1, TEXT(" - %s"), tcMes[0]); // ウィンドウタイトル後半を構築
    lstrcat(tcStr2, tcStr1); // 処理結果に結合
    SetWindowText(hwnd, tcStr2); // ウィンドウタイトル更新

    PostMessage(hwnd, APP_THREADEND, 0, 0);
    return IDE_SUCCESS;
}