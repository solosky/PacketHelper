/************************************************************************
Inline HOOK API V1.1

thanks to ������Ӱ, xIkUg ,sucsor

by solosky <solosky772@qq.com>
created at 2011.06.29, updated at 2011.06.30  

----------------------------------
Inline HOOK API V1.1 2011.06.30
----------------------------------
1. �޸�û���ж�����ռ��Ƿ�ɹ���BUG��
2. �޸��ͷ��ڴ�Ĳ��������BUG��


----------------------------------
Inline HOOK API V1.0 2011.06.29
----------------------------------
1. HOOK�κκ���������֪��ԭ�ͺ͵��÷�ʽ���ɣ�����HOOK���󷽷�������Ҫ����ͻָ�thisָ�룩
2. UnHOOK�Ѿ�HOOK�ĺ���


��л������Ӱ�ṩ�ķ���೤�����棡��
************************************************************************/

/************************************************************************/
#include "StdAfx.h"
#include "inlinehook.h"
#include <windows.h>
#include <TLHELP32.H>
/************************************************************************/
/* InlineHOOKһ������
/* param ppRealFun		��HOOK�ĺ���ָ�룬ע���������ָ������ָ�룬��Ϊ��Ҫ�޸����ֵ���������µĺ�����ַ
/* param pHookFun		�Լ���д��HOOK����ָ��
/* param eHookType		HOOK�ķ�ʽ��������HOOK_TYPEö����
/* param bSuspendThreads�Ƿ��Զ������̣߳���ֹHOOK�Ĺ������н��̷��ʵ��³������
/* return PHOOK_ENV		�ɹ��󷵻�HOOK֮��Ļ��������ģ����������ָ��ȡ��HOOK��ʧ�ܷ���NULL 
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
	
	//���ȼ�����Ҫ�滻ԭʼ������ǰ�����ֽ�
	DWORD dwMinReplaceCodeSize = dwMinSize[eHookType];
	DWORD replaceCodeSize = FindOpcodeSize((BYTE*)dwRealFun);
	while (replaceCodeSize<dwMinReplaceCodeSize)
		replaceCodeSize += FindOpcodeSize((BYTE*)((DWORD)dwRealFun + (DWORD)replaceCodeSize));
	if (replaceCodeSize>16) return NULL;

	//����HOOK�Ļ������������STUB��һЩ����
	PHOOK_ENV pHookEnv = (PHOOK_ENV)VirtualAlloc(NULL,sizeof(HOOK_ENV),MEM_COMMIT,PAGE_EXECUTE_READWRITE);
	if(pHookEnv==NULL)	return NULL;
	
	memset(pHookEnv, sizeof(HOOK_ENV), 0);
	memset(pHookEnv->mHookStub.uJumpBack, 0x90, sizeof(pHookEnv->mHookStub.uJumpBack));	//������غ���ȥ�Ļ�����Ϊnop
	memset(pHookEnv->mHookStub.uJumpTo,  0x90, sizeof(pHookEnv->mHookStub.uJumpTo));

	//�ڻ��������ϱ���һЩ��Ϣ�����ڻָ�hook
	pHookEnv->dwReplacedBytes = replaceCodeSize;		//�滻���ֽڳ���
	pHookEnv->dwRealFunAddr = dwRealFun;				//ԭʼ������ַ
	pHookEnv->dwHookFunAddr = dwHookFun;				//HOOK������ַ
	
	//���STUB��ת��HOOK�����Ĵ���
	*(pHookEnv->mHookStub.uJumpTo) = 0xE9;			//JMPָ��
	*(DWORD*)(pHookEnv->mHookStub.uJumpTo+1) = dwHookFun-((DWORD)pHookEnv->mHookStub.uJumpTo)-5; //A B C => B=C-A-5

	//���STUB��ת��ԭ������ͷreplaceCodeSize���ֽڴ���
	memcpy(pHookEnv->mHookStub.uJumpBack, (PVOID)dwRealFun, replaceCodeSize);
	*(pHookEnv->mHookStub.uJumpBack+replaceCodeSize) = 0xE9;	//JMPָ�׼�����ص�ԭʼ����
	*(DWORD*)(pHookEnv->mHookStub.uJumpBack+replaceCodeSize+1) = (dwRealFun+replaceCodeSize)-((DWORD)(pHookEnv->mHookStub.uJumpBack)+replaceCodeSize)-5;
	
	//�ڸ���ԭʼ����֮ǰ�����ж��Ƿ���������߳�
	if(bSuspendThreads)
	{
		bSuspended = SuspendThreads(TRUE);
		if(!bSuspended) goto FAILED;
	}

	//�ؼ���ʱ�����ˣ�����HOOK�ķ�����ͬ���޸�ԭʼ������ǰ�����ֽڣ�ʹ��ԭ��������ת���Զ���ĺ�������
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

	//���ԭʼ��ַʣ����ֽ���
	if(replaceCodeSize>dwMinReplaceCodeSize) 
		memset((PVOID)(dwRealFun+dwMinReplaceCodeSize), 0x90, replaceCodeSize-dwMinReplaceCodeSize);

	//�ָ�ԭʼ�����Ķ�д����
	if (!VirtualProtect((PVOID)dwRealFun,replaceCodeSize,dwOldProctect,&dwOldProctect)) goto FAILED;

	//�޸���ʵ������ַΪSTUB������ָ�ʼ
	*(ppRealFun) = pHookEnv->mHookStub.uJumpBack;

	//�ָ��߳�
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
/* ȡ��HOOKһ������
/* param pHookEnv	HOOK�Ļ��������ģ���hookFun��������
/* param bSuspendThreads�Ƿ��Զ������̣߳���ֹHOOK�Ĺ������н��̷��ʵ��³������
/* return bool �ɹ�����TRUEʧ�ܷ���FALSE
/************************************************************************/
BOOL __stdcall UnHookFun(PHOOK_ENV pHookEnv, BOOL bSuspendThreads)
{
	DWORD dwOldProctect = 0;
	BOOL bSuspended = FALSE;

	//�ڸ���ԭʼ����֮ǰ�����ж��Ƿ���������߳�
	if(bSuspendThreads)
	{
		bSuspended = SuspendThreads(TRUE);
		if(!bSuspended) goto FAILED;
	}

	//�ָ���ʵ�����޸ĵ�ǰ�����ֽڼ���
	
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
/* ������߻ָ����˵�ǰ�̵߳������߳�
/* param IsSuspend TRUE�����̣߳�FALSE�ָ��߳�
/* return bool �ɹ�����TRUEʧ�ܷ���FALSE
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
/* ���Ҹ�����ַ�Ļ���ָ���  (��л������Ӱ�ṩ�ķ���೤������)
/* param intr0	ָ���ַ
/* return bool �ɹ����ص�ǰָ���
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
 ʹ�÷�����

ԭʼ��LoadLibraryA�������ǣ�

HMODULE WINAPI LoadLibraryA( LPCSTR lpLibFileName );

1. ��ô���ȶ���һ��hook��WINAPI������
typedef HMODULE (WINAPI* __pfnLoadLibraryA)(LPCTSTR lpFileName);

2. ����һ��ȫ�ֵı��������ڱ�����ʽ�ĺ�����ַ
__pfnLoadLibraryA real_loadLibrary;

3. �Զ����Լ���hook���������������������ʵ�ı�HOOK������ַ
HMODULE  my_loadLibrary(LPCTSTR lpFileName)
{
    char s[30];
    sprintf(s, "loading dll: %s", lpFileName);
    MessageBox(NULL, s, "MSG", MB_OK);
    real_loadLibrary(lpFileName);
}


4. HOOK�Ĺ���
    real_loadLibrary = LoadLibraryA;    //����ԭʼ��LoadLibrary
    PHOOK_ENV env = HookFun((PVOID*)&real_loadLibrary, (PVOID)my_loadLibrary);  //HOOK, ����PHOOK_ENV, ͬʱ����ʵ�ĺ�����ַ���浽��real_loadLibrary������
    LoadLibrary("test.dll");     //�������ã��Ѿ���HOOK
    UnHookFun(env);             //ȡ��HOOK
    LoadLibrary("test.dll");     //ûЧ��

*/
//////////////////////////////////////////////////////////////////////////////////////////////////

//�����������п�����Ϊ��ʾʹ��

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
//������Ҫ����һ����HOOK�ĺ���ԭ��
typedef DWORD (* pFun)(DWORD a, DWORD b);
//Ȼ����Ҫ����һ��ȫ�ֵĺ���ָ�룬������ʵ�ĺ�����ַ��Ҳ���Ǿ��������ĺ���������ڣ�
pFun real_add;


//����Ǳ�HOOK�ĺ���
DWORD add(DWORD a, DWORD b)
{
	printf("add: %d+%d=%d\n", a, b, a+b);
	return a+b;
}

//������Զ����HOOK����������ͨ��ȫ�ֵĺ���ָ�������ʵ�ĺ���
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
�����
 add: 10+6=16
my_add(mins): 10-6=4
add: 10+6=16
my_add(mins): 4-7=-3
add: 4+7=11
add: 10+6=16                                                              
*/


#endif
/////////////////////////////////////////////////////////////////////////

