// Engine.h: interface for the CEngine class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ENGINE_H__22802DD7_D7EB_4234_9781_E237657471AC__INCLUDED_)
#define AFX_ENGINE_H__22802DD7_D7EB_4234_9781_E237657471AC__INCLUDED_
#pragma once

#include <xrCPU_Pipe.h>

#include "engineAPI.h"
#include "eventAPI.h"
#include "xrSheduler.h"

class CInifile;

class ENGINE_API CEngine
{
	HMODULE hPSGP;
public:
	BENCH_SEC_SCRAMBLEMEMBER1
	// DLL api stuff
	CEngineAPI External;
	CEventAPI Event;
	CSheduler Sheduler;

	void Initialize();
	void Destroy();

	CEngine();
	~CEngine();
};

ENGINE_API extern xrDispatchTable PSGP;
ENGINE_API extern CEngine Engine;

extern ENGINE_API CInifile* pGameIni;

#define READ_IF_EXISTS(ltx,method,section,name,default_value)\
 (((ltx)->line_exist(section, name)) ? ((ltx)->method(section, name)) : (default_value))

#endif // !defined(AFX_ENGINE_H__22802DD7_D7EB_4234_9781_E237657471AC__INCLUDED_)
