#ifndef xrPlatformH
#define xrPlatformH
#pragma once

#ifndef DEBUG
#ifdef _DEBUG
#define DEBUG
#endif
#ifdef MIXED
#define DEBUG
#endif
#endif

#ifndef DEBUG
#define MASTER_GOLD
#endif // DEBUG

//#define BENCHMARK_BUILD

#ifdef BENCHMARK_BUILD
#define BENCH_SEC_CALLCONV __stdcall
#define BENCH_SEC_SCRAMBLEVTBL1 virtual int GetFlags() { return 1;}
#define BENCH_SEC_SCRAMBLEVTBL2 virtual void* GetData() { return 0;}
#define BENCH_SEC_SCRAMBLEVTBL3 virtual void* GetCache(){ return 0;}
#define BENCH_SEC_SIGN , void *pBenchScrampleVoid = 0
#define BENCH_SEC_SCRAMBLEMEMBER1 float m_fSrambleMember1;
#define BENCH_SEC_SCRAMBLEMEMBER2 float m_fSrambleMember2;
#else // BENCHMARK_BUILD
#define BENCH_SEC_CALLCONV
#define BENCH_SEC_SCRAMBLEVTBL1
#define BENCH_SEC_SCRAMBLEVTBL2
#define BENCH_SEC_SCRAMBLEVTBL3
#define BENCH_SEC_SIGN
#define BENCH_SEC_SCRAMBLEMEMBER1
#define BENCH_SEC_SCRAMBLEMEMBER2
#endif // BENCHMARK_BUILD

#if (defined(_DEBUG) || defined(MIXED) || defined(DEBUG)) && !defined(FORCE_NO_EXCEPTIONS)
// "debug" or "mixed"
#if !defined(_CPPUNWIND)
#error Please enable exceptions...
#endif
#define _HAS_EXCEPTIONS 1 // STL
#define XRAY_EXCEPTIONS 1 // XRAY
#else
// "release"
#if defined(_CPPUNWIND) && !defined __clang__
#error Please disable exceptions...
#endif
#define _HAS_EXCEPTIONS 1 // STL
#define XRAY_EXCEPTIONS 0 // XRAY
#define LUABIND_NO_EXCEPTIONS
#pragma warning(disable:4530)
#endif

#if !defined(_MT)
// multithreading disabled
#error Please enable multi-threaded library...
#endif
#ifdef _EDITOR
# define NO_FS_SCAN
#endif

// inline control - redefine to use compiler's heuristics ONLY
// it seems "IC" is misused in many places which cause code-bloat
// ...and VC7.1 really don't miss opportunities for inline :)
#ifdef _EDITOR
# define __forceinline inline
#endif
#define _inline inline
#define __inline inline
#define IC inline
#define ICF __forceinline // !!! this should be used only in critical places found by PROFILER
#ifdef _EDITOR
# define ICN
#else
# define ICN __declspec (noinline)
#endif

#define UNUSED(...) (void)(__VA_ARGS__)

#ifndef DEBUG
#pragma inline_depth ( 254 )
#pragma inline_recursion( on )
//#pragma intrinsic (abs, fabs, fmod, sin, cos, tan, asin, acos, atan, sqrt, exp, log, log10, strcat)
#endif

#define ALIGN(a) __declspec(align(a))

#endif // xrPlatformH