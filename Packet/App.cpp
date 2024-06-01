// App.cpp: implementation of the CApp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "App.h"
#include "inlinehook.h"
#include "Locker.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CApp* CApp::st_singleton = NULL;

CApp::CApp()
{
	m_pNetConnection	= NULL;
	m_pNetWorker		= NULL;
	m_pSendHookEnv		= NULL;
	m_pRecvHookEnv		= NULL;
	m_pRealRecv			= NULL;
	m_pRealSend			= NULL;
	m_hNowSocket		= NULL;
	m_bIsHooked			= FALSE;
}

CApp::~CApp()
{
	Dbg("App::destructor...");
}

CApp* CApp::GetSingleton()
{
	static CApp instance;
	st_singleton = &instance;
	return st_singleton;
}


VOID CApp::Init()
{
	//初始化连接到抓包服务器
	m_pNetConnection = new CNioPipeConnection(PIPE_NAME);
	m_pNetWorker = new CNetWorker(m_pNetConnection, this, TRUE);
	m_pNetWorker->Start();

	//建立一个线程单独的发送数据
	DWORD dwThreadId = 0;
	m_hMsgEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hSendThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) CApp::SendMsgProc, 
		(LPVOID)this, 0, &dwThreadId);

	Dbg("App::Init.");
}

VOID CApp::Destroy()
{
	if(m_bIsHooked)
	{
		SuspendThreads(TRUE);
		UnHookFun(m_pRecvHookEnv, FALSE);
		UnHookFun(m_pSendHookEnv, FALSE);
		UnHookFun(m_pConnectHookEnv, FALSE);
		UnHookFun(m_pCloseHookEnv, FALSE);
		SuspendThreads(FALSE);
		m_bIsHooked = FALSE;
	}

	
	if(m_pNetWorker!=NULL && m_pNetWorker->IsExited()) 
	{
		m_pNetWorker->Stop();
	}

	safe_delete(m_pNetWorker);
	safe_delete(m_pNetConnection);
	CloseHandle(m_hMsgEvent);
	WaitForSingleObject(m_hSendThread, 1000);

	Dbg("App::Destory.");
}

VOID CApp::Exit()
{
	Dbg("App::Exit.");
	FreeLibraryAndExitThread(m_hIntance, 0);
}

VOID CApp::OnConnected(INetConnection* piNetConnection)
{
	Dbg("Connected to server. initializing...");
}


VOID CApp::OnError(INetConnection* piNetConnection)
{
	Dbg("Connection Error:");
	Exit();
}

VOID CApp::OnClosed(INetConnection* piNetConnection)
{
	Dbg("Connection Closed:");
	Exit();
}

VOID CApp::OnPacket(INetConnection* piNetConnection, CPacket *pPacket)
{
	switch(pPacket->GetCmd())
	{
	case CMD_SEND_MSG:
		{
			m_HookInfo.dwFixSequece = TRUE;	//强制修复序列号
			m_SendParser.SetFixSequece(m_HookInfo.dwFixSequece);
			m_RecvParser.SetFixSequece(m_HookInfo.dwFixSequece);
			m_SendParser.SetExtra(EXTRA_USER_MSG);
			
			MySend(m_hNowSocket, (char*)pPacket->GetData(), pPacket->GetLength(), 0);
			
			PMESSAGE pMsg = (PMESSAGE) pPacket->GetData();
			//Dbg("USER SEND MSG:CMD=0x%X, SEQ=0x%X, LEN=%hd", pMsg->uCmd, pMsg->cSequece, MAKE_LENGTH(pMsg));
		}
		break;
		
	case CMD_HOOK_FUN:
		{
			//HOOK发送和接收数据函数
			HMODULE hModule = LoadLibrary("ws2_32.dll");
			m_pRealSend = (FunSend) GetProcAddress(hModule, "send");
			m_pRealRecv = (FunRecv) GetProcAddress(hModule, "recv");
			m_pRealConnect = (FunConnect) GetProcAddress(hModule, "connect");
			m_pRealClose = (FunClose) GetProcAddress(hModule, "closesocket");
			PHOOK_INFO pHookInfo = (PHOOK_INFO)pPacket->GetData();
			m_HookInfo.dwHookType = pHookInfo->dwHookType;
			m_HookInfo.dwFixSequece = pHookInfo->dwFixSequece;
			DWORD dwHookType = pHookInfo->dwHookType;
			
			m_SendParser.SetFixSequece(pHookInfo->dwFixSequece);
			m_RecvParser.SetFixSequece(pHookInfo->dwFixSequece);
			//挂起除当前线程的所有线程，执行HOOK操作
			SuspendThreads(TRUE);
			m_pSendHookEnv = HookFun((PVOID*)& m_pRealSend, CApp::MySend, dwHookType, FALSE);
			m_pRecvHookEnv = HookFun((PVOID*)& m_pRealRecv, CApp::MyRecv, dwHookType, FALSE);
			m_pConnectHookEnv = HookFun((PVOID*)& m_pRealConnect, CApp::MyConnect, dwHookType, FALSE);
			m_pCloseHookEnv = HookFun((PVOID*)& m_pRealClose, CApp::MyClose, dwHookType, FALSE);
			m_bIsHooked = TRUE;
			SuspendThreads(FALSE);

			Dbg("HOOK:HookType=%d, m_pSendHookEnv=0x%X, m_pRecvHookEnv=0x%X", dwHookType, m_pSendHookEnv, m_pRecvHookEnv);
		}
		break;
	

	}
}

VOID CApp::ProcessMessage(SOCKET sk, int eAction, PMESSAGE pMsg, DWORD dwExtra)
{
	PMSG_ENTRY pEntry = (PMSG_ENTRY) malloc(sizeof(CONNECTION)+MSG_LENGTH(pMsg));
	if(pEntry==NULL)	return;
	
	memset(pEntry, 0, sizeof(CONNECTION));
	memcpy(&(pEntry->Message), pMsg, MSG_LENGTH(pMsg));
	
	sockaddr_in PeerAddr;
	int nNameLen = sizeof(PeerAddr);
	if(SOCKET_ERROR!=getpeername(sk, (struct sockaddr *)&PeerAddr, &nNameLen))
	{
		pEntry->Connection.dwAddr	= *((DWORD*)&PeerAddr.sin_addr);
		pEntry->Connection.uPort	= ntohs(PeerAddr.sin_port);
	}

	pEntry->Connection.dwAction = eAction;
	pEntry->Connection.dwPid	= GetCurrentProcessId();
	pEntry->Connection.dwSocket = sk;
	pEntry->Connection.dwExtra  = dwExtra; 

	m_MsgLock.Lock();
	m_MsgList.push_back(pEntry);
	SetEvent(m_hMsgEvent);
	m_MsgLock.Unlock();
}

int CApp::ProcessSend(SOCKET sk, char* buff, int length, int flags)
{
	//Dbg("===================send:socket=%d, length=%d===================", sk, length);
	m_hNowSocket = sk;		//
	PMESSAGE pMsg = NULL;
	CParser* pParser = &(CApp::GetSingleton()->m_SendParser);
	DWORD dwNextPos = 0;
	while((dwNextPos=pParser->Parse((BYTE*)buff, length, dwNextPos)) <= length)
	{
		if(pParser->HasMessage())
		{
			pMsg =pParser->GetMessage();
			Dbg("SEND MSG:CMD=0x%X, SEQ=0x%X, LEN=%hd", pMsg->uCmd, pMsg->cSequece, MAKE_LENGTH(pMsg));
			ProcessMessage(sk, ACTION_SEND, pMsg, pParser->GetExtra());
			pParser->ResetMessage();
		}

		if(dwNextPos==length)	break;
	}
		
	return length;
}

int CApp::ProcessRecv(SOCKET sk,char* buff,int length,int flags)
{
	Dbg("=======================recv:socket=%d, length=%d===============", sk, length);
	if(length>0)
	{
		CParser* pParser = &(CApp::GetSingleton()->m_RecvParser);
		PMESSAGE pMsg = NULL;
		DWORD dwNextPos = 0;
		while((dwNextPos=pParser->Parse((BYTE*)buff, length, dwNextPos)) <= length)
		{
			if(pParser->HasMessage())
			{
				pMsg = pParser->GetMessage();
				Dbg("recv MSG:CMD=0x%X, SEQ=0x%X, LEN=%hd", pMsg->uCmd, pMsg->cSequece, MAKE_LENGTH(pMsg));
				ProcessMessage(sk, ACTION_RECV, pMsg, pParser->GetExtra());
				pParser->ResetMessage();
			}

			if (dwNextPos==length)	break;
		}
	}
	
	return length;
}

DWORD WINAPI CApp::SendMsgProc(PVOID pParam)
{	
	Dbg("msg send thread is waiting.");
	CApp *pApp = CApp::GetSingleton();
	while(WaitForSingleObject(pApp->m_hMsgEvent, INFINITE)==WAIT_OBJECT_0)
	{
		//把当前列表中的所有的消息复制到一个临时队列里，避免迭代发送卡住send函数
		pApp->m_SendLock.Lock();
		list<PMSG_ENTRY> tmpList;
		if(pApp->m_MsgList.size()>0)
		{
			tmpList.insert(tmpList.end(), pApp->m_MsgList.begin(), pApp->m_MsgList.end());
			pApp->m_MsgList.clear();
		}
		pApp->m_SendLock.Unlock();
		//Dbg("msg send pending: %d MSGS.", tmpList.size());
		//遍历这个临时列表，发送所有的消息
		list<PMSG_ENTRY>::iterator it;
		for(it=tmpList.begin(); it!=tmpList.end(); it++)
		{
			PMSG_ENTRY pMsgEntry = *it;
			pApp->m_SendPacket.Reset();
			pApp->m_SendPacket.Set(CMD_GAME_MSG, sizeof(CONNECTION)+MSG_LENGTH_X(pMsgEntry->Message), (BYTE*) pMsgEntry);
			BOOL bRet = pApp->m_SendPacket.Send(pApp->m_pNetConnection);
			
			Dbg("send a msg to server: action=%d, socket=%d, CMD=%X, SEQ=%X, LENGTH=%d, ret=%d, err=%d", 
				pMsgEntry->Connection.dwAction, pMsgEntry->Connection.dwSocket,
				pMsgEntry->Message.uCmd, pMsgEntry->Message.cSequece, MAKE_LENGTH_X(pMsgEntry->Message), bRet, GetLastError());

			safe_free(pMsgEntry);
		}

		//Dbg("msg send done: %d MSGS.", tmpList.size());
		tmpList.clear();
	
	}
	Dbg("msg send thread died!!!");
	return 0;
}



int WINAPI CApp::MyRecv(SOCKET sk,char* buff,int length,int flags)
{
	CApp *pApp = CApp::GetSingleton();
	int nRecvLength = pApp->m_pRealRecv(sk, buff, length, flags);
	return pApp->ProcessRecv(sk, buff, nRecvLength, flags);
}

int WINAPI CApp::MySend(SOCKET sk, char* buff, int length, int flags)
{
	CApp *pApp = CApp::GetSingleton();
	if(pApp->m_HookInfo.dwFixSequece)
		CAutoLocker lock(pApp->m_SendLock);
	
	length = pApp->ProcessSend(sk, buff, length, flags);
	return pApp->m_pRealSend(sk, buff, length, flags);
}

int WINAPI CApp::MyConnect(SOCKET sk, sockaddr *name, int namelen)
{
	CApp *pApp = CApp::GetSingleton();
	MESSAGE msg;
	memset(&msg, 0, sizeof(MESSAGE));
	int ret = pApp->m_pRealConnect(sk, name, namelen);
	pApp->m_hNowSocket = sk;
	pApp->ProcessMessage(sk, ACTION_CONNECT, &msg, 0);
	pApp->m_SendParser.ResetSequece();
	pApp->m_RecvParser.ResetSequece();
	return ret;
}

int WINAPI CApp::MyClose(SOCKET sk)
{
	CApp *pApp = CApp::GetSingleton();
	MESSAGE msg;
	memset(&msg, 0, sizeof(msg));
	pApp->ProcessMessage(sk, ACTION_CLOSE, &msg, 0);
	pApp->m_SendParser.ResetSequece();
	pApp->m_RecvParser.ResetSequece();
	return pApp->m_pRealClose(sk);
}