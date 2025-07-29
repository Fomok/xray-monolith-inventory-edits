#ifndef xrCoreH
#define xrCoreH
#pragma once

#include <windows.h>

#include "xrstring.h"
#include "log.h"

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

extern XRCORE_API xrCore Core;

#endif
