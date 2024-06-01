// Receiver.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "Receiver.h"
#include "App.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
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


VOID MY_API SetEventHandler(FnEventHandler pfEventHandler)
{
	CApp::GetSingleton()->m_fnEventHandler = pfEventHandler;
}

DWORD MY_API StartCapture(DWORD dwPid,DWORD dwHookType, DWORD dwAutoFixSeq)
{
	return CApp::GetSingleton()->StartCapture(dwPid, dwHookType, dwAutoFixSeq);
}

DWORD MY_API StopCapture(DWORD dwPid)
{
	return CApp::GetSingleton()->StopCapture(dwPid);
}

DWORD MY_API SendPacket(PMESSAGE pMsg)
{
	return CApp::GetSingleton()->SendMessage(pMsg);
}