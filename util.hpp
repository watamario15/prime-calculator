#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <string>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace util {
// Shows a message box with TaskDialog or MessageBoxW.
//
// When the system this program is running on supports TaskDialog, and no features that are MessageBoxW specific
// (MB_ABORTRETRYIGNORE, MB_CANCELTRYCONTINUE, MB_HELP, default selection, etc) is used, this function uses TaskDialog.
// In other cases, uses MessageBoxW. This function substitutes MB_ICONQUESTION with MB_ICONINFORMATION on TaskDialog, as
// it doesn't support it.
int messageBox(HWND hWnd, HINSTANCE hInst, const wchar_t *lpText, const wchar_t *lpCaption, unsigned uType);
}  // namespace util
#endif
