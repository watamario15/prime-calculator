#ifndef RUNNER_HPP_
#define RUNNER_HPP_

#include "main.hpp"

#ifdef UNDER_CE
// The return type of a thread function.
typedef DWORD tret_t;
#else
// The return type of a thread function.
typedef unsigned int tret_t;
#endif

namespace runner {
extern unsigned int timerID;
extern HANDLE hThread;
extern volatile bool isAborted;  // `volatile` is important here to avoid an optimization

// Reads input boxes and starts the calculation.
void begin(void);

// Prime factorization thread
tret_t WINAPI primeFactor(void *lpParameter);

// Prime enumeration thread
tret_t WINAPI primeEnumerator(void *lpParameter);
}  // namespace runner

#endif
