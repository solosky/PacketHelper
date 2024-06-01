/************************************************************************
Inline HOOK API V1.1

thanks to 海风月影, xIkUg ,sucsor

by solosky <solosky772@qq.com>
created at 2011.06.29, updated at 2011.06.30  

----------------------------------
Inline HOOK API V1.1 2011.06.30
----------------------------------
1. 修复没有判断申请空间是否成功的BUG；
2. 修复释放内存的参数错误的BUG；


----------------------------------
Inline HOOK API V1.0 2011.06.29
----------------------------------
1. HOOK任何函数，仅需知道原型和调用方式即可（可以HOOK对象方法，但需要保存和恢复this指针）
2. UnHOOK已经HOOK的函数


感谢海风月影提供的反汇编长度引擎！！
************************************************************************/

/************************************************************************/
#include "StdAfx.h"
#include "inlinehook.h"
#include <windows.h>
#include <TLHELP32.H>
/************************************************************************/
/* InlineHOOK一个函数
/* param ppRealFun		被HOOK的函数指针，注意这里必须指向函数的指针，因为需要修改这个值，来返回新的函数地址
/* param pHookFun		自己编写的HOOK函数指针
/* param eHookType		HOOK的方式，定义在HOOK_TYPE枚举中
/* param bSuspendThreads是否自动挂起线程，防止HOOK的过程中有进程访问导致程序错误
/* return PHOOK_ENV		成功后返回HOOK之后的环境上下文，可以用这个指针取消HOOK，失败返回NULL 
/************************************************************************/
PHOOK_ENV __stdcall HookFun(PVOID *ppRealFun,const PVOID pHookFun, int eHookType, BOOL bSuspendThreads)
{
	DWORD dwRealFun = *(DWORD*)ppRealFun;
	DWORD dwHookFun = (DWORD) pHookFun;
	DWORD dwMinSize[] = {5, 6, 7};
	BOOL  bSuspended = FALSE;
	DWORD dwOldProctect = 0;
	DWORD dwHookOpCodeLen = 0;

	if(eHookType<0 || eHookType>2)	return NULL;
	
	//首先计算需要替换原始函数的前几个字节
	DWORD dwMinReplaceCodeSize = dwMinSize[eHookType];
	DWORD replaceCodeSize = FindOpcodeSize((BYTE*)dwRealFun);
	while (replaceCodeSize<dwMinReplaceCodeSize)
		replaceCodeSize += FindOpcodeSize((BYTE*)((DWORD)dwRealFun + (DWORD)replaceCodeSize));
	if (replaceCodeSize>16) return NULL;

	//分配HOOK的环境变量，存放STUB和一些数据
	PHOOK_ENV pHookEnv = (PHOOK_ENV)VirtualAlloc(NULL,sizeof(HOOK_ENV),MEM_COMMIT,PAGE_EXECUTE_READWRITE);
	if(pHookEnv==NULL)	return NULL;
	
	memset(pHookEnv, sizeof(HOOK_ENV), 0);
	memset(pHookEnv->mHookStub.uJumpBack, 0x90, sizeof(pHookEnv->mHookStub.uJumpBack));	//填充跳回和跳去的机器码为nop
	memset(pHookEnv->mHookStub.uJumpTo,  0x90, sizeof(pHookEnv->mHookStub.uJumpTo));

	//在环境变量上保存一些信息，便于恢复hook
	pHookEnv->dwReplacedBytes = replaceCodeSize;		//替换的字节长度
	pHookEnv->dwRealFunAddr = dwRealFun;				//原始函数地址
	pHookEnv->dwHookFunAddr = dwHookFun;				//HOOK函数地址
	
	//填充STUB跳转到HOOK函数的代码
	*(pHookEnv->mHookStub.uJumpTo) = 0xE9;			//JMP指令
	*(DWORD*)(pHookEnv->mHookStub.uJumpTo+1) = dwHookFun-((DWORD)pHookEnv->mHookStub.uJumpTo)-5; //A B C => B=C-A-5

	//填充STUB跳转回原函数的头replaceCodeSize个字节代码
	memcpy(pHookEnv->mHookStub.uJumpBack, (PVOID)dwRealFun, replaceCodeSize);
	*(pHookEnv->mHookStub.uJumpBack+replaceCodeSize) = 0xE9;	//JMP指令，准备跳回到原始函数
	*(DWORD*)(pHookEnv->mHookStub.uJumpBack+replaceCodeSize+1) = (dwRealFun+replaceCodeSize)-((DWORD)(pHookEnv->mHookStub.uJumpBack)+replaceCodeSize)-5;
	
	//在更改原始代码之前，先判断是否挂起所有线程
	if(bSuspendThreads)
	{
		bSuspended = SuspendThreads(TRUE);
		if(!bSuspended) goto FAILED;
	}

	//关键的时候来了，根据HOOK的方法不同，修改原始函数的前几个字节，使得原函数能跳转到自定义的函数中来
	if (!VirtualProtect((PVOID)dwRealFun, replaceCodeSize,PAGE_EXECUTE_READWRITE,&dwOldProctect)) goto FAILED;
	if(eHookType==HEAD_JMP_HOOK)
	{
		*(UCHAR*)(dwRealFun) = 0xE9; //JMP
		*(DWORD*)(dwRealFun+1) = (DWORD)(pHookEnv->mHookStub.uJumpTo)-dwRealFun-5;
	}
	else if(eHookType==PUSH_RET_HOOK)
	{
		*(UCHAR*)(dwRealFun) = 0x68;	//PUSH
		*(DWORD*)(dwRealFun+1) = (DWORD)(pHookEnv->mHookStub.uJumpTo);
		*(UCHAR*)(dwRealFun+5) = 0xC3;	//RET
	}
	else if(eHookType==EAX_JMP_HOOK)
	{
		*(UCHAR*)(dwRealFun) = 0xB8;  //MOV EAX,x
		*(DWORD*)(dwRealFun+1) = (DWORD)(pHookEnv->mHookStub.uJumpTo);
		*(WORD*)(dwRealFun+5) = 0xE0FF;	//JMP EAX
	}
	else
	{
		goto FAILED;
	}

	//填充原始地址剩余的字节数
	if(replaceCodeSize>dwMinReplaceCodeSize) 
		memset((PVOID)(dwRealFun+dwMinReplaceCodeSize), 0x90, replaceCodeSize-dwMinReplaceCodeSize);

	//恢复原始函数的读写属性
	if (!VirtualProtect((PVOID)dwRealFun,replaceCodeSize,dwOldProctect,&dwOldProctect)) goto FAILED;

	//修改真实函数地址为STUB的跳回指令开始
	*(ppRealFun) = pHookEnv->mHookStub.uJumpBack;

	//恢复线程
	if(bSuspended)
		SuspendThreads(FALSE);

	return pHookEnv;

FAILED:
		if(bSuspended)
			SuspendThreads(FALSE);
		if(pHookEnv) 
			VirtualFree(pHookEnv, 0, MEM_RELEASE);
		return NULL;
}

/************************************************************************/
/* 取消HOOK一个函数
/* param pHookEnv	HOOK的环境上下文，由hookFun函数返回
/* param bSuspendThreads是否自动挂起线程，防止HOOK的过程中有进程访问导致程序错误
/* return bool 成功返回TRUE失败返回FALSE
/************************************************************************/
BOOL __stdcall UnHookFun(PHOOK_ENV pHookEnv, BOOL bSuspendThreads)
{
	DWORD dwOldProctect = 0;
	BOOL bSuspended = FALSE;

	//在更改原始代码之前，先判断是否挂起所有线程
	if(bSuspendThreads)
	{
		bSuspended = SuspendThreads(TRUE);
		if(!bSuspended) goto FAILED;
	}

	//恢复真实函数修改的前几个字节即可
	
	if (!VirtualProtect((PVOID)pHookEnv->dwRealFunAddr, pHookEnv->dwReplacedBytes, PAGE_EXECUTE_READWRITE,&dwOldProctect)) goto FAILED;
	memcpy((PVOID)pHookEnv->dwRealFunAddr, (PVOID)pHookEnv->mHookStub.uJumpBack, pHookEnv->dwReplacedBytes);
	if (!VirtualProtect((PVOID)pHookEnv->dwRealFunAddr, pHookEnv->dwReplacedBytes,dwOldProctect,&dwOldProctect)) goto FAILED;
	if(!VirtualFree(pHookEnv, 0, MEM_RELEASE)) goto FAILED;
	return TRUE;

FAILED:
	if(bSuspended)
		SuspendThread(FALSE);
	return FALSE;

}

/************************************************************************/
/* 挂起或者恢复除了当前线程的所有线程
/* param IsSuspend TRUE挂起线程，FALSE恢复线程
/* return bool 成功返回TRUE失败返回FALSE
/************************************************************************/
BOOL __stdcall SuspendThreads(BOOL IsSuspend)
{
	DWORD dwCurrentPid = GetCurrentProcessId();
	DWORD dwCurrentTid = GetCurrentThreadId();

	FnOpenThread fpOpenThread = (FnOpenThread) GetProcAddress(GetModuleHandle("Kernel32.dll"), "OpenThread");
	if(fpOpenThread==NULL)
	{
		return FALSE;
	}

	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	THREADENTRY32 te32;
	te32.dwSize = sizeof(THREADENTRY32); 

	if(!Thread32First(hThreadSnap, &te32)) 
	{
		CloseHandle(hThreadSnap);
		return(FALSE);
	}
	do 
	{ 
		if(te32.th32OwnerProcessID==dwCurrentPid && te32.th32ThreadID!=dwCurrentTid )
		{
			HANDLE hThread = fpOpenThread(THREAD_SUSPEND_RESUME, TRUE, te32.th32ThreadID);
			if(hThread!=INVALID_HANDLE_VALUE)
			{
				if(IsSuspend)
					SuspendThread(hThread);
				else
					ResumeThread(hThread);

				CloseHandle(hThread);
			}

			
		}
	} while(Thread32Next(hThreadSnap, &te32));

	CloseHandle(hThreadSnap);
	return(TRUE);
}

/************************************************************************/
/* 查找给定地址的机器指令长度  (感谢海风月影提供的反汇编长度引擎)
/* param intr0	指令地址
/* return bool 成功返回当前指令长度
/************************************************************************/
DWORD __stdcall FindOpcodeSize(BYTE* iptr0)
{
	BYTE* iptr = iptr0;
	DWORD f = 0;
prefix:
	BYTE b = *iptr++;
	f |= table_1[b];

	if (f&C_FUCKINGTEST)
		if (((*iptr)&0x38)==0x00)   // ttt
			f=C_MODRM+C_DATAW0;       // TEST
		else
			f=C_MODRM;                // NOT,NEG,MUL,IMUL,DIV,IDIV

	if (f&C_TABLE_0F)
	{
		b = *iptr++;
		f = table_0F[b];
	}

	if (f==C_ERROR)
	{
		//printf("error in %02X\n",b);
		return C_ERROR;
	}

	if (f&C_PREFIX)
	{
		f&=~C_PREFIX;
		goto prefix;
	}

	if (f&C_DATAW0) if (b&0x01) f|=C_DATA66; else f|=C_DATA1;

	if (f&C_MODRM)
	{
		b = *iptr++;
		BYTE mod = b & 0xC0;
		BYTE rm  = b & 0x07;
		if (mod!=0xC0)
		{
			if (f&C_67)         // modrm16
			{
				if ((mod==0x00)&&(rm==0x06)) f|=C_MEM2;
				if (mod==0x40) f|=C_MEM1;
				if (mod==0x80) f|=C_MEM2;
			}
			else                // modrm32
			{
				if (mod==0x40) f|=C_MEM1;
				if (mod==0x80) f|=C_MEM4;
				if (rm==0x04) rm = (*iptr++) & 0x07;    // rm<-sib.base
				if ((rm==0x05)&&(mod==0x00)) f|=C_MEM4;
			}
		}
	} // C_MODRM

	if (f&C_MEM67)  if (f&C_67) f|=C_MEM2;  else f|=C_MEM4;
	if (f&C_DATA66) if (f&C_66) f|=C_DATA2; else f|=C_DATA4;

	if (f&C_MEM1)  iptr++;
	if (f&C_MEM2)  iptr+=2;
	if (f&C_MEM4)  iptr+=4;

	if (f&C_DATA1) iptr++;
	if (f&C_DATA2) iptr+=2;
	if (f&C_DATA4) iptr+=4;

	return iptr - iptr0;
}


/************************************************************************
 使用方法：

原始的LoadLibraryA的声明是：

HMODULE WINAPI LoadLibraryA( LPCSTR lpLibFileName );

1. 那么首先定义一下hook的WINAPI的类型
typedef HMODULE (WINAPI* __pfnLoadLibraryA)(LPCTSTR lpFileName);

2. 定义一个全局的变量，用于保存正式的函数地址
__pfnLoadLibraryA real_loadLibrary;

3. 自定义自己的hook函数，可以在里面调用真实的被HOOK函数地址
HMODULE  my_loadLibrary(LPCTSTR lpFileName)
{
    char s[30];
    sprintf(s, "loading dll: %s", lpFileName);
    MessageBox(NULL, s, "MSG", MB_OK);
    real_loadLibrary(lpFileName);
}


4. HOOK的过程
    real_loadLibrary = LoadLibraryA;    //保存原始的LoadLibrary
    PHOOK_ENV env = HookFun((PVOID*)&real_loadLibrary, (PVOID)my_loadLibrary);  //HOOK, 返回PHOOK_ENV, 同时把真实的函数地址保存到了real_loadLibrary变量中
    LoadLibrary("test.dll");     //正常调用，已经被HOOK
    UnHookFun(env);             //取消HOOK
    LoadLibrary("test.dll");     //没效果

*/
//////////////////////////////////////////////////////////////////////////////////////////////////

//定义下面这行可以作为演示使用

//#define TEST_MAIN1
#define TEST_MAIN2

#ifdef TEST_MAIN1
typedef HMODULE (WINAPI* __pfnLoadLibraryA)(LPCTSTR lpFileName);
__pfnLoadLibraryA real_loadLibrary;

HMODULE WINAPI my_loadLibrary(LPCTSTR lpFileName)
{
    char s[30];
    sprintf(s, "loading dll: %s", lpFileName);
    MessageBox(NULL, s, "MSG", MB_OK);
    return real_loadLibrary(lpFileName);
}

int main()
{
    real_loadLibrary = LoadLibraryA;   
    PHOOK_ENV env = HookFun((PVOID*)&real_loadLibrary, (PVOID)my_loadLibrary);  
    LoadLibrary("test.dll");   
    UnHookFun(env);      
    LoadLibrary("test.dll");    
	
	return 1;
}
#endif


#ifdef TEST_MAIN2
//首先需要定义一个被HOOK的函数原型
typedef DWORD (* pFun)(DWORD a, DWORD b);
//然后需要定义一个全局的函数指针，保存真实的函数地址（也就是经过处理后的函数的新入口）
pFun real_add;


//这个是被HOOK的函数
DWORD add(DWORD a, DWORD b)
{
	printf("add: %d+%d=%d\n", a, b, a+b);
	return a+b;
}

//这个是自定义的HOOK函数，可以通过全局的函数指针调用真实的函数
 DWORD my_add(DWORD a, DWORD b)
{
	printf("my_add(mins): %d-%d=%d\n",a, b, a-b);
	return real_add(a, b); 
}

int main(int argc, char **argv)
{
	DWORD a = 10;
	DWORD b = 6;
	add(a, b);

	
	real_add = add;

	 PHOOK_ENV env = HookFun((PVOID*)&real_add, (PVOID)my_add, HEAD_JMP_HOOK);  

	add(a, b);
	add(4, 7);

	UnHookFun(env);
	add(a, b);
	return 1;
}

/*
输出：
 add: 10+6=16
my_add(mins): 10-6=4
add: 10+6=16
my_add(mins): 4-7=-3
add: 4+7=11
add: 10+6=16                                                              
*/


#endif
/////////////////////////////////////////////////////////////////////////

