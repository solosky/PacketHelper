#pragma once
#include "stdafx.h"
#include "Common.h"

class CHelper
{
public:
	CHelper(void);
	~CHelper(void);

public:
	static void GetDirFile(TCHAR* szPath, TCHAR* szFile, DWORD dwMaxLength);
	static BOOL InjectSingleDll(HANDLE hProcess, char *szDllName, LPDWORD lpExitCode);
	static BOOL FreeSingleDll(HANDLE hProcess, char *szDllName, LPDWORD lpExitCode);
};

