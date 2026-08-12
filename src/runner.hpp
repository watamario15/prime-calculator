#ifndef RUNNER_HPP_
#define RUNNER_HPP_

#include "main.hpp"

#ifdef UNDER_CE
// The return type of a thread function.
typedef DWORD tret_t;
#else
// The return type of a thread function.
typedef unsigned tret_t;
#endif

namespace runner {
extern HANDLE hThread;
extern volatile bool isAborted;  // `volatile` is important here to avoid an optimization

// Reads input boxes and starts the calculation.
void begin(void);
}  // namespace runner

#endif
