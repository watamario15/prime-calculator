#ifndef RUNNER_HPP_
#define RUNNER_HPP_

#include "main.hpp"

#ifdef __BORLANDC__
typedef ULONGLONG (*wcstoull_t)(const wchar_t *str, wchar_t **endptr, int base);
#define ULLONG_MAX 0xffffffffffffffffui64
#endif

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

// Prime factorization thread
tret_t WINAPI primeFactor(void *lpParameter);

// Prime enumeration thread
tret_t WINAPI primeEnumerator(void *lpParameter);
}  // namespace runner

#endif
