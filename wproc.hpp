#ifndef WPROC_HPP_
#define WPROC_HPP_

#include "main.hpp"

namespace wproc {
struct editorprops_t {
  WNDPROC defProc;
  HWND hNextWnd;
  bool runOnEnter;
};

// Window procedure for the main window.
LRESULT CALLBACK wndProc(HWND hWnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam);

// Hook window procedure for input boxes.
LRESULT CALLBACK inputProc(HWND hWnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam);
}  // namespace wproc

#endif
