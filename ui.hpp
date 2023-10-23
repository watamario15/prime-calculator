#ifndef UI_HPP_
#define UI_HPP_

#include "main.hpp"

namespace ui {
// Outputs an null-terminated string to the output box.
void appendOutput(const wchar_t *str);

/*
// Enables/Disables menu items from the smaller nearest 10 multiple to `_endID`.
void enableMenus(unsigned endID, bool enable);

// Sets a UI state.
void setState(enum app::state_t state, bool force = false);

// Focuses on a recently focused edit control if a non-edit control is given.
// Updates internal information if an edit control is given.
void updateFocus(HWND hWndCtl = NULL);

// Switches between wordwrap enabled and disabled for edit controls.
void switchWordwrap();

// Switches between dark/light theme.
void switchTheme();

// Sets the title of the main window.
void updateTitle();
*/
}  // namespace ui

#endif
