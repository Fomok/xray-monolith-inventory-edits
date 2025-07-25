#ifndef xrCoreH
#define xrCoreH
#pragma once

// *** try to minimize code bloat of STLport
#ifdef XRCORE_EXPORTS // no exceptions, export allocator and common stuff
#define _STLP_DESIGNATED_DLL 1
#define _STLP_USE_DECLSPEC 1
#else
#define _STLP_USE_DECLSPEC 1 // no exceptions, import allocator and common stuff
#endif

#ifdef XRCORE_STATIC
# define NO_FS_SCAN
#endif

#define MODULE_NAME "xrCore.dll"

// Our headers
//#ifdef XRCORE_STATIC
//# define XRCORE_API
//#else
# ifdef XRCORE_EXPORTS
# define XRCORE_API
//__declspec(dllexport)
# else
# define XRCORE_API
//__declspec(dllimport)
# endif
//#endif

#include <windows.h>
#include "_stl_extensions.h"
#include "xrstring.h"

// stl ext
struct XRCORE_API xr_rtoken
{
	shared_str name;
	int id;

	xr_rtoken(LPCSTR _nm, int _id)
	{
		name = _nm;
		id = _id;
	}

public:
	void rename(LPCSTR _nm) { name = _nm; }
	bool equal(LPCSTR _nm) { return (0 == xr_strcmp(*name, _nm)); }
};

#pragma pack (push,1)
struct XRCORE_API xr_shortcut
{
	enum
	{
		flShift = 0x20,
		flCtrl = 0x40,
		flAlt = 0x80,
	};

	union
	{
		struct
		{
			u8 key;
			Flags8 ext;
		};

		u16 hotkey;
	};

	xr_shortcut(u8 k, BOOL a, BOOL c, BOOL s) : key(k)
	{
		ext.assign(u8((a ? flAlt : 0) | (c ? flCtrl : 0) | (s ? flShift : 0)));
	}

	xr_shortcut()
	{
		ext.zero();
		key = 0;
	}

	bool similar(const xr_shortcut& v) const { return ext.equal(v.ext) && (key == v.key); }
};
#pragma pack (pop)

DEFINE_VECTOR(shared_str, RStringVec, RStringVecIt);
DEFINE_SET(shared_str, RStringSet, RStringSetIt);
DEFINE_VECTOR(xr_rtoken, RTokenVec, RTokenVecIt);

#define xr_pure_interface __interface

#include "log.h"

// destructor
template <class T>
class destructor
{
	T* ptr;
public:
	destructor(T* p) { ptr = p; }
	~destructor() { xr_delete(ptr); }
	IC T& operator()()
	{
		return *ptr;
	}
};

// ********************************************** The Core definition
class XRCORE_API xrCore
{
public:
	string64 ApplicationName;
	string_path ApplicationPath;
	string_path WorkingPath;
	string64 UserName;
	string64 CompName;
	char* Params;
	DWORD dwFrame;
	bool april1;

public:
	void _initialize(LPCSTR ApplicationName, LogCallback cb = 0, BOOL init_fs = TRUE, LPCSTR fs_fname = 0);
	void _destroy();
};

//Borland class dll interface
#define _BCL __stdcall

//Borland global function dll interface
#define _BGCL __stdcall


extern XRCORE_API xrCore Core;

#endif
