
#include "StdAfx.h"
#include "Common.h"
#include <time.h>

//用 DbgView.exe输出调试信息
VOID Dbg(const char* szFormat, ...)
{
	va_list	  arg; 
	char szTemp[1024];

	va_start(arg, szFormat);
	vsprintf(szTemp, szFormat, arg); 
	va_end(arg); 
	OutputDebugString(szTemp);
}





















