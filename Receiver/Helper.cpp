#include "stdafx.h"
#include "Helper.h"
#include "App.h"
#include <TlHelp32.h>
#include <stdio.h>
#include <tchar.h>

CHelper::CHelper()
{
}


CHelper::~CHelper()
{
}

//获取程序根目录一个文件的绝对路径
void CHelper::GetDirFile(TCHAR* szPath, TCHAR* szFile, DWORD dwMaxLength)
{
	int len = GetModuleFileName(NULL, szPath, dwMaxLength);
	for (int i=len-1; i>=0; i--)
	{
		if ('\\' == szPath[i])
		{
			szPath[i+1] = '\0';
			break;
		}
	}
	_tcsncat(szPath, szFile, dwMaxLength-_tcslen(szPath));
}

//注入一个DLL
BOOL CHelper::InjectSingleDll(HANDLE hProcess, char *szDllName, LPDWORD lpExitCode)
{
	LPVOID	hPath;
	HANDLE	hThread;
	DWORD	dwTid;
	DWORD	dwSize;

	hPath = VirtualAllocEx(hProcess,NULL,strlen(szDllName)+1,MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (hPath == NULL) return FALSE;

	if (WriteProcessMemory(hProcess,hPath,szDllName,strlen(szDllName),&dwSize) == 0) return FALSE;

	hThread = CreateRemoteThread(hProcess,NULL,0,
	 	(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(_T("Kernel32.dll")), _T("LoadLibraryA")),
	  	hPath,	0,	&dwTid);

	WaitForSingleObject(hThread, INFINITE);
	GetExitCodeThread(hThread, lpExitCode);
	VirtualFree(hPath, strlen(szDllName)+1, MEM_RELEASE);
	return true;
}

//卸载
BOOL CHelper::FreeSingleDll(HANDLE hProcess, char *szDllName, LPDWORD lpExitCode)
{
	LPVOID	hPath;
	HANDLE	hThread;
	DWORD	dwTid;
	DWORD	dwSize;
	HMODULE hModule = 0;

	hPath = VirtualAllocEx(hProcess,NULL,strlen(szDllName)+1,MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (hPath == NULL) return FALSE;

	if (WriteProcessMemory(hProcess,hPath,szDllName,strlen(szDllName),&dwSize) == 0) return FALSE;

	hThread = CreateRemoteThread(hProcess,NULL,0,
	 	(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(_T("Kernel32.dll")), _T("GetModuleHandleA")),
	  	hPath,	0,	&dwTid);

	WaitForSingleObject(hThread, INFINITE);
	GetExitCodeThread(hThread, (LPDWORD)&hModule);
	VirtualFree(hPath, strlen(szDllName)+1, MEM_RELEASE);

	hThread = CreateRemoteThread(hProcess,NULL,0,
	 	(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(_T("Kernel32.dll")), _T("FreeLibrary")),
	  	hModule,0,	&dwTid);

	//WaitForSingleObject(hThread, INFINITE);
	//GetExitCodeThread(hThread, (LPDWORD)lpExitCode);

	return true;
}
