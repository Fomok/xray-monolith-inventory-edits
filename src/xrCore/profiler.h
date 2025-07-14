#pragma once

// Profiler backends
#define PROFILER_NONE   0
#define PROFILER_OPTICK 1

// Set active profiler backend
#if !defined(XRCORE_PROFILER)
    #define XRCORE_PROFILER PROFILER_NONE
//  #define XRCORE_PROFILER PROFILER_OPTICK
#endif

// Implement profiler macro interface
#if XRCORE_PROFILER == PROFILER_OPTICK
#	include "optick.h"
#	define PROF_THREAD(...) OPTICK_THREAD(__VA_ARGS__)
#	define PROF_START_CAPTURE() OPTICK_START_CAPTURE()
#	define PROF_STOP_CAPTURE() OPTICK_STOP_CAPTURE()
#	define PROF_SAVE_CAPTURE(...) OPTICK_SAVE_CAPTURE(__VA_ARGS__)
#	define PROF_FRAME(...) OPTICK_FRAME(__VA_ARGS__)
#	define PROF_EVENT(...) OPTICK_EVENT(__VA_ARGS__)
#	define START_PROFILE(...) { PROF_EVENT(__VA_ARGS__)
#	define STOP_PROFILE		}
#else
#	define PROF_THREAD(...)
#	define PROF_START_CAPTURE()
#	define PROF_STOP_CAPTURE()
#	define PROF_SAVE_CAPTURE(...)
#	define PROF_FRAME(...)
#	define PROF_EVENT(...)
#	define START_PROFILE(...) {
#	define STOP_PROFILE		}
#endif
