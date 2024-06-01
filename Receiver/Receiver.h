
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the RECEIVER_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// RECEIVER_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef RECEIVER_EXPORTS
#define MY_API  __declspec(dllexport)
#else
#define MY_API  __declspec(dllimport)
#endif

#include "Common.h"
extern "C"
{
	VOID MY_API SetEventHandler(FnEventHandler pfEventHandler);
	DWORD MY_API StartCapture(DWORD dwPid, DWORD dwHookType, DWORD dwAutoFixSeq);
	DWORD MY_API StopCapture(DWORD dwPid);
	DWORD MY_API SendPacket(PMESSAGE pMsg);
}
