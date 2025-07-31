#ifndef STDAFX_3DA
#define STDAFX_3DA
#pragma once

// you must define ENGINE_BUILD then building the engine itself
// and not define it if you are about to build DLL
/*
#ifndef NO_ENGINE_API
#ifdef ENGINE_BUILD
#define DLL_API
//__declspec(dllimport)
#define ENGINE_API
//__declspec(dllexport)
#else
#undef DLL_API
#define DLL_API
//__declspec(dllexport)
#define ENGINE_API
//__declspec(dllimport)
#endif
#else
#define ENGINE_API
#define DLL_API
#endif // NO_ENGINE_API
*/

#include "device.h"
#include "../xrcore/fs.h"

#include "../xrcdb/xrXRC.h"

#include "../xrSound/sound.h"

class CInifile;
extern ENGINE_API CInifile* pGameIni;

#ifndef DEBUG
# define LUABIND_NO_ERROR_CHECKING
#endif

#if !defined(DEBUG) || defined(FORCE_NO_EXCEPTIONS)
# define LUABIND_NO_EXCEPTIONS
# define BOOST_NO_EXCEPTIONS
#endif

#define LUABIND_DONT_COPY_STRINGS

#define READ_IF_EXISTS(ltx,method,section,name,default_value)\
 (((ltx)->line_exist(section, name)) ? ((ltx)->method(section, name)) : (default_value))

#endif // !defined STDAFX_3DA
