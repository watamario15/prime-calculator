#ifndef WPROC_HPP_
#define WPROC_HPP_

#include "main.hpp"

namespace wproc {
struct editorprops_t {
  WNDPROC defProc;
  HWND hPrevWnd;
  HWND hNextWnd;
  bool runOnEnter;
};

// Window procedure for the main window.
LRESULT CALLBACK wndProc(HWND hWnd, unsigned uMsg, WPARAM wParam, LPARAM lParam);

// Hook window procedure for input boxes.
LRESULT CALLBACK inputProc(HWND hWnd, unsigned uMsg, WPARAM wParam, LPARAM lParam);
}  // namespace wproc

#endif
