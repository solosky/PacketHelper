// Packet.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "App.h"

BOOL WINAPI DllMain(HANDLE hinstDLL,DWORD dwReason,LPVOID lpvReserved)
{
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
			CApp::GetSingleton()->m_hIntance = (HINSTANCE) hinstDLL;
			CApp::GetSingleton()->Init();
				break;

		case DLL_THREAD_ATTACH:
				break;

		case DLL_THREAD_DETACH:
				break;

		case DLL_PROCESS_DETACH:
			CApp::GetSingleton()->Destroy();
				break;

		default:
				break;
	}
	return TRUE;
}
