#include "ui.hpp"

#include "msg.hpp"
#include "wproc.hpp"

namespace ui {
// Outputs an null-terminated string to the output box.
void appendOutput(const wchar_t *str) {
  int editLen = (int)SendMessageW(app::hEdiOut, WM_GETTEXTLENGTH, 0, 0);
  SendMessageW(app::hEdiOut, EM_SETSEL, editLen, editLen);    // Moves the caret to the end of the output box
  SendMessageW(app::hEdiOut, EM_REPLACESEL, 0, (WPARAM)str);  // Writes the content
}
}  // namespace ui
