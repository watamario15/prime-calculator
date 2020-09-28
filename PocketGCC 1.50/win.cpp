#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <tchar.h>

#define TARGET_CPU TEXT("ARMv4I")
#define COMPILER_NAME TEXT("PocketGCC 1.50")
#define TARGET_PLATFORM TEXT("WinCE")

#define WND_CLASS_NAME TEXT("Prime_Factorization_Main")
#define TIMER_AWAIT 250
#define NUMOFSTRINGTABLE 47 // Number of strings in the String Table
#define APP_SETFOCUS WM_APP // Between WM_APP and 0xBFFF can be used freely

HWND hwnd, hbtn_ok, hbtn_abort, hbtn_clr, hbtn_0, hbtn_1, hbtn_2, hbtn_3, hbtn_4, hbtn_5, hbtn_6, hbtn_7, hbtn_8, hbtn_9, hbtn_CE, hbtn_BS, hedi0, hedi1, hedi2, hedi_out, hCmdBar, hwnd_temp, hwnd_focused;
HINSTANCE hInst; // Backup of Instance Handle
HMENU hmenu;
MENUITEMINFO mii;
HACCEL hAccel;
OPENFILENAME ofn;
DWORD dThreadID, dwtemp;
HANDLE hThread, hFile;
HDC hdc, hMemDC;
HFONT hMesFont, hFbtn, hFedi, hFnote; // Fonts to create
LOGFONT rLogfont; // Structure for fonts
HBRUSH hBrush, hBshSys; // Brushes to create
HPEN hPen, hPenSys; // Pens to create
PAINTSTRUCT ps;
RECT rect;
HBITMAP hBitmap;
INT r=0, g=255, b=255, scrx=0, scry=0, editlen=0, btnsize[4], nbpos[2], CmdBar_Height, StoppingTimer=0;
UINT menu[5];
LONGLONG num[3]; // Receive entered numbers
bool aborted=false, working=false, onlycnt=false, usefile=false, mode=false, overwrite=false;
TCHAR tctemp[1024]/*Concatnate and receive*/, tcmes[NUMOFSTRINGTABLE][4096]/*Store sentences*/, tcFile[MAX_PATH], tcEdit[65540];
CHAR cEdit[65540];
FARPROC wpedi0_old, wpedi1_old, wpedi2_old; // Store addresses of default Window Procedures

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK Edit0WindowProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK Edit1WindowProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK Edit2WindowProc(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI PrimeFactorization(LPVOID);
DWORD WINAPI ListPrimeNumbers(LPVOID);
void Paint();
void ResizeMoveControls();
void OutputToEditbox(HWND hWnd, LPCTSTR arg){ // Add a string to a edit control
    INT EditLen = (INT)SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0);
    SendMessage(hWnd, EM_SETSEL, EditLen, EditLen);
    SendMessage(hWnd, EM_REPLACESEL, 0, (WPARAM)arg);
    return;
}
void FinalizeErrorLPN(){
    TCHAR tcstr[1024];
    working = false;
    EnableWindow(hbtn_ok, TRUE);
    EnableWindow(hedi0, TRUE);
    EnableWindow(hedi1, TRUE);
    if(!onlycnt) EnableWindow(hedi2, TRUE);
    EnableWindow(hbtn_abort, FALSE);
    EnableMenuItem(hmenu, 2, MF_BYPOSITION | MF_ENABLED); // Re-enable "option" menu
    CommandBar_DrawMenuBar(hCmdBar, 1); // Redraw the menu
    wsprintf(tcstr, TEXT("%s - ") TARGET_CPU, tcmes[0]);
    SetWindowText(hwnd, tcstr);
    SendMessage(hwnd, APP_SETFOCUS, 0, 0);
    aborted = false;
    StoppingTimer--;
    if(!StoppingTimer) SetTimer(hwnd, 1, TIMER_AWAIT, NULL);
    return;
}

// Changed the 3rd argumant from LPSTR to LPTSTR (due to Windows CE's proprietary specification)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd){
    hInst = hInstance;
    
    WNDCLASS wcl; // WNDCLASSEX is not supported on Windows CE
    wcl.hInstance = hInstance;
    wcl.lpszClassName = WND_CLASS_NAME;
    wcl.lpfnWndProc = WindowProc;
    wcl.style = 0;
    wcl.hIcon = LoadIcon(hInstance, TEXT("Res_Icon")); // Load the icon
    wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcl.lpszMenuName = 0;
    wcl.cbClsExtra = 0;
    wcl.cbWndExtra = 0;
    wcl.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    if(!RegisterClass(&wcl)) return FALSE;

    // Load English resources
    for(int i=0; i<NUMOFSTRINGTABLE; i++) LoadString(hInstance, i+1100, tcmes[i], sizeof(tcmes[0])/sizeof(tcmes[0][0]));
    
    // Generate the default title string
    wsprintf(tctemp, TEXT("%s - ") TARGET_CPU, tcmes[0]);

    hwnd = CreateWindowEx( // It seems CreateWindowEx is supported on Windows CE.
        0, 
        WND_CLASS_NAME,
        tctemp,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPCHILDREN, // WS_CLIPCHILDREN disables drawing on areas of child windows.
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        480,
        320,
        NULL,
        NULL,
        hInstance,
        NULL);

    // Proper sizes for controls will be decided in WM_SIZE.
    hbtn_ok = CreateWindowEx( // OK button
        0,
        TEXT("BUTTON"),
        tcmes[4],
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 
        0,
        0,
        0,
        hwnd,
        (HMENU)0,
        hInstance,
        NULL);

    hbtn_abort = CreateWindowEx( // Abort button
        0,
        TEXT("BUTTON"),
        tcmes[5],
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_DISABLED,
        0,
        0,
        0,
        0,
        hwnd,
        (HMENU)1,
        hInstance,
        NULL);

    hbtn_clr = CreateWindowEx( // Clear History button
        0,
        TEXT("BUTTON"),
        tcmes[6],
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0,
        0,
        0,
        0,
        hwnd,
        (HMENU)2,
        hInstance,
        NULL);
    
    hbtn_0 = CreateWindowEx( // 0
        0,
        TEXT("BUTTON"),
        TEXT("0"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0,
        0,
        0,
        0,
        hwnd,
        (HMENU)10,
        hInstance,
        NULL);

    hbtn_1 = CreateWindowEx( // 1
        0,
        TEXT("BUTTON"),
        TEXT("1"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0,
        0,
        0,
        0,
        hwnd,
        (HMENU)11,
        hInstance,
        NULL);

    hbtn_2 = CreateWindowEx( // 2
        0,
        TEXT("BUTTON"),
        TEXT("2"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0,
        0,
        0,
        0,
        hwnd,
        (HMENU)12,
        hInstance,
        NULL);

    hbtn_3 = CreateWindowEx( // 3
        0,
        TEXT("BUTTON"),
        TEXT("3"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0,
        0,
        0,
        0,
        hwnd,
        (HMENU)13,
        hInstance,
        NULL);

    hbtn_4 = CreateWindowEx( // 4
        0,
        TEXT("BUTTON"),
        TEXT("4"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0,
        0,
        0,
        0,
        hwnd,
        (HMENU)14,
        hInstance,
        NULL);

    hbtn_5 = CreateWindowEx( // 5
        0,
        TEXT("BUTTON"),
        TEXT("5"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0,
        0,
        0,
        0,
        hwnd,
        (HMENU)15,
        hInstance,
        NULL);

    hbtn_6 = CreateWindowEx( // 6
        0,
        TEXT("BUTTON"),
        TEXT("6"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0,
        0,
        0,
        0,
        hwnd,
        (HMENU)16,
        hInstance,
        NULL);

    hbtn_7 = CreateWindowEx( // 7
        0,
        TEXT("BUTTON"),
        TEXT("7"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0,
        0,
        0,
        0,
        hwnd,
        (HMENU)17,
        hInstance,
        NULL);

    hbtn_8 = CreateWindowEx( // 8
        0,
        TEXT("BUTTON"),
        TEXT("8"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0,
        0,
        0,
        0,
        hwnd,
        (HMENU)18,
        hInstance,
        NULL);

    hbtn_9 = CreateWindowEx( // 9
        0,
        TEXT("BUTTON"),
        TEXT("9"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0,
        0,
        0,
        0,
        hwnd,
        (HMENU)19,
        hInstance,
        NULL);

    hbtn_BS = CreateWindowEx( // Back Space
        0,
        TEXT("BUTTON"),
        tcmes[44],
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0,
        0,
        0,
        0,
        hwnd,
        (HMENU)20,
        hInstance,
        NULL);

    hbtn_CE = CreateWindowEx( // Clear Entry
        0,
        TEXT("BUTTON"),
        tcmes[45],
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0,
        0,
        0,
        0,
        hwnd,
        (HMENU)21,
        hInstance,
        NULL);
        
    if(!hwnd) return FALSE;
    
    ShowWindow(hwnd, nShowCmd);
    ShowWindow(hwnd, SW_MAXIMIZE);
    UpdateWindow(hwnd);
    hAccel = LoadAccelerators(hInstance, TEXT("Res_Accel"));

    MSG msg;
    BOOL bRet;
    SetTimer(hwnd, 1, TIMER_AWAIT, NULL);

    while( (bRet=GetMessage(&msg, hwnd, 0, 0)) ){ // Continue unless the message is WM_QUIT(=0)
        if(bRet==-1) break; // Break when an error occurs
        else if(!TranslateAccelerator(hwnd, hAccel, &msg)){
            TranslateMessage(&msg);
            DispatchMessage(&msg); 
        }
    }
    return (int)msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    switch(uMsg){
        case WM_CREATE: // Called at first
            // Initialize the menu
            hCmdBar = CommandBar_Create(hInst, hWnd, 1);
            CommandBar_InsertMenubarEx(hCmdBar, hInst, TEXT("Res_EnglishMenu"), 0);
            CommandBar_Show(hCmdBar, TRUE);
            CmdBar_Height = CommandBar_Height(hCmdBar);
            hmenu = CommandBar_GetMenu(hCmdBar, 0);
            CheckMenuRadioItem(hmenu, 2051, 2052, 2051, MF_BYCOMMAND);
            CheckMenuRadioItem(hmenu, 2070, 2071, 2071, MF_BYCOMMAND);
            EnableMenuItem(hmenu, 2060, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(hmenu, 2061, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(hmenu, 2062, MF_BYCOMMAND | MF_GRAYED);

            hMemDC = CreateCompatibleDC(NULL);
            hBshSys = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
            hPenSys = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
            
            hedi0 = CreateWindowEx( // Input box
                0,
                TEXT("EDIT"),
                NULL,
                WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER,
                0,
                0,
                0,
                0,
                hWnd,
                (HMENU)50,
                ((LPCREATESTRUCT)(lParam))->hInstance,
                NULL);
            SendMessage(hedi0, EM_SETLIMITTEXT, (WPARAM)19, 0); // Limit of the number of digits for signed 64bits integer
            wpedi0_old = (FARPROC)SetWindowLong(hedi0, GWL_WNDPROC, (DWORD)Edit0WindowProc);
            SetFocus(hedi0); hwnd_focused = hedi0;
            
            hedi_out = CreateWindowEx( // Results box
                0,
                TEXT("EDIT"),
                NULL,
                WS_CHILD | WS_VISIBLE | ES_READONLY | ES_LEFT | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
                0,
                0,
                0,
                0,
                hWnd,
                (HMENU)60,
                ((LPCREATESTRUCT)(lParam))->hInstance,
                NULL);
            SendMessage(hedi_out, EM_SETLIMITTEXT, (WPARAM)65536, 0); // Maxmize the limit of the number of characters
            break;

        case WM_CLOSE: // Called when closing
            KillTimer(hWnd, 1); // Stop the timer
            StoppingTimer++;
            if(IDYES == MessageBox(hWnd, tcmes[7], tcmes[8], MB_YESNO | MB_ICONINFORMATION)){
                DestroyWindow(hWnd);
            }else{
                StoppingTimer--;
                if(!StoppingTimer) SetTimer(hWnd, 1, TIMER_AWAIT, NULL); // Restart the timer
                SetFocus(hwnd_focused); // Set focus on an edit control which had focus most recently
            }
            break;

        case WM_TIMER:
            if( (hwnd_temp=GetFocus()) && (hwnd_temp==hedi0 || hwnd_temp==hedi1 || hwnd_temp==hedi2 || hwnd_temp==hedi_out)) hwnd_focused = hwnd_temp;
            
            if(b<=0 && g<255) g+=8; //
            if(g>=255 && r>0) r-=8; //
            if(r<=0 && b<255) b+=8; //
            if(b>=255 && g>0) g-=8; //
            if(g<=0 && r<255) r+=8; //
            if(r>=255 && b>0) b-=8; //
                                    //
            if(r>255) r=255;        //
            if(r<0) r=0;            //
            if(g>255) g=255;        //
            if(g<0) g=0;            //
            if(b>255) b=255;        //
            if(b<0) b=0;            //

            DeleteObject(hBrush);
            DeleteObject(hPen);
            hBrush = CreateSolidBrush(RGB(r, g, b)); // Create a brush (Filler)
            hPen = CreatePen(PS_SOLID, 1, RGB(r, g, b)); // Create a pen (Frame)
            InvalidateRect(hWnd, NULL, FALSE);
            break;
                
        case WM_SIZE: // When the window size is changed
            ResizeMoveControls();
            break;

        case WM_PAINT:
            Paint();
            break;

        case WM_ACTIVATE:
            // When the main window is activated, set focus on an edit control which had focus most recently
            if(LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE) SetFocus(hwnd_focused);
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam)){
                case 0: // OK button
                    if(working) break;
                    working = true;
                    KillTimer(hWnd, 1);
                    StoppingTimer++;
                    EnableWindow(hbtn_ok, FALSE);
                    EnableWindow(hedi0, FALSE);
                    EnableWindow(hedi1, FALSE);
                    EnableWindow(hedi2, FALSE);
                    EnableWindow(hbtn_abort, TRUE);
                    EnableMenuItem(hmenu, 2, MF_BYPOSITION | MF_GRAYED); // Gray out the "options" menu
                    CommandBar_DrawMenuBar(hCmdBar, 1); // Redraw the "options" menu

                    SendMessage(hedi0, WM_GETTEXT, 31, (LPARAM)tctemp);
                    num[0] = _ttoi64(tctemp);
                    if(mode){
                        SendMessage(hedi1, WM_GETTEXT, 31, (LPARAM)tctemp);
                        num[1] = _ttoi64(tctemp);
                        SendMessage(hedi2, WM_GETTEXT, 31, (LPARAM)tctemp);
                        num[2] = _ttoi64(tctemp);
                    }
                    SetWindowText(hWnd, tcmes[9]);
                    InvalidateRect(hWnd, NULL, FALSE);

                    if(!mode) hThread = CreateThread(NULL, 0, PrimeFactorization, NULL, 0, &dThreadID);
                    else hThread = CreateThread(NULL, 0, ListPrimeNumbers, NULL, 0, &dThreadID);
                    SetThreadPriority(hThread, THREAD_PRIORITY_BELOW_NORMAL);
                    break;

                case 1: // Abort button
                    aborted = true;
                    break;

                case 2: // Clear the history
                    editlen = (INT)SendMessage(hedi_out, WM_GETTEXTLENGTH, 0, 0);
                    SendMessage(hedi_out, EM_SETSEL, 0, editlen);
                    SendMessage(hedi_out, EM_REPLACESEL, 0, (WPARAM)TEXT(""));
                    break;

                case 2001: // Save to a text file
                    tcFile[0] = L'\0'; tcEdit[0] = L'\0';
                    ZeroMemory(&ofn, sizeof(OPENFILENAME));
                    ofn.lStructSize = sizeof(OPENFILENAME);
                    ofn.hwndOwner = hWnd;
                    ofn.lpstrFilter = TEXT("ANSI TEXT (*.txt)\0*.txt\0")
                                      TEXT("All files (*.*)\0*.*\0");
                    ofn.lpstrFile = tcFile;
                    ofn.nMaxFile = MAX_PATH;
                    ofn.lpstrDefExt = TEXT(".txt");
                    ofn.lpstrTitle = tcmes[10];
                    ofn.Flags = OFN_OVERWRITEPROMPT;
                    if(!GetSaveFileName(&ofn)) break;

                    hFile = CreateFile(
                        tcFile,
                        GENERIC_WRITE,
                        FILE_SHARE_READ, // Allow other softwares to just "read" the file while processing
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
                    if(hFile == INVALID_HANDLE_VALUE){
                        MessageBox(hWnd, tcmes[11], tcmes[12], MB_OK | MB_ICONWARNING);
                        break;
                    }

                    GetWindowText(hedi_out, tcEdit, (editlen=GetWindowTextLength(hedi_out))+1);
                    WideCharToMultiByte(932, NULL, tcEdit, editlen+1, cEdit,
                        WideCharToMultiByte(932, NULL, tcEdit, editlen+1, NULL, 0, NULL, NULL), // Call twice to get the length of the converted text
                        NULL, NULL);
                    if(WriteFile(hFile, cEdit, strlen(cEdit)*sizeof(CHAR), &dwtemp, NULL)){
                        MessageBox(hWnd, tcmes[13], tcmes[14], MB_OK | MB_ICONINFORMATION);
                    }else{
                        MessageBox(hWnd, tcmes[15], tcmes[12], MB_OK | MB_ICONWARNING);
                    }
                    CloseHandle(hFile);
                    break;

                case 2009: // Exit menu in the menubar
                    SendMessage(hWnd, WM_CLOSE, 0, 0);
                    break;

                case 2021: // Cut
                    SendMessage(hwnd_focused, WM_CUT, 0, 0);
                    break;

                case 2022: // Copy
                    SendMessage(hwnd_focused, WM_COPY, 0, 0);
                    break;

                case 2023: // Paste
                    SendMessage(hwnd_focused, WM_PASTE, 0, 0);
                    break;

                case 2024: // Select All
                    editlen = (INT)SendMessage(hwnd_focused, WM_GETTEXTLENGTH, 0, 0);
                    SendMessage(hwnd_focused, EM_SETSEL, 0, editlen);
                    break;

                case 20: // Partially delete
                    editlen = SendMessage(hwnd_focused, EM_GETSEL, 0, 0);
                    if(LOWORD(editlen)==HIWORD(editlen)){
                        SendMessage(hwnd_focused, EM_SETSEL, LOWORD(editlen)-1, LOWORD(editlen));
                        SendMessage(hwnd_focused, EM_REPLACESEL, 0, (WPARAM)TEXT(""));
                    }else SendMessage(hwnd_focused, EM_REPLACESEL, 0, (WPARAM)TEXT(""));
                    break;

                case 21: // Clear Entry
                    editlen = SendMessage(hwnd_focused, WM_GETTEXTLENGTH, 0, 0);
                    SendMessage(hwnd_focused, EM_SETSEL, 0, editlen);
                    SendMessage(hwnd_focused, EM_REPLACESEL, 0, (WPARAM)TEXT(""));
                    break;

                case 2051: // "Prime Factorization" menu in the menubar
                    if(!mode) break;
                    CheckMenuRadioItem(hmenu, 2051, 2052, 2051, MF_BYCOMMAND);
                    EnableMenuItem(hmenu, 2060, MF_BYCOMMAND | MF_GRAYED);
                    EnableMenuItem(hmenu, 2061, MF_BYCOMMAND | MF_GRAYED);
                    EnableMenuItem(hmenu, 2062, MF_BYCOMMAND | MF_GRAYED);
                    SetFocus(hedi0);
                    DestroyWindow(hedi1); // Delete the edit control added in mode 1
                    DestroyWindow(hedi2); // Delete the edit control added in mode 1
                    mode = 0;
                    SendMessage(hWnd, WM_SIZE, 0, 0);
                    break;

                case 2052: // "List/Count Prime Numbers" menu in the menubar
                    if(mode) break;
                    CheckMenuRadioItem(hmenu, 2051, 2052, 2052, MF_BYCOMMAND);
                    EnableMenuItem(hmenu, 2060, MF_BYCOMMAND | MF_ENABLED);
                    EnableMenuItem(hmenu, 2061, MF_BYCOMMAND | MF_ENABLED);
                    EnableMenuItem(hmenu, 2062, MF_BYCOMMAND | MF_ENABLED);
                    mode = 1;
                    hedi1 = CreateWindowEx( // Input box
                        0,
                        TEXT("EDIT"),
                        NULL,
                        WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
                        0,
                        0,
                        0,
                        0,
                        hWnd,
                        (HMENU)51,
                        hInst,
                        NULL);
                    SendMessage(hedi1, EM_SETLIMITTEXT, (WPARAM)19, 0);
                    wpedi1_old = (FARPROC)SetWindowLong(hedi1, GWL_WNDPROC, (DWORD)Edit1WindowProc); // Change the Window Procedure

                    hedi2 = CreateWindowEx( // Input box
                        0,
                        TEXT("EDIT"),
                        NULL,
                        WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
                        0,
                        0,
                        0,
                        0,
                        hWnd,
                        (HMENU)52,
                        hInst,
                        NULL);
                    SendMessage(hedi2, EM_SETLIMITTEXT, (WPARAM)19, 0);
                    SendMessage(hWnd, WM_SIZE, 0, 0);
                    wpedi2_old = (FARPROC)SetWindowLong(hedi2, GWL_WNDPROC, (DWORD)Edit2WindowProc); // Change the Window Procedure
                    break;

                case 2060: // "Display only number of prime numbers" menu in the menubar
                    ZeroMemory(&mii, sizeof(mii));
                    mii.fMask = MIIM_STATE;
                    mii.cbSize = sizeof(MENUITEMINFO);
                    GetMenuItemInfo(hmenu, 2060, FALSE, &mii);
                    if(mii.fState & MF_CHECKED){ // In case the menu item is already checked
                        CheckMenuItem(hmenu, 2060, MF_BYCOMMAND | MF_UNCHECKED);
                        EnableMenuItem(hmenu, 2061, MF_BYCOMMAND | MF_ENABLED);
                        EnableWindow(hedi2, TRUE);
                        onlycnt = false;
                    }else{
                        CheckMenuItem(hmenu, 2060, MF_BYCOMMAND | MF_CHECKED);
                        EnableMenuItem(hmenu, 2061, MF_BYCOMMAND | MF_GRAYED);
                        EnableWindow(hedi2, FALSE);
                        onlycnt = true;
                    }
                    break;

                case 2061: // "Output to a text file" menu in the menubar
                    ZeroMemory(&mii, sizeof(mii));
                    mii.fMask = MIIM_STATE;
                    mii.cbSize = sizeof(MENUITEMINFO);
                    GetMenuItemInfo(hmenu, 2061, FALSE, &mii);
                    if(mii.fState & MF_CHECKED){
                        CheckMenuItem(hmenu, 2061, MF_BYCOMMAND | MF_UNCHECKED);
                        usefile = false;
                    }else{
                        CheckMenuItem(hmenu, 2061, MF_BYCOMMAND | MF_CHECKED);
                        usefile = true;
                    }
                    break;

                case 2062: // "Overwrite a existing file" menu in the menubar
                    ZeroMemory(&mii, sizeof(mii));
                    mii.fMask = MIIM_STATE;
                    mii.cbSize = sizeof(MENUITEMINFO);
                    GetMenuItemInfo(hmenu, 2062, FALSE, &mii);
                    if(mii.fState & MF_CHECKED){
                        CheckMenuItem(hmenu, 2062, MF_BYCOMMAND | MF_UNCHECKED);
                        overwrite = false;
                    }else{
                        CheckMenuItem(hmenu, 2062, MF_BYCOMMAND | MF_CHECKED);
                        overwrite = true;
                    }
                    break;

                case 2070: // Set to Japanese
                    // Backup the state
                    ZeroMemory(&mii, sizeof(mii));
                    mii.fMask = MIIM_STATE;
                    mii.cbSize = sizeof(MENUITEMINFO);
                    GetMenuItemInfo(hmenu, 2051, FALSE, &mii);
                    menu[0] = mii.fState;
                    ZeroMemory(&mii, sizeof(mii));
                    mii.fMask = MIIM_STATE;
                    mii.cbSize = sizeof(MENUITEMINFO);
                    GetMenuItemInfo(hmenu, 2052, FALSE, &mii);
                    menu[1] = mii.fState;
                    ZeroMemory(&mii, sizeof(mii));
                    mii.fMask = MIIM_STATE;
                    mii.cbSize = sizeof(MENUITEMINFO);
                    GetMenuItemInfo(hmenu, 2060, FALSE, &mii);
                    menu[2] = mii.fState;
                    ZeroMemory(&mii, sizeof(mii));
                    mii.fMask = MIIM_STATE;
                    mii.cbSize = sizeof(MENUITEMINFO);
                    GetMenuItemInfo(hmenu, 2061, FALSE, &mii);
                    menu[3] = mii.fState;
                    ZeroMemory(&mii, sizeof(mii));
                    mii.fMask = MIIM_STATE;
                    mii.cbSize = sizeof(MENUITEMINFO);
                    GetMenuItemInfo(hmenu, 2062, FALSE, &mii);
                    menu[4] = mii.fState;

                    DestroyMenu(hmenu);
                    CommandBar_Destroy(hCmdBar);
                    hCmdBar = CommandBar_Create(hInst, hWnd, 1);
                    CommandBar_InsertMenubarEx(hCmdBar, hInst, TEXT("Res_JapaneseMenu"), 0); // Load the Japanese menu
                    CommandBar_Show(hCmdBar, TRUE);
                    CmdBar_Height = CommandBar_Height(hCmdBar);
                    hmenu = CommandBar_GetMenu(hCmdBar, 0);
                    CheckMenuRadioItem(hmenu, 2070, 2071, 2070, MF_BYCOMMAND); // Check "Japanese"

                    // Restore Checked/Unchecked
                    if(menu[0] & MF_CHECKED) CheckMenuRadioItem(hmenu, 2051, 2052, 2051, MF_BYCOMMAND);
                    else CheckMenuRadioItem(hmenu, 2051, 2052, 2052, MF_BYCOMMAND);
                    if(menu[2] & MF_CHECKED) CheckMenuItem(hmenu, 2060, MF_BYCOMMAND | MF_CHECKED);
                    if(menu[3] & MF_CHECKED) CheckMenuItem(hmenu, 2061, MF_BYCOMMAND | MF_CHECKED);
                    if(menu[4] & MF_CHECKED) CheckMenuItem(hmenu, 2062, MF_BYCOMMAND | MF_CHECKED);

                    // Restore Enabled/Grayed
                    if(menu[2] & MF_GRAYED) EnableMenuItem(hmenu, 2060, MF_BYCOMMAND | MF_GRAYED);
                    if(menu[3] & MF_GRAYED) EnableMenuItem(hmenu, 2061, MF_BYCOMMAND | MF_GRAYED);
                    if(menu[4] & MF_GRAYED) EnableMenuItem(hmenu, 2062, MF_BYCOMMAND | MF_GRAYED);

                    {for(int i=0; i<NUMOFSTRINGTABLE; i++) LoadString(hInst, i+1000, tcmes[i], sizeof(tcmes[0])/sizeof(tcmes[0][0]));}
                    SetWindowText(hbtn_ok, tcmes[4]);
                    SetWindowText(hbtn_abort, tcmes[5]);
                    SetWindowText(hbtn_clr, tcmes[6]);
                    SetWindowText(hbtn_BS, tcmes[44]);
                    SetWindowText(hbtn_CE, tcmes[45]);
                    wsprintf(tctemp, TEXT("%s - ") TARGET_CPU, tcmes[0]);
                    SetWindowText(hwnd, tctemp);
                    break;

                case 2071: // Set to English
                    // Backup the state
                    ZeroMemory(&mii, sizeof(mii));
                    mii.fMask = MIIM_STATE;
                    mii.cbSize = sizeof(MENUITEMINFO);
                    GetMenuItemInfo(hmenu, 2051, FALSE, &mii);
                    menu[0] = mii.fState;
                    ZeroMemory(&mii, sizeof(mii));
                    mii.fMask = MIIM_STATE;
                    mii.cbSize = sizeof(MENUITEMINFO);
                    GetMenuItemInfo(hmenu, 2052, FALSE, &mii);
                    menu[1] = mii.fState;
                    ZeroMemory(&mii, sizeof(mii));
                    mii.fMask = MIIM_STATE;
                    mii.cbSize = sizeof(MENUITEMINFO);
                    GetMenuItemInfo(hmenu, 2060, FALSE, &mii);
                    menu[2] = mii.fState;
                    ZeroMemory(&mii, sizeof(mii));
                    mii.fMask = MIIM_STATE;
                    mii.cbSize = sizeof(MENUITEMINFO);
                    GetMenuItemInfo(hmenu, 2061, FALSE, &mii);
                    menu[3] = mii.fState;
                    ZeroMemory(&mii, sizeof(mii));
                    mii.fMask = MIIM_STATE;
                    mii.cbSize = sizeof(MENUITEMINFO);
                    GetMenuItemInfo(hmenu, 2062, FALSE, &mii);
                    menu[4] = mii.fState;

                    DestroyMenu(hmenu);
                    CommandBar_Destroy(hCmdBar);
                    hCmdBar = CommandBar_Create(hInst, hWnd, 1);
                    CommandBar_InsertMenubarEx(hCmdBar, hInst, TEXT("Res_EnglishMenu"), 0); // Load the English menu
                    CommandBar_Show(hCmdBar, TRUE);
                    CmdBar_Height = CommandBar_Height(hCmdBar);
                    hmenu = CommandBar_GetMenu(hCmdBar, 0);
                    CheckMenuRadioItem(hmenu, 2070, 2071, 2071, MF_BYCOMMAND); // Check "English"

                    // Restore Checked/Unchecked
                    if(menu[0] & MF_CHECKED) CheckMenuRadioItem(hmenu, 2051, 2052, 2051, MF_BYCOMMAND);
                    else CheckMenuRadioItem(hmenu, 2051, 2052, 2052, MF_BYCOMMAND);
                    if(menu[2] & MF_CHECKED) CheckMenuItem(hmenu, 2060, MF_BYCOMMAND | MF_CHECKED);
                    if(menu[3] & MF_CHECKED) CheckMenuItem(hmenu, 2061, MF_BYCOMMAND | MF_CHECKED);
                    if(menu[4] & MF_CHECKED) CheckMenuItem(hmenu, 2062, MF_BYCOMMAND | MF_CHECKED);

                    // Restore Enabled/Grayed
                    if(menu[2] & MF_GRAYED) EnableMenuItem(hmenu, 2060, MF_BYCOMMAND | MF_GRAYED);
                    if(menu[3] & MF_GRAYED) EnableMenuItem(hmenu, 2061, MF_BYCOMMAND | MF_GRAYED);
                    if(menu[4] & MF_GRAYED) EnableMenuItem(hmenu, 2062, MF_BYCOMMAND | MF_GRAYED);

                    {for(int i=0; i<NUMOFSTRINGTABLE; i++) LoadString(hInst, i+1100, tcmes[i], sizeof(tcmes[0])/sizeof(tcmes[0][0]));}
                    SetWindowText(hbtn_ok, tcmes[4]);
                    SetWindowText(hbtn_abort, tcmes[5]);
                    SetWindowText(hbtn_clr, tcmes[6]);
                    SetWindowText(hbtn_BS, tcmes[44]);
                    SetWindowText(hbtn_CE, tcmes[45]);
                    wsprintf(tctemp, TEXT("%s - ") TARGET_CPU, tcmes[0]);
                    SetWindowText(hwnd, tctemp);
                    break;

                case 2101: // "How to use" menu in the menubar
                    KillTimer(hWnd, 1);
                    StoppingTimer++;
                    MessageBox(hWnd, tcmes[(mode ? 3 : 2)], tcmes[16], MB_OK | MB_ICONINFORMATION);
                    StoppingTimer--;
                    if(!StoppingTimer) SetTimer(hWnd, 1, TIMER_AWAIT, NULL);
                    break;

                case 2109: // "About" menu in the menubar
                    KillTimer(hWnd, 1);
                    StoppingTimer++;
                    wsprintf(tctemp, TEXT("%s")
                        TEXT("%s") COMPILER_NAME TEXT("\n")
                        TEXT("%s") TARGET_PLATFORM TEXT(" Application\n")
                        TEXT("%s") TARGET_CPU TEXT("\n")
                        TEXT("%s") TEXT(__DATE__) TEXT(" ") TEXT(__TIME__)
                        TEXT("\n(C) 2018-2020 watamario"),
                        tcmes[1], tcmes[40], tcmes[41], tcmes[42], tcmes[43]);
                    MessageBox(hWnd, tctemp, tcmes[17], MB_OK | MB_ICONINFORMATION);
                    StoppingTimer--;
                    if(!StoppingTimer) SetTimer(hWnd, 1, TIMER_AWAIT, NULL);
                    break;
            }
            if(LOWORD(wParam)>=10 && LOWORD(wParam)<20){ // Screen keyboard
                _itow(LOWORD(wParam)-10, tctemp, 10);
                SendMessage(hwnd_focused, EM_REPLACESEL, 0, (WPARAM)tctemp);
            }
            if(LOWORD(wParam)<50) SetFocus(hwnd_focused); // Set focus on an edit control if the clicked control isn't an edit control
            else return DefWindowProc(hWnd, uMsg, wParam, lParam);
            break;

        case APP_SETFOCUS:
            SetFocus(hwnd_focused);
            break;
        
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
                
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

// Custom window procedure for the input box
LRESULT CALLBACK Edit0WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    switch(uMsg){
        case WM_DESTROY:
            SetWindowLong(hWnd, GWL_WNDPROC, (DWORD)wpedi0_old); // Set back to the default Window Procedure
            break;

        // Run or move to the next edit control by the Enter key. If it isn't a needed one, let it go to the default. (Don't put other messages!!)
        case WM_CHAR:
            switch((CHAR)wParam){
                case VK_RETURN:
                    if(!working && !mode){
                        working = true;
                        KillTimer(hwnd, 1);
                        StoppingTimer++;
                        EnableWindow(hbtn_ok, FALSE);
                        EnableWindow(hedi0, FALSE);
                        EnableWindow(hedi1, FALSE);
                        EnableWindow(hedi2, FALSE);
                        EnableWindow(hbtn_abort, TRUE);
                        EnableMenuItem(hmenu, 2, MF_BYPOSITION | MF_GRAYED); // Gray out the "options" menu
                        CommandBar_DrawMenuBar(hCmdBar, 1); // Redraw the "options" menu

                        SendMessage(hedi0, WM_GETTEXT, 31, (LPARAM)tctemp);
                        num[0] = _ttoi64(tctemp);
                        if(mode){
                            SendMessage(hedi1, WM_GETTEXT, 31, (LPARAM)tctemp);
                            num[1] = _ttoi64(tctemp);
                            SendMessage(hedi2, WM_GETTEXT, 31, (LPARAM)tctemp);
                            num[2] = _ttoi64(tctemp);
                        }
                        SetWindowText(hwnd, tcmes[9]);

                        if(!mode) hThread = CreateThread(NULL, 0, PrimeFactorization, NULL, 0, &dThreadID);
                        else hThread = CreateThread(NULL, 0, ListPrimeNumbers, NULL, 0, &dThreadID);
                        SetThreadPriority(hThread, THREAD_PRIORITY_BELOW_NORMAL);
                    }else if(mode) SetFocus(hedi1);
                    return 0;
                case 'q':
                    SendMessage(hedi0, EM_REPLACESEL, 0, (WPARAM)TEXT("1"));
                    return 0;
                case 'w':
                    SendMessage(hedi0, EM_REPLACESEL, 0, (WPARAM)TEXT("2"));
                    return 0;
                case 'e':
                    SendMessage(hedi0, EM_REPLACESEL, 0, (WPARAM)TEXT("3"));
                    return 0;
                case 'r':
                    SendMessage(hedi0, EM_REPLACESEL, 0, (WPARAM)TEXT("4"));
                    return 0;
                case 't':
                    SendMessage(hedi0, EM_REPLACESEL, 0, (WPARAM)TEXT("5"));
                    return 0;
                case 'y':
                    SendMessage(hedi0, EM_REPLACESEL, 0, (WPARAM)TEXT("6"));
                    return 0;
                case 'u':
                    SendMessage(hedi0, EM_REPLACESEL, 0, (WPARAM)TEXT("7"));
                    return 0;
                case 'i':
                    SendMessage(hedi0, EM_REPLACESEL, 0, (WPARAM)TEXT("8"));
                    return 0;
                case 'o':
                    SendMessage(hedi0, EM_REPLACESEL, 0, (WPARAM)TEXT("9"));
                    return 0;
                case 'p':
                    SendMessage(hedi0, EM_REPLACESEL, 0, (WPARAM)TEXT("0"));
                    return 0;
            }
        default:
            return CallWindowProc(wpedi0_old, hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK Edit1WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    switch(uMsg){
        case WM_DESTROY:
            SetWindowLong(hWnd, GWL_WNDPROC, (DWORD)wpedi1_old);
            break;

        // Move to the next edit control by the Enter key. If it isn't a needed one, let it go to the default. (Don't put other messages!!)
        case WM_CHAR:
            switch((CHAR)wParam){
                case VK_RETURN:
                    SetFocus(hedi2);
                    return 0;
                case 'q':
                    SendMessage(hedi1, EM_REPLACESEL, 0, (WPARAM)TEXT("1"));
                    return 0;
                case 'w':
                    SendMessage(hedi1, EM_REPLACESEL, 0, (WPARAM)TEXT("2"));
                    return 0;
                case 'e':
                    SendMessage(hedi1, EM_REPLACESEL, 0, (WPARAM)TEXT("3"));
                    return 0;
                case 'r':
                    SendMessage(hedi1, EM_REPLACESEL, 0, (WPARAM)TEXT("4"));
                    return 0;
                case 't':
                    SendMessage(hedi1, EM_REPLACESEL, 0, (WPARAM)TEXT("5"));
                    return 0;
                case 'y':
                    SendMessage(hedi1, EM_REPLACESEL, 0, (WPARAM)TEXT("6"));
                    return 0;
                case 'u':
                    SendMessage(hedi1, EM_REPLACESEL, 0, (WPARAM)TEXT("7"));
                    return 0;
                case 'i':
                    SendMessage(hedi1, EM_REPLACESEL, 0, (WPARAM)TEXT("8"));
                    return 0;
                case 'o':
                    SendMessage(hedi1, EM_REPLACESEL, 0, (WPARAM)TEXT("9"));
                    return 0;
                case 'p':
                    SendMessage(hedi1, EM_REPLACESEL, 0, (WPARAM)TEXT("0"));
                    return 0;
            }
        default:
            return CallWindowProc(wpedi1_old, hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK Edit2WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    switch(uMsg){
        case WM_DESTROY:
            SetWindowLong(hWnd, GWL_WNDPROC, (DWORD)wpedi2_old);
            break;

        // Run by the Enter key. If it isn't a needed one, let it go to the default. (Don't put other messages!!)
        case WM_CHAR:
            switch((CHAR)wParam){
                case VK_RETURN:
                    if(!working){
                        working = true;
                        KillTimer(hwnd, 1);
                        StoppingTimer++;
                        EnableWindow(hbtn_ok, FALSE);
                        EnableWindow(hedi0, FALSE);
                        EnableWindow(hedi1, FALSE);
                        EnableWindow(hedi2, FALSE);
                        EnableWindow(hbtn_abort, TRUE);
                        EnableMenuItem(hmenu, 2, MF_BYPOSITION | MF_GRAYED);
                        CommandBar_DrawMenuBar(hCmdBar, 1);

                        SendMessage(hedi0, WM_GETTEXT, 31, (LPARAM)tctemp);
                        num[0] = _ttoi64(tctemp);
                        if(mode){
                            SendMessage(hedi1, WM_GETTEXT, 31, (LPARAM)tctemp);
                            num[1] = _ttoi64(tctemp);
                            SendMessage(hedi2, WM_GETTEXT, 31, (LPARAM)tctemp);
                            num[2] = _ttoi64(tctemp);
                        }
                        SetWindowText(hwnd, tcmes[9]);

                        if(!mode) hThread = CreateThread(NULL, 0, PrimeFactorization, NULL, 0, &dThreadID);
                        else hThread = CreateThread(NULL, 0, ListPrimeNumbers, NULL, 0, &dThreadID);
                        SetThreadPriority(hThread, THREAD_PRIORITY_BELOW_NORMAL);
                    }
                    return 0;
                case 'q':
                    SendMessage(hedi2, EM_REPLACESEL, 0, (WPARAM)TEXT("1"));
                    return 0;
                case 'w':
                    SendMessage(hedi2, EM_REPLACESEL, 0, (WPARAM)TEXT("2"));
                    return 0;
                case 'e':
                    SendMessage(hedi2, EM_REPLACESEL, 0, (WPARAM)TEXT("3"));
                    return 0;
                case 'r':
                    SendMessage(hedi2, EM_REPLACESEL, 0, (WPARAM)TEXT("4"));
                    return 0;
                case 't':
                    SendMessage(hedi2, EM_REPLACESEL, 0, (WPARAM)TEXT("5"));
                    return 0;
                case 'y':
                    SendMessage(hedi2, EM_REPLACESEL, 0, (WPARAM)TEXT("6"));
                    return 0;
                case 'u':
                    SendMessage(hedi2, EM_REPLACESEL, 0, (WPARAM)TEXT("7"));
                    return 0;
                case 'i':
                    SendMessage(hedi2, EM_REPLACESEL, 0, (WPARAM)TEXT("8"));
                    return 0;
                case 'o':
                    SendMessage(hedi2, EM_REPLACESEL, 0, (WPARAM)TEXT("9"));
                    return 0;
                case 'p':
                    SendMessage(hedi2, EM_REPLACESEL, 0, (WPARAM)TEXT("0"));
                    return 0;
            }
        default:
            return CallWindowProc(wpedi2_old, hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

void ResizeMoveControls(){
    GetClientRect(hwnd, &rect);
    scrx = rect.right; scry = rect.bottom-CmdBar_Height; // Subtract the Command Bar's width

    // Create a font for the main window
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
    _tcscpy(rLogfont.lfFaceName, TEXT("MS PGothic"));
    DeleteObject(hMesFont);
    hMesFont = CreateFontIndirect(&rLogfont); // Create a font
    
    // Create a font for buttons
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
    _tcscpy(rLogfont.lfFaceName, TEXT("MS PGothic"));
    DeleteObject(hFbtn);
    hFbtn = CreateFontIndirect(&rLogfont);

    // Create a font for notes
    if(15*scrx/700 < 15*scry/400) rLogfont.lfHeight = 15*scrx/700;
    else rLogfont.lfHeight = 15*scry/400;
    DeleteObject(hFnote);
    hFnote = CreateFontIndirect(&rLogfont);

    // Create a font for edit boxes
    if(16*scrx/700 < 16*scry/400) rLogfont.lfHeight = 16*scrx/700;
    else rLogfont.lfHeight = 16*scry/400;
    if(rLogfont.lfHeight<12) rLogfont.lfHeight=12;
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
    _tcscpy(rLogfont.lfFaceName, TEXT("MS Gothic"));
    DeleteObject(hFedi);
    hFedi = CreateFontIndirect(&rLogfont);

    // Apply newly created fonts to controls
    SendMessage(hbtn_ok, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
    SendMessage(hbtn_abort, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
    SendMessage(hbtn_clr, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
    SendMessage(hbtn_0, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
    SendMessage(hbtn_1, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
    SendMessage(hbtn_2, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
    SendMessage(hbtn_3, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
    SendMessage(hbtn_4, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
    SendMessage(hbtn_5, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
    SendMessage(hbtn_6, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
    SendMessage(hbtn_7, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
    SendMessage(hbtn_8, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
    SendMessage(hbtn_9, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
    SendMessage(hbtn_BS, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
    SendMessage(hbtn_CE, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
    SendMessage(hedi0, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
    SendMessage(hedi_out, WM_SETFONT, (WPARAM)hFedi, MAKELPARAM(FALSE, 0));
    if(mode){
        SendMessage(hedi1, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
        SendMessage(hedi2, WM_SETFONT, (WPARAM)hFbtn, MAKELPARAM(FALSE, 0));
    }

    // Move and resize controls
    MoveWindow(hCmdBar, NULL, NULL, NULL, NULL, TRUE);
    btnsize[0] = 64*scrx/700; btnsize[1] = 32*scry/400;
    btnsize[2] = scrx*9/120; btnsize[3] = 8*scry/100;
    if(!mode){
        MoveWindow(hedi0, btnsize[0], CmdBar_Height, btnsize[0]*4, btnsize[1], TRUE);
        MoveWindow(hbtn_ok, btnsize[0]*5, CmdBar_Height, btnsize[0], btnsize[1], TRUE);
        MoveWindow(hbtn_abort, btnsize[0]*6, CmdBar_Height, btnsize[0], btnsize[1], TRUE);
        MoveWindow(hbtn_clr, btnsize[0]*7, CmdBar_Height, btnsize[0]*5/2, btnsize[1], TRUE);
        MoveWindow(hedi_out, scrx/20, scry*9/40+CmdBar_Height+btnsize[3], scrx*9/10, scry*29/40-btnsize[3], TRUE);
        nbpos[0] = scrx/20; nbpos[1] = scry*9/40+CmdBar_Height;
    }else{
        MoveWindow(hedi0, btnsize[0], CmdBar_Height, btnsize[0]*4, btnsize[1], TRUE);
        MoveWindow(hedi1, btnsize[0]*6, CmdBar_Height, btnsize[0]*4, btnsize[1], TRUE);
        MoveWindow(hedi2, btnsize[0], btnsize[1]+CmdBar_Height, btnsize[0]*2, btnsize[1], TRUE);
        MoveWindow(hbtn_ok, btnsize[0]*3, btnsize[1]+CmdBar_Height, btnsize[0], btnsize[1], TRUE);
        MoveWindow(hbtn_abort, btnsize[0]*4, btnsize[1]+CmdBar_Height, btnsize[0], btnsize[1], TRUE);
        MoveWindow(hbtn_clr, btnsize[0]*5, btnsize[1]+CmdBar_Height, btnsize[0]*5/2, btnsize[1], TRUE);
        MoveWindow(hedi_out, scrx/20, scry*3/10+CmdBar_Height+btnsize[3], scrx*9/10, scry*13/20-btnsize[3], TRUE);
        nbpos[0] = scrx/20; nbpos[1] = scry*3/10+CmdBar_Height;
    }
    MoveWindow(hbtn_0, nbpos[0], nbpos[1], btnsize[2], btnsize[3], TRUE);
    nbpos[0]+=btnsize[2];
    MoveWindow(hbtn_1, nbpos[0], nbpos[1], btnsize[2], btnsize[3], TRUE);
    nbpos[0]+=btnsize[2];
    MoveWindow(hbtn_2, nbpos[0], nbpos[1], btnsize[2], btnsize[3], TRUE);
    nbpos[0]+=btnsize[2];
    MoveWindow(hbtn_3, nbpos[0], nbpos[1], btnsize[2], btnsize[3], TRUE);
    nbpos[0]+=btnsize[2];
    MoveWindow(hbtn_4, nbpos[0], nbpos[1], btnsize[2], btnsize[3], TRUE);
    nbpos[0]+=btnsize[2];
    MoveWindow(hbtn_5, nbpos[0], nbpos[1], btnsize[2], btnsize[3], TRUE);
    nbpos[0]+=btnsize[2];
    MoveWindow(hbtn_6, nbpos[0], nbpos[1], btnsize[2], btnsize[3], TRUE);
    nbpos[0]+=btnsize[2];
    MoveWindow(hbtn_7, nbpos[0], nbpos[1], btnsize[2], btnsize[3], TRUE);
    nbpos[0]+=btnsize[2];
    MoveWindow(hbtn_8, nbpos[0], nbpos[1], btnsize[2], btnsize[3], TRUE);
    nbpos[0]+=btnsize[2];
    MoveWindow(hbtn_9, nbpos[0], nbpos[1], btnsize[2], btnsize[3], TRUE);
    nbpos[0]+=btnsize[2];
    MoveWindow(hbtn_BS, nbpos[0], nbpos[1], btnsize[2], btnsize[3], TRUE);
    nbpos[0]+=btnsize[2];
    MoveWindow(hbtn_CE, nbpos[0], nbpos[1], btnsize[2], btnsize[3], TRUE);

    DeleteObject(hBitmap);
    hBitmap = CreateCompatibleBitmap(hdc=GetDC(hwnd), scrx, scry);
    ReleaseDC(hwnd, hdc);
    SelectObject(hMemDC, hBitmap); // Set the control with the new size to the memory device context
    InvalidateRect(hwnd, NULL, FALSE);
    return;
}

void Paint(){
    GetClientRect(hwnd, &rect);
    SelectObject(hMemDC, hPen); // Set the pen to the device context
    SelectObject(hMemDC, hBrush); // Set the brush to the device context
    Rectangle(hMemDC, rect.left, rect.top, rect.right, rect.bottom); // Draw a rectangle that fills the area

    if(!mode){
        SelectObject(hMemDC, hPenSys);
        SelectObject(hMemDC, hBshSys);
        Rectangle(hMemDC, 0, 0, btnsize[0], btnsize[1]);

        SetBkMode(hMemDC, TRANSPARENT);
        SetTextColor(hMemDC, RGB(0, 0, 0));
        SelectObject(hMemDC, hFnote); // Set the font to the device context
        rect.right=btnsize[0]; rect.bottom=btnsize[1];
        DrawText(hMemDC, tcmes[18], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }else{
        SelectObject(hMemDC, hPenSys);
        SelectObject(hMemDC, hBshSys);
        Rectangle(hMemDC, 0, 0, btnsize[0], btnsize[1]);
        Rectangle(hMemDC, btnsize[0]*5, 0, btnsize[0]*6, btnsize[1]);
        Rectangle(hMemDC, 0, btnsize[1], btnsize[0], btnsize[1]*2);

        SetBkMode(hMemDC, TRANSPARENT);
        SetTextColor(hMemDC, RGB(0, 0, 0));
        SelectObject(hMemDC, hFnote);
        rect.right=btnsize[0]; rect.bottom=btnsize[1];
        DrawText(hMemDC, tcmes[19], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        rect.left=btnsize[0]*5; rect.right=btnsize[0]*6;
        DrawText(hMemDC, tcmes[20], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        rect.left=0; rect.top=btnsize[1]; rect.bottom=btnsize[1]*2; rect.right=btnsize[0];
        DrawText(hMemDC, tcmes[21], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    SetBkMode(hMemDC, OPAQUE);
    SetBkColor(hMemDC, RGB(255, 255, 0));
    SetTextColor(hMemDC, RGB(0, 0, 255));
    SelectObject(hMemDC, hMesFont);
    rect.left=0; rect.right=scrx;
    if(!mode){
        rect.top = btnsize[1];
        rect.bottom = scry*9/40;
    }else{
        rect.top = btnsize[1]*2;
        rect.bottom = scry*3/10;
    }
    if(working) DrawText(hMemDC, tcmes[22], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    else if(!mode) DrawText(hMemDC, tcmes[23], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    else if(mode) DrawText(hMemDC, tcmes[24], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    rect.left=0; rect.right=scrx; rect.top=0; rect.bottom=scry;
    
    hdc = BeginPaint(hwnd, &ps);
    BitBlt(hdc, 0, CmdBar_Height, rect.right, rect.bottom, hMemDC, 0, 0, SRCCOPY);
    EndPaint(hwnd, &ps);
    return;
}

DWORD WINAPI PrimeFactorization(LPVOID lpParameter){
    LONGLONG N=num[0], cnt=0, i=2;
    bool chk=false;
    TCHAR tcresult[1024] = TEXT(""), tcstr[1024] = TEXT("");

    if(N<=0) { // In case the input was invalid
        wsprintf(tcstr, TEXT("%s - ") TARGET_CPU, tcmes[0]);
        SetWindowText(hwnd, tcstr);
        MessageBox(hwnd, tcmes[25], tcmes[12], MB_OK | MB_ICONWARNING);
        OutputToEditbox(hedi_out, tcmes[26]);
        working = false;
        EnableWindow(hbtn_ok, TRUE);
        EnableWindow(hedi0, TRUE);
        EnableWindow(hbtn_abort, FALSE);
        EnableMenuItem(hmenu, 2, MF_BYPOSITION | MF_ENABLED); // Re-enable the "options" menu
        CommandBar_DrawMenuBar(hCmdBar, 1); // Redraw the "options" menu
        InvalidateRect(hwnd, NULL, FALSE);
        SendMessage(hwnd, APP_SETFOCUS, 0, 0);
        StoppingTimer--;
        if(!StoppingTimer) SetTimer(hwnd, 1, TIMER_AWAIT, NULL);
        return 0;
    }

    // Calculation
    while(1){
        while(i<=N){
            if(aborted) break;
            if(N%i==0 && N!=i){
                chk=true;
                break;
            }
            if(N/i<i || N==i){
                chk=false;
                break;
            }
            if(i==2) i++;
            else i+=2;
        }
        if(aborted) break;
        if(chk){
            wsprintf(tcstr, TEXT("%I64dx"), i); // Convert found prime factor to a string, then add "x"
            _tcscat(tcresult, tcstr); // Add to the result string
        }else{
            wsprintf(tcstr, TEXT("%I64d"), N); // Convert the number itself to a string (when it is a prime number)
            _tcscat(tcresult, tcstr); // Add to the result string
            break;
        }
        N/=i;
        cnt++;
    }
    if(cnt==0 && N>1) _tcscat(tcresult, tcmes[27]); // When the input number was a prime number

    if(aborted){ // When aborted
        working = false;
        EnableWindow(hbtn_ok, TRUE);
        EnableWindow(hedi0, TRUE);
        EnableWindow(hbtn_abort, FALSE);
        EnableMenuItem(hmenu, 2, MF_BYPOSITION | MF_ENABLED); // Re-enable the "options" menu
        CommandBar_DrawMenuBar(hCmdBar, 1); // Redraw the "options" menu
        wsprintf(tcstr, TEXT("%s - ") TARGET_CPU, tcmes[0]);
        SetWindowText(hwnd, tcstr);
        SendMessage(hwnd, APP_SETFOCUS, 0, 0);
        aborted = false;
        StoppingTimer--;
        if(!StoppingTimer) SetTimer(hwnd, 1, TIMER_AWAIT, NULL);
        return 0;
    }

    // Generate a result string
    wsprintf(tcstr, TEXT("%s%I64d = %s"), tcmes[28], num[0], tcresult);
    
    // Update the Window's title and the result box
    OutputToEditbox(hedi_out, tcstr);
    SendMessage(hedi_out, EM_REPLACESEL, 0, (WPARAM)TEXT("\r\n")); // When you add a string sequencely, this single function works.
    wsprintf(tcresult, TEXT(" - %s"), tcmes[0]);
    _tcscat(tcstr, tcresult);
    SetWindowText(hwnd, tcstr);

    working = false;
    EnableWindow(hbtn_ok, TRUE);
    EnableWindow(hedi0, TRUE);
    EnableWindow(hbtn_abort, FALSE);
    EnableMenuItem(hmenu, 2, MF_BYPOSITION | MF_ENABLED); // Re-enable the "options" menu
    CommandBar_DrawMenuBar(hCmdBar, 1); // Redraw the "options" menu
    InvalidateRect(hwnd, NULL, FALSE);
    StoppingTimer--;
    if(!StoppingTimer) SetTimer(hwnd, 1, TIMER_AWAIT, NULL);
    SendMessage(hwnd, APP_SETFOCUS, 0, 0);
    return 0;
}

DWORD WINAPI ListPrimeNumbers(LPVOID lpParameter){
    LONGLONG cnt=0, i, j;
    DWORD ret;
    TCHAR tcresult[1024] = TEXT(""), tcstr[1024] = TEXT(""), strFile[MAX_PATH] = TEXT("");
    CHAR cresult[1024] = "", cstr[1024] = "", cmes[1024];
    HANDLE hfile = NULL;
    OPENFILENAME ofn = {0};

    // Adjust the input number (mainly for blank or "0")
    if(num[0]<2) num[0]=2;
    if(num[0]!=2 && !(num[0]%2)) num[0]++;
    if(num[1]<1) num[1]=0x7fffffffffffffff;
    if(num[2]<1 || onlycnt) num[2]=0x7fffffffffffffff;
    if(num[1]-num[0]<1) { // In case the input was invalid
        wsprintf(tcstr, TEXT("%s - ") TARGET_CPU, tcmes[0]);
        SetWindowText(hwnd, tcstr);
        MessageBox(hwnd, tcmes[29], tcmes[12], MB_OK | MB_ICONWARNING);
        OutputToEditbox(hedi_out, tcmes[30]);
        FinalizeErrorLPN();
        return 0;
    }

    // When the entered upper limit of the number of prime numbers was large
    if(num[2]>1000 && !usefile && !onlycnt){
        if(IDYES == MessageBox(hwnd, tcmes[31], tcmes[32], MB_YESNO | MB_ICONINFORMATION)){
            usefile = true;
            CheckMenuItem(hmenu, 2061, MF_BYCOMMAND | MF_CHECKED);
        }else{
            FinalizeErrorLPN();
            return 0;
        }
    }

    // Prepare for the text file output (unless "Display only number of prime numbers" mode)
    if(!onlycnt && usefile){
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFilter = TEXT("ANSI TEXT (*.txt)\0*.txt\0")
                          TEXT("All files (*.*)\0*.*\0");
        ofn.lpstrFile = strFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrDefExt = TEXT(".txt");
        ofn.lpstrTitle = tcmes[33];
        ofn.Flags = (overwrite ? OFN_OVERWRITEPROMPT : NULL); // In case it is an existing file, warn if the mode is "overwrite."
        if(!GetSaveFileName(&ofn)){
            wsprintf(tcstr, TEXT("%s - ") TARGET_CPU, tcmes[0]);
            SetWindowText(hwnd, tcstr);
            FinalizeErrorLPN();
            return 0;
        }

        hfile = CreateFile(
            strFile,
            GENERIC_WRITE,
            FILE_SHARE_READ, // Allow other softwares to just "read" the file while processing
            NULL,
            (overwrite ? CREATE_ALWAYS : OPEN_ALWAYS), // If "overwrite" is enebled, always create a file. If not, open or create.
            FILE_ATTRIBUTE_NORMAL,
            NULL);
        if(hfile == INVALID_HANDLE_VALUE){
            wsprintf(tcstr, TEXT("%s - ") TARGET_CPU, tcmes[0]);
            SetWindowText(hwnd, tcstr);
            MessageBox(hwnd, tcmes[11], tcmes[12], MB_OK | MB_ICONWARNING);
            OutputToEditbox(hedi_out, tcmes[34]);
            FinalizeErrorLPN();
            return 0;
        }
        if( (ret=SetFilePointer(hfile, 0, NULL, FILE_END)) ){ // Set the File Pointer at the end of the file & judge whether adding to an existing file or a new file
            if(ret==0xFFFFFFFF){
                wsprintf(tcstr, TEXT("%s - ") TARGET_CPU, tcmes[0]);
                SetWindowText(hwnd, tcstr);
                MessageBox(hwnd, tcmes[35], tcmes[12], MB_OK | MB_ICONWARNING);
                OutputToEditbox(hedi_out, tcmes[36]);
                CloseHandle(hfile);
                FinalizeErrorLPN();
                return 0;
            }
            WriteFile(hfile, "\r\n\r\n", strlen("\r\n\r\n")*sizeof(CHAR), &dwtemp, NULL); // If adding, insert two new lines
        }
    }

    if(!onlycnt && !usefile) OutputToEditbox(hedi_out, tcmes[28]);
    else if(!onlycnt && usefile){
        WideCharToMultiByte(932, NULL, tcmes[28], lstrlen(tcmes[28]), cmes,
            WideCharToMultiByte(932, NULL, tcmes[28], lstrlen(tcmes[28]), NULL, 0, NULL, NULL), // Call twice to get the length of the converted text
            NULL, NULL);
        if(!WriteFile(hfile, cmes, strlen(cmes)*sizeof(CHAR), &dwtemp, NULL)){
            wsprintf(tcstr, TEXT("%s - ") TARGET_CPU, tcmes[0]);
            SetWindowText(hwnd, tcstr);
            MessageBox(hwnd, tcmes[15], tcmes[12], MB_OK | MB_ICONWARNING);
            OutputToEditbox(hedi_out, tcmes[37]);
            CloseHandle(hfile);
            FinalizeErrorLPN();
            return 0;
        }
        OutputToEditbox(hedi_out, tcmes[38]);
    }

    // Process
    for(i=num[0]; i<=num[1] && cnt<num[2]; i++){
        for(j=2; j<=i; j++) {
            if(aborted) break;
            if(i%j==0 && i!=j) break;
            if(i/j<j || i==j) {
                if(!onlycnt){
                    tcresult[0] = L'\0'; cresult[0] = '\0';
                    if(cnt){
                        wsprintf(tcresult, TEXT(", %I64d"), i);
                        if(usefile) sprintf(cresult, ", %I64d", i);
                    }else{
                        wsprintf(tcresult, TEXT("%I64d"), i);
                        if(usefile) sprintf(cresult, "%I64d", i);
                    }
                    if(usefile) WriteFile(hfile, cresult, strlen(cresult)*sizeof(CHAR), &dwtemp, NULL);
                    else OutputToEditbox(hedi_out, tcresult);
                }
                cnt++;
                break;
            }
            if(j!=2) j++; // If not 2, add 1 again.
        }
        if(aborted) break;
        if(i!=2) i++; // If not 2, add 1 again.
    }

    // When aborted
    if(aborted) {
        if(!onlycnt && !usefile) OutputToEditbox(hedi_out, TEXT("\r\n"));
        else if(!onlycnt && usefile){
            WriteFile(hfile, "\r\n", strlen("\r\n")*sizeof(CHAR), &dwtemp, NULL);
            CloseHandle(hfile);
        } else if(onlycnt){
            wsprintf(tcstr, tcmes[46], cnt, num[0], num[1], num[2]);
            OutputToEditbox(hedi_out, tcstr);
        }
        FinalizeErrorLPN();
        return 0;
    }

    // Final output and so on
    if(!onlycnt && !usefile) OutputToEditbox(hedi_out, TEXT("\r\n"));
    else if(!onlycnt && usefile) WriteFile(hfile, "\r\n", strlen("\r\n")*sizeof(CHAR), &dwtemp, NULL);
    wsprintf(tcstr, tcmes[39], cnt, num[0], num[1], num[2]);
    WideCharToMultiByte(932, NULL, tcmes[39], lstrlen(tcmes[39]), cmes,
        WideCharToMultiByte(932, NULL, tcmes[39], lstrlen(tcmes[39]), NULL, 0, NULL, NULL), // Call twice to get the length of the converted text
        NULL, NULL);
    if(!onlycnt && usefile){
        sprintf(cstr, cmes, cnt, num[0], num[1], num[2]);
        WriteFile(hfile, cstr, strlen(cstr)*sizeof(CHAR), &dwtemp, NULL);
        WriteFile(hfile, "\r\n", strlen("\r\n")*sizeof(CHAR), &dwtemp, NULL);
        CloseHandle(hfile);
    }
    OutputToEditbox(hedi_out, tcstr);
    SendMessage(hedi_out, EM_REPLACESEL, 0, (WPARAM)TEXT("\r\n")); // '\r': CR, '\n': LF
    wsprintf(tcresult, TEXT(" - %s"), tcmes[0]);
    _tcscat(tcstr, tcresult);
    SetWindowText(hwnd, tcstr);

    working = false;
    EnableWindow(hbtn_ok, TRUE);
    EnableWindow(hedi0, TRUE);
    EnableWindow(hedi1, TRUE);
    if(!onlycnt) EnableWindow(hedi2, TRUE);
    EnableWindow(hbtn_abort, FALSE);
    EnableMenuItem(hmenu, 2, MF_BYPOSITION | MF_ENABLED);  // Re-enable the "options" menu
    CommandBar_DrawMenuBar(hCmdBar, 1); // Redraw the "options" menu
    InvalidateRect(hwnd, NULL, FALSE);
    SendMessage(hwnd, APP_SETFOCUS, 0, 0);
    StoppingTimer--;
    if(!StoppingTimer) SetTimer(hwnd, 1, TIMER_AWAIT, NULL);
    return 0;
}
