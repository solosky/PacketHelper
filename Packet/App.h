// App.h: interface for the CApp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_APP_H__47B1A415_DC71_4A0B_A74B_1326EC9D70AB__INCLUDED_)
#define AFX_APP_H__47B1A415_DC71_4A0B_A74B_1326EC9D70AB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Common.h"
#include "../Common/Net.h"
#include "Parser.h"
#include "Locker.h"

class CApp: public CSimpleNetHandler
{
public:
	CApp();
	virtual ~CApp();
	VOID Init();
	VOID Destroy();

	VOID Exit();
	
	static CApp* GetSingleton();
	static int WINAPI MyRecv(SOCKET sk,char* buff,int length,int flags);
	static int WINAPI MySend(SOCKET sk,char* buff, int length, int flags);
	static int WINAPI MyConnect(SOCKET s, sockaddr *name,int namelen);
	static int WINAPI MyClose(SOCKET s);
	static DWORD WINAPI SendMsgProc(PVOID pParam);

private:
	int ProcessSend(SOCKET sk,char* buff, int length, int flags);
	int ProcessRecv(SOCKET sk,char* buff,int length,int flags);
	VOID ProcessMessage(SOCKET sk, int eAction, PMESSAGE pMsg, DWORD dwExtra);
	/*INetHandler*/
public:
	virtual VOID OnClosed(INetConnection* piNetConnection);
	virtual VOID OnError(INetConnection* piNetConnection);
	virtual VOID OnPacket(INetConnection* piNetConnection, CPacket *pPacket);
	virtual VOID OnConnected(INetConnection * piNetConnection);

public:
	static CApp*		st_singleton;

	HINSTANCE			m_hIntance;
	INetConnection*		m_pNetConnection;
	CNetWorker*			m_pNetWorker;
	struct _HOOK_ENV*	m_pSendHookEnv;
	struct _HOOK_ENV*	m_pRecvHookEnv;
	struct _HOOK_ENV*	m_pConnectHookEnv;
	struct _HOOK_ENV*	m_pCloseHookEnv;
	BOOL				m_bIsHooked;
	FunSend				m_pRealSend;
	FunRecv				m_pRealRecv;
	FunConnect			m_pRealConnect;
	FunClose			m_pRealClose;
	CParser				m_SendParser;
	CParser				m_RecvParser;
	CLocker				m_SendLock;
	CLocker				m_MsgLock;
	list<PMSG_ENTRY>	m_MsgList;
	HANDLE				m_hMsgEvent;
	HANDLE				m_hSendThread;
	SOCKET				m_hNowSocket;
	HOOK_INFO			m_HookInfo;
	CPacket				m_SendPacket;
	
	

};

#endif // !defined(AFX_APP_H__47B1A415_DC71_4A0B_A74B_1326EC9D70AB__INCLUDED_)
