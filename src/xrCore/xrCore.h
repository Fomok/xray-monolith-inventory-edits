#ifndef xrCoreH
#define xrCoreH
#pragma once

#include <windows.h>
#include "_stl_extensions.h"
#include "xrstring.h"

// stl ext
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
