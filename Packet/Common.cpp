
#include "StdAfx.h"
#include "Common.h"
#include <time.h>

//�� DbgView.exe���������Ϣ
VOID Dbg(const char* szFormat, ...)
{
	va_list	  arg; 
	char szTemp[1024];
	//time_t rawTime;
	//struct tm * timeInfo;

	//time(&rawTime);
	//timeInfo = localtime(&rawTime);
	//strftime (szTemp, sizeof(szTemp),"[%H:%M:%S] ",timeInfo);

	va_start(arg, szFormat);
	vsprintf(szTemp, szFormat, arg); 
	va_end(arg); 
	OutputDebugString(szTemp);
}
