// EngineAPI.cpp: implementation of the CEngineAPI class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EngineAPI.h"
#include "../xrcdb/xrXRC.h"

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
    hGame = nullptr;
    hRender = nullptr;
    hTuner = nullptr;
    pCreate = nullptr;
    pDestroy = nullptr;
    tune_pause = dummy;
    tune_resume = dummy;
    tune_enabled = false;
}

CEngineAPI::~CEngineAPI()
{
    // destroy quality token here
    if (vid_quality_token)
    {
        for (int i = 0; vid_quality_token[i].name; i++)
        {
            xr_free(vid_quality_token[i].name);
        }
        xr_free(vid_quality_token);
        vid_quality_token = nullptr;
    }
}

extern u32 renderer_value; //con cmd
ENGINE_API int g_current_renderer = 0;

ENGINE_API bool is_enough_address_space_available()
{
    SYSTEM_INFO system_info;

	GetSystemInfo(&system_info);

	return (*(u32*)&system_info.lpMaximumApplicationAddress) > 0x90000000;
}

void CEngineAPI::InitializeNotDedicated()
{
	LPCSTR r2_name = "xrRender_R2.dll";
    LPCSTR r3_name = "xrRender_R3.dll";
    LPCSTR r4_name = "xrRender_R4.dll";

    if (psDeviceFlags.test(rsR4))
    {
        // try to initialize R4
        Log("Loading DLL:", r4_name);
        hRender = LoadLibrary(r4_name);
        if (nullptr == hRender)
        {
            // try to load R1
            Msg("! ...Failed - incompatible hardware/pre-Vista OS.");
            psDeviceFlags.set(rsR2, TRUE);
        }
    }

    if (psDeviceFlags.test(rsR3))
    {
        // try to initialize R3
        Log("Loading DLL:", r3_name);
        hRender = LoadLibrary(r3_name);
        if (nullptr == hRender)
        {
            // try to load R1
            Msg("! ...Failed - incompatible hardware/pre-Vista OS.");
            psDeviceFlags.set(rsR2, TRUE);
        }
        else
            g_current_renderer = 3;
    }

    if (psDeviceFlags.test(rsR2))
    {
        // try to initialize R2
        psDeviceFlags.set(rsR4, FALSE);
        psDeviceFlags.set(rsR3, FALSE);
        Log("Loading DLL:", r2_name);
        hRender = LoadLibrary(r2_name);
        if (nullptr == hRender)
        {
            // try to load R1
            Msg("! ...Failed - incompatible hardware.");
        }
        else
            g_current_renderer = 2;
    }
}

void CEngineAPI::Initialize()
{
    //////////////////////////////////////////////////////////////////////////
    // render
    LPCSTR r1_name = "xrRender_R1.dll";

    InitializeNotDedicated();

    if (!hRender)
    {
        // try to load R1
        psDeviceFlags.set(rsR4, FALSE);
        psDeviceFlags.set(rsR3, FALSE);
        psDeviceFlags.set(rsR2, FALSE);
        renderer_value = 0; //con cmd

        Log("Loading DLL:", r1_name);
        hRender = LoadLibrary(r1_name);
        if (!hRender) R_CHK(GetLastError());
        R_ASSERT(hRender);
        g_current_renderer = 1;
    }

    Device.ConnectToRender();

    // game
    {
        LPCSTR g_name = "xrGame.dll";
        Log("Loading DLL:", g_name);
        hGame = LoadLibrary(g_name);
        if (!hGame) R_CHK(GetLastError());
        R_ASSERT2(hGame, "Game DLL raised exception during loading or there is no game DLL at all");
        pCreate = (Factory_Create*)(GetProcAddress(hGame, "xrFactory_Create"));
        R_ASSERT(pCreate);
        pDestroy = (Factory_Destroy*)(GetProcAddress(hGame, "xrFactory_Destroy"));
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
        if (!hTuner) R_CHK(GetLastError());
        R_ASSERT2(hTuner, "Intel vTune is not installed");
        tune_enabled = TRUE;
        tune_pause = (VTPause*)GetProcAddress(hTuner, "VTPause");
        R_ASSERT(tune_pause);
        tune_resume = (VTResume*)GetProcAddress(hTuner, "VTResume");
        R_ASSERT(tune_resume);
    }
}

void CEngineAPI::Destroy()
{
    if (hGame) { FreeLibrary(hGame); hGame = nullptr; }
    if (hRender) { FreeLibrary(hRender); hRender = nullptr; }
    pCreate = nullptr;
    pDestroy = nullptr;
    Engine.Event._destroy();
    XRC.r_clear_compact();
}

extern "C" {
	using SupportsAdvancedRendering = bool __cdecl(void);
    using SupportsDX10Rendering = bool __declspec(dllexport)();
    using SupportsDX11Rendering = bool __declspec(dllexport)();
};

void CEngineAPI::CreateRendererList()
{
    // TODO: ask renderers if they are supported!
    if (vid_quality_token != nullptr) return;
    bool bSupports_r2 = false;
    bool bSupports_r2_5 = false;
    bool bSupports_r3 = false;
    bool bSupports_r4 = false;

    LPCSTR r2_name = "xrRender_R2.dll";
    LPCSTR r3_name = "xrRender_R3.dll";
    LPCSTR r4_name = "xrRender_R4.dll";

    if (strstr(Core.Params, "-perfhud_hack"))
    {
        bSupports_r2 = true;
        bSupports_r2_5 = true;
        bSupports_r3 = true;
        bSupports_r4 = true;
    }
    else
    {
        // try to initialize R2
        Log("Loading DLL:", r2_name);
        hRender = LoadLibrary(r2_name);
        if (hRender)
        {
            bSupports_r2 = true;
            auto* test_rendering = (SupportsAdvancedRendering*)(GetProcAddress(hRender, "SupportsAdvancedRendering"));
            R_ASSERT(test_rendering);
            bSupports_r2_5 = test_rendering();
            FreeLibrary(hRender);
        }

        // try to initialize R3
        Log("Loading DLL:", r3_name);
        // Hide "d3d10.dll not found" message box for XP
        SetErrorMode(SEM_FAILCRITICALERRORS);
        hRender = LoadLibrary(r3_name);
        // Restore error handling
        SetErrorMode(0);
        if (hRender)
        {
            auto* test_dx10_rendering =  (SupportsDX10Rendering*)(GetProcAddress(hRender, "SupportsDX10Rendering"));
            R_ASSERT(test_dx10_rendering);
            bSupports_r3 = test_dx10_rendering();
            FreeLibrary(hRender);
        }

        // try to initialize R4
        Log("Loading DLL:", r4_name);
        // Hide "d3d10.dll not found" message box for XP
        SetErrorMode(SEM_FAILCRITICALERRORS);
        hRender = LoadLibrary(r4_name);
        // Restore error handling
        SetErrorMode(0);
        if (hRender)
        {
            auto* test_dx11_rendering = (SupportsDX11Rendering*)(GetProcAddress(hRender, "SupportsDX11Rendering"));
            R_ASSERT(test_dx11_rendering);
            bSupports_r4 = test_dx11_rendering();
            FreeLibrary(hRender);
        }
    }

    hRender = nullptr;

    xr_vector<LPCSTR> _tmp;
    u32 i = 0;
    bool bBreakLoop = false;
    for (; i < 6; ++i)
    {
        switch (i)
        {
        case 1:
            if (!bSupports_r2)
                bBreakLoop = true;
            break;
        case 3: //"renderer_r2.5"
            if (!bSupports_r2_5)
                bBreakLoop = true;
            break;
        case 4: //"renderer_r_dx10"
            if (!bSupports_r3)
                bBreakLoop = true;
            break;
        case 5: //"renderer_r_dx11"
            if (!bSupports_r4)
                bBreakLoop = true;
            break;
        default:
            ;
        }

        if (bBreakLoop) break;

        _tmp.emplace_back(nullptr);
        LPCSTR val;
        switch (i)
        {
        case 0:
            val = "renderer_r1";
            break;
        case 1:
            val = "renderer_r2a";
            break;
        case 2:
            val = "renderer_r2";
            break;
        case 3:
            val = "renderer_r2.5";
            break;
        case 4:
            val = "renderer_r3";
            break; // -)
        case 5:
            val = "renderer_r4";
            break; // -)
        default:
            NODEFAULT;
        }
        if (bBreakLoop) break;
        _tmp.back() = xr_strdup(val);
    }
    const u32 _cnt = static_cast<u32>(_tmp.size() + 1);
    vid_quality_token = xr_alloc<xr_token>(_cnt);

    vid_quality_token[_cnt - 1].id = -1;
    vid_quality_token[_cnt - 1].name = nullptr;

#ifdef DEBUG
    Msg("Available render modes[%d]:", _tmp.size());
#endif // DEBUG
    for (u32 i_render = 0; i_render < _tmp.size(); ++i_render)
    {
        vid_quality_token[i_render].id = static_cast<int>(i_render);
        vid_quality_token[i_render].name = _tmp[i_render];
#ifdef DEBUG
        Msg("[%s]", _tmp[i]);
#endif // DEBUG
    }
}