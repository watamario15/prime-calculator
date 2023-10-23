#include "util.hpp"

namespace util {
#ifdef UNDER_CE
int messageBox(HWND hWnd, HINSTANCE hInst, const wchar_t *lpText, const wchar_t *lpCaption, unsigned uType) {
  UNREFERENCED_PARAMETER(hInst);
  return MessageBoxW(hWnd, lpText, lpCaption, uType);
}
#else
// The function pointer type for TaskDialog API.
typedef HRESULT(__stdcall *TaskDialog_t)(HWND hwndOwner, HINSTANCE hInstance, const wchar_t *pszWindowTitle,
                                         const wchar_t *pszMainInstruction, const wchar_t *pszContent,
                                         int dwCommonButtons, const wchar_t *pszIcon, int *pnButton);

int messageBox(HWND hWnd, HINSTANCE hInst, const wchar_t *lpText, const wchar_t *lpCaption, unsigned uType) {
  // Tests whether uType uses some features that TaskDialog doesn't support.
  if (uType & ~(MB_ICONMASK | MB_TYPEMASK)) goto mbfallback;

  int buttons;
  switch (uType & MB_TYPEMASK) {
    case MB_OK:
      buttons = 1;
      break;
    case MB_OKCANCEL:
      buttons = 1 + 8;
      break;
    case MB_RETRYCANCEL:
      buttons = 16 + 8;
      break;
    case MB_YESNO:
      buttons = 2 + 4;
      break;
    case MB_YESNOCANCEL:
      buttons = 2 + 4 + 8;
      break;
    default:  // Not supported by TaskDialog.
      goto mbfallback;
  }

  wchar_t *icon;
  switch (uType & MB_ICONMASK) {
    case 0:
      icon = NULL;
      break;
    case MB_ICONWARNING:  // Same value as MB_ICONEXCLAMATION.
      icon = MAKEINTRESOURCEW(-1);
      break;
    case MB_ICONERROR:  // Same value as MB_ICONSTOP and MB_ICONHAND.
      icon = MAKEINTRESOURCEW(-2);
      break;
    default:  // Substitute anything else for the information icon.
      icon = MAKEINTRESOURCEW(-3);
  }

  {
    // Tries to load the TaskDialog API which is a newer substitite of MessageBoxW.
    // This API is per monitor DPI aware but doesn't exist before Windows Vista.
    // To make the system select version 6 comctl32.dll, we don't use the full path here.
    HMODULE comctl32 = LoadLibraryW(L"comctl32.dll");
    if (!comctl32) goto mbfallback;

    TaskDialog_t taskDialog = (TaskDialog_t)(void *)GetProcAddress(comctl32, "TaskDialog");
    if (!taskDialog) {
      FreeLibrary(comctl32);
      goto mbfallback;
    }

    int result;
    taskDialog(hWnd, hInst, lpCaption, L"", lpText, buttons, icon, &result);
    FreeLibrary(comctl32);
    return result;
  }

mbfallback:
  return MessageBoxW(hWnd, lpText, lpCaption, uType);
}
#endif
}  // namespace util
