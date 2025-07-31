// EngineAPI.cpp: implementation of the CEngineAPI class.
//
//////////////////////////////////////////////////////////////////////

#include "Engine.h"
#include "EngineAPI.h"
#include "xrCore.h"
#include "xrXRC.h"

//#include "securom_api.h"

extern xr_token* vid_quality_token;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void __cdecl dummy(void)
{
};

CEngineAPI::CEngineAPI()
{
	//hGame = 0;
	//hRender = 0;
	hTuner = 0;
	pCreate = 0;
	pDestroy = 0;
	tune_pause = dummy;
	tune_resume = dummy;
}

CEngineAPI::~CEngineAPI()
{
	// destroy quality token here
	if (vid_quality_token)
	{
		xr_free(vid_quality_token);
		vid_quality_token = NULL;
	}
}

extern u32 renderer_value; //con cmd
ENGINE_API int g_current_renderer = 0;

ENGINE_API bool is_enough_address_space_available()
{
	return true; // we are on 64 bit, so it's always true
	//SYSTEM_INFO system_info;
	//    GetSystemInfo(&system_info);
	//    return (*(u32*)&system_info.lpMaximumApplicationAddress) > 0x90000000;
}

#ifndef DEDICATED_SERVER

extern BOOL DllMainXrRenderR1(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);
extern BOOL DllMainXrRenderR2(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);
extern BOOL DllMainXrRenderR3(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);
extern BOOL DllMainXrRenderR4(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);

#ifdef STATIC_RENDERER_R1
    #define DLL_MAIN_RENDERER DllMainXrRenderR1
#endif
#ifdef STATIC_RENDERER_R2
    #define DLL_MAIN_RENDERER DllMainXrRenderR2
#endif
#ifdef STATIC_RENDERER_R3
    #define DLL_MAIN_RENDERER DllMainXrRenderR3
#endif
#ifdef STATIC_RENDERER_R4
    #define DLL_MAIN_RENDERER DllMainXrRenderR4
#endif

void CEngineAPI::InitializeNotDedicated()
{
#ifdef STATIC_RENDERER_R4
	LPCSTR r4_name = "xrRender_R4.dll";
	//if (psDeviceFlags.test(rsR4))
    {
        // try to initialize R4
		psDeviceFlags.set(rsR2, FALSE);
		psDeviceFlags.set(rsR3, FALSE);
		Log("Loading DLL:", r4_name);
		DllMainXrRenderR4(NULL, DLL_PROCESS_ATTACH, NULL);
        //hRender = LoadLibrary(r4_name);
	//if (0 == hRender)
	//{
	//    // try to load R1
	//    Msg("! ...Failed - incompatible hardware/pre-Vista OS.");
	//    psDeviceFlags.set(rsR2, TRUE);
        //}
		g_current_renderer = 0;
    }
#endif

#ifdef STATIC_RENDERER_R3
	LPCSTR r3_name = "xrRender_R3.dll";
	//if (psDeviceFlags.test(rsR3))
	{
		// try to initialize R3
		psDeviceFlags.set(rsR2, FALSE);
		psDeviceFlags.set(rsR4, FALSE);
		Log("Loading DLL:", r3_name);
		DllMainXrRenderR3(NULL, DLL_PROCESS_ATTACH, NULL);
		//hRender = LoadLibrary(r3_name);
		//if (0 == hRender)
		//{
		//    // try to load R1
		//    Msg("! ...Failed - incompatible hardware/pre-Vista OS.");
		//    psDeviceFlags.set(rsR2, TRUE);
		//}
		//else
		g_current_renderer = 3;
	}
#endif

#ifdef STATIC_RENDERER_R2
	LPCSTR r2_name = "xrRender_R2.dll";
	//if (psDeviceFlags.test(rsR2))
    {
        // try to initialize R2
        psDeviceFlags.set(rsR3, FALSE);
		psDeviceFlags.set(rsR4, FALSE);
		Log("Loading DLL:", r2_name);
		DllMainXrRenderR2(NULL, DLL_PROCESS_ATTACH, NULL);
		//hRender = LoadLibrary(r2_name);
	//if (0 == hRender)
	//{
	//    // try to load R1
	//    Msg("! ...Failed - incompatible hardware.");
	//}
        //else
            g_current_renderer = 2;
    }
#endif
}
#endif // DEDICATED_SERVER

extern BOOL DllMainXrGame(HANDLE hModule, u32 ul_reason_for_call, LPVOID lpReserved);

extern "C"
DLL_Pure* __cdecl xrFactory_Create(CLASS_ID clsid);
extern "C"
void __cdecl xrFactory_Destroy(DLL_Pure* O);


void CEngineAPI::Initialize(void)
{
	//////////////////////////////////////////////////////////////////////////
	// render
#ifndef DEDICATED_SERVER
	InitializeNotDedicated();
#endif // DEDICATED_SERVER

#ifdef STATIC_RENDERER_R1
	LPCSTR r1_name = "xrRender_R1.dll";
	//if (0 == hRender)
    {
        // try to load R1
        psDeviceFlags.set(rsR4, FALSE);
        psDeviceFlags.set(rsR3, FALSE);
        psDeviceFlags.set(rsR2, FALSE);
        renderer_value = 0; //con cmd

        Log("Loading DLL:", r1_name);
		DllMainXrRenderR1(NULL, DLL_PROCESS_ATTACH, NULL);
		//hRender = LoadLibrary(r1_name);
	//if (0 == hRender) R_CHK(GetLastError());
        //R_ASSERT(hRender);
        g_current_renderer = 1;
    }
#endif

	Device.ConnectToRender();

	// game
	{
		LPCSTR g_name = "xrGame.dll";
		Log("Loading DLL:", g_name);
		//hGame = LoadLibrary(g_name);
		DllMainXrGame(NULL, DLL_PROCESS_ATTACH, NULL);
		//if (0 == hGame) R_CHK(GetLastError());
		//R_ASSERT2(hGame, "Game DLL raised exception during loading or there is no game DLL at all");
		//pCreate = (Factory_Create*)GetProcAddress(hGame, "xrFactory_Create");
		pCreate = xrFactory_Create;
		R_ASSERT(pCreate);
		//pDestroy = (Factory_Destroy*)GetProcAddress(hGame, "xrFactory_Destroy");
		pDestroy = xrFactory_Destroy;
		R_ASSERT(pDestroy);
	}

	//////////////////////////////////////////////////////////////////////////
	// vTune
	tune_enabled = FALSE;
	if (strstr(Core.Params, "-tune"))
	{
		LPCSTR g_name = "vTuneAPI.dll";
		Log("Loading DLL:", g_name);
		hTuner = LoadLibrary(g_name);
		if (0 == hTuner)
			R_CHK(GetLastError());
		R_ASSERT2(hTuner, "Intel vTune is not installed");
		tune_enabled = TRUE;
		tune_pause = (VTPause*)GetProcAddress(hTuner, "VTPause");
		R_ASSERT(tune_pause);
		tune_resume = (VTResume*)GetProcAddress(hTuner, "VTResume");
		R_ASSERT(tune_resume);
	}
}

void CEngineAPI::Destroy(void)
{
	//if (hGame) { FreeLibrary(hGame); hGame = 0; }
	DllMainXrGame(NULL, DLL_PROCESS_DETACH, NULL);
	//if (hRender) { FreeLibrary(hRender); hRender = 0; }
	DLL_MAIN_RENDERER(NULL, DLL_PROCESS_DETACH, NULL);
	pCreate = 0;
	pDestroy = 0;
	Engine.Event._destroy();
	XRC.r_clear_compact();
}

extern "C" {
typedef bool __cdecl SupportsAdvancedRenderingREF(void);
typedef bool /*_declspec(dllexport)*/ SupportsDX10RenderingREF();
typedef bool /*_declspec(dllexport)*/ SupportsDX11RenderingREF();
};

extern "C" {
#ifdef STATIC_RENDERER_R2
	bool SupportsAdvancedRendering();
#endif
#ifdef STATIC_RENDERER_R3
bool SupportsDX10Rendering();

#endif
#ifdef STATIC_RENDERER_R4
	bool SupportsDX11Rendering();
#endif
};

void CEngineAPI::CreateRendererList()
{
#ifdef DEDICATED_SERVER

    vid_quality_token = xr_alloc<xr_token>(2);

    vid_quality_token[0].id = 0;
    vid_quality_token[0].name = xr_strdup("renderer_r1");

    vid_quality_token[1].id = -1;
    vid_quality_token[1].name = NULL;

#else
	// TODO: ask renderers if they are supported!
	if (vid_quality_token != NULL) return;
	bool bSupports_r2 = false;
	bool bSupports_r2_5 = false;
	bool bSupports_r3 = false;
	bool bSupports_r4 = false;

	if (strstr(Core.Params, "-perfhud_hack"))
	{
		bSupports_r2 = true;
		bSupports_r2_5 = true;
		bSupports_r3 = true;
		bSupports_r4 = true;
	}
	else
	{
#ifdef STATIC_RENDERER_R2
		LPCSTR r2_name = "xrRender_R2.dll";
		// try to initialize R2
        Log("Loading DLL:", r2_name);
        //hRender = LoadLibrary(r2_name);
		DllMainXrRenderR2(NULL, DLL_PROCESS_ATTACH, NULL);
        //if (hRender)
        {
            bSupports_r2 = true;
            //SupportsAdvancedRenderingREF* test_rendering = (SupportsAdvancedRenderingREF*)GetProcAddress(hRender, "SupportsAdvancedRendering");
            SupportsAdvancedRenderingREF* test_rendering = SupportsAdvancedRendering;
            R_ASSERT(test_rendering);
            bSupports_r2_5 = test_rendering();
            //FreeLibrary(hRender);
        }
#endif

#ifdef STATIC_RENDERER_R3
		LPCSTR r3_name = "xrRender_R3.dll";
		// try to initialize R3
		Log("Loading DLL:", r3_name);
		// Hide "d3d10.dll not found" message box for XP
		SetErrorMode(SEM_FAILCRITICALERRORS);
		//hRender = LoadLibrary(r3_name);
		DllMainXrRenderR3(NULL, DLL_PROCESS_ATTACH, NULL);
		// Restore error handling
		SetErrorMode(0);
		//if (hRender)
		{
			//SupportsDX10RenderingREF* test_dx10_rendering = (SupportsDX10RenderingREF*)GetProcAddress(hRender, "SupportsDX10Rendering");
			SupportsDX10RenderingREF* test_dx10_rendering = SupportsDX10Rendering;
			R_ASSERT(test_dx10_rendering);
			bSupports_r3 = test_dx10_rendering();
			//FreeLibrary(hRender);
		}
#endif

#ifdef STATIC_RENDERER_R4
		LPCSTR r4_name = "xrRender_R4.dll";
		// try to initialize R4
        Log("Loading DLL:", r4_name);
        // Hide "d3d10.dll not found" message box for XP
        SetErrorMode(SEM_FAILCRITICALERRORS);
        //hRender = LoadLibrary(r4_name);
		DllMainXrRenderR4(NULL, DLL_PROCESS_ATTACH, NULL);
        // Restore error handling
        SetErrorMode(0);
        //if (hRender)
        {
            //SupportsDX11RenderingREF* test_dx11_rendering = (SupportsDX11RenderingREF*)GetProcAddress(hRender, "SupportsDX11Rendering");
            SupportsDX11RenderingREF* test_dx11_rendering = SupportsDX11Rendering;
            R_ASSERT(test_dx11_rendering);
            bSupports_r4 = test_dx11_rendering();
            //FreeLibrary(hRender);
        }
#endif
	}

	//hRender = 0;
	xr_vector<LPCSTR> _tmp;
#ifdef STATIC_RENDERER_R1
	_tmp.push_back("renderer_r1");
#endif
#ifdef STATIC_RENDERER_R2
	bool proceed = true;
	if (proceed &= bSupports_r2, proceed)
    {
        _tmp.push_back("renderer_r2a");
        _tmp.push_back("renderer_r2");
    }
    if (proceed &= bSupports_r2_5, proceed)
        _tmp.push_back("renderer_r2.5");
#endif
#ifdef STATIC_RENDERER_R3
	bool proceed = true;
	if (proceed &= bSupports_r3, proceed)
		_tmp.push_back("renderer_r3");
#endif
#ifdef STATIC_RENDERER_R4
	bool proceed = true;
	if (proceed &= bSupports_r4, proceed)
        _tmp.push_back("renderer_r4");
#endif

	R_ASSERT2(_tmp.size() != 0, "No valid renderer found, please use a render system that's supported by your PC");

	u32 _cnt = _tmp.size() + 1;
	vid_quality_token = xr_alloc<xr_token>(_cnt);

	vid_quality_token[_cnt - 1].id = -1;
	vid_quality_token[_cnt - 1].name = NULL;

	//#ifdef DEBUG
	Msg("Available render modes[%d]:", _tmp.size());
	//#endif // DEBUG
	for (u32 i = 0; i < _tmp.size(); ++i)
	{
		vid_quality_token[i].id = i;
		vid_quality_token[i].name = _tmp[i];
		//#ifdef DEBUG
		Msg("[%s]", _tmp[i]);
		//#endif // DEBUG
	}
#endif //#ifndef DEDICATED_SERVER
}
