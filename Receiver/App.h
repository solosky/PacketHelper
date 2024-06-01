// App.h: interface for the CApp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_APP_H__392AC1F3_4D6F_4708_93DD_71B874983D87__INCLUDED_)
#define AFX_APP_H__392AC1F3_4D6F_4708_93DD_71B874983D87__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../Common/Net.h"
#include "Common.h"

class CApp: public CSimpleNetHandler
{
public:
	CApp();
	virtual ~CApp();
	static CApp* GetSingleton();

public:
	static CApp*		st_singleton;
	CNetServer*			m_pNetServer;
	FnEventHandler		m_fnEventHandler;
	HINSTANCE			m_hIntance;
	INetConnection*		m_pCurrentConn;
	HOOK_INFO			m_HookInfo;

public:
	DWORD StartCapture(DWORD dwPid, DWORD dwHookType, DWORD dwAutoFixSeq);
	DWORD StopCapture(DWORD dwPid);
	DWORD SendMessage(PMESSAGE pMsg);

private:
	DWORD ToggleInjectDll(DWORD dwPid, BOOL bIsInject);
	VOID  InvokeEventHandler(DWORD dwEventType, DWORD dwParam1, DWORD dwParam2);

public:
	virtual VOID OnConnected(INetConnection* piNetConnection);
	virtual VOID OnError(INetConnection* piNetConnection);
	virtual VOID OnClosed(INetConnection* piNetConnection);
	virtual VOID OnPacket(INetConnection* piNetConnection, CPacket *pPacket);
	virtual VOID OnNew(INetConnection* piNetConnection);


};

#endif // !defined(AFX_APP_H__392AC1F3_4D6F_4708_93DD_71B874983D87__INCLUDED_)
