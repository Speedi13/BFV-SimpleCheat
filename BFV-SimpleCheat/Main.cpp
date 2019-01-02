#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h>
#include "FrostbiteSDK.h"
#include "Util.h"
#include "ESP.h"

typedef DWORD64 (__fastcall* t_fb__DxRenderer__endFrame)( fb::DxRenderer*, DWORD64 a2, bool presentEnable );
t_fb__DxRenderer__endFrame fb__DxRenderer__endFrame;

DWORD64 __fastcall DxRenderer__HkEndFrame( fb::DxRenderer* _this , DWORD64 a2, bool presentEnable )
{
	//call the esp render function
	g_pESP->Render( _this );
	//return original function
	return fb__DxRenderer__endFrame( _this, a2, presentEnable );
};

DWORD __stdcall SetupHook( HMODULE hOwnModule )
{
	AllocConsole();
	freopen("CONIN$", "r", stdin); 
	freopen("CONOUT$", "w", stdout); 
	freopen("CONOUT$", "w", stderr); 

	fb::DxRenderer* pDxRenderer = fb::DxRenderer::GetInstance();
	if (!ValidPointer(pDxRenderer))
	{
		printf("[!] DxRenderer is invalid!\n");
		return 1;
	}
	printf("[+] DxRenderer: 0x%I64X\n",pDxRenderer);
	fb::DxDisplaySettings *pDxDisplaySettings = pDxRenderer->getDxDisplaySettings();
	if (!ValidPointer(pDxDisplaySettings))
	{
		printf("[!] DxDisplaySettings is invalid!\n");
		return 1;
	}

	fb::Screen* pScreen = pDxRenderer->GetScreenByNumber(NULL);
	if (!ValidPointer(pScreen))
	{
		printf("[!] Screen is invalid!\n");
		return 1;
	}

	g_pESP = new ESP();

	printf("[+] Hooking fb::DxRenderer::endFrame");
	fb__DxRenderer__endFrame = (t_fb__DxRenderer__endFrame)HookVTableFunction( (DWORD64**)pDxRenderer, (BYTE*)&DxRenderer__HkEndFrame, 9 );
	printf(" [0x%I64X]\n",fb__DxRenderer__endFrame);

	return ERROR_SUCCESS;
};

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved //can be used for parameters
					 )
{
	UNREFERENCED_PARAMETER(lpReserved);

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread( NULL, NULL, (LPTHREAD_START_ROUTINE)SetupHook, hModule, NULL, NULL );
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}