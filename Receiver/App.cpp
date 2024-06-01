// App.cpp: implementation of the CApp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Common.h"
#include "App.h"
#include "Helper.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CApp* CApp::st_singleton = NULL;

CApp* CApp::GetSingleton()
{
	static CApp instance;
	st_singleton = &instance;
	return st_singleton;
}


CApp::CApp()
{
	m_pNetServer = new CNetServer(new CPipeAcceptor(PIPE_NAME), this);
	m_pNetServer->Start();
}

CApp::~CApp()
{
	m_pNetServer->Stop();
	INetAcceptor* pAcceptor = m_pNetServer->GetNetAcceptor();
	safe_delete(pAcceptor);
	safe_delete(m_pNetServer);
}


DWORD CApp::StartCapture(DWORD dwPid, DWORD dwHookType, DWORD dwAutoFixSeq)
{
	m_HookInfo.dwHookType = dwHookType;
	m_HookInfo.dwFixSequece = dwAutoFixSeq;
	return ToggleInjectDll(dwPid, TRUE);
}

DWORD CApp::StopCapture(DWORD dwPid)
{
	//return ToggleInjectDll(dwPid, FALSE);
	m_pCurrentConn->Close();
	m_pCurrentConn = NULL;	//Memory leaks...
	return RET_OK;
}

DWORD CApp::SendMessage(PMESSAGE pMsg)
{
	if(m_pCurrentConn!=NULL)
	{
		CPacket packet(CMD_SEND_MSG, MSG_LENGTH(pMsg), (BYTE*) pMsg);
		packet.Send(m_pCurrentConn);
		return RET_OK;
	}
	else
	{
		return RET_ERR;
	}

}

DWORD CApp::ToggleInjectDll(DWORD dwPid,BOOL bIsInject)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, dwPid);
	if(hProcess==INVALID_HANDLE_VALUE)
	{
		return RET_ERR;
	}

	CHAR szDllPath[MAX_PATH];
	DWORD dwExitCode = 0;
	CHelper::GetDirFile(szDllPath, "Packet.dll", MAX_PATH);

	BOOL bRet = FALSE;
	if (bIsInject)
	{
		bRet = CHelper::InjectSingleDll (hProcess, szDllPath, &dwExitCode);
		//Dbg("Inject Dll:path=%s, ret=%d", szDllPath, bRet);
	}
	else
	{
		bRet = CHelper::FreeSingleDll(hProcess, szDllPath, &dwExitCode);
		//Dbg("Free Dll:path=%s, ret=%d", szDllPath, bRet);
	}

	return bRet ? RET_OK : RET_ERR;
}


VOID CApp::OnConnected(INetConnection* piNetConnection)
{
	Dbg("Client Connected:");
}

VOID CApp::OnError(INetConnection* piNetConnection)
{
	Dbg("Connection Error:");
	InvokeEventHandler(EVENT_CONN_CLOSED, 0, 0);
	m_pCurrentConn = NULL;
}

VOID CApp::OnPacket(INetConnection* piNetConnection, CPacket *pPacket)
{
	switch(pPacket->GetCmd())
	{
	case CMD_GAME_MSG:
		{
			PMSG_ENTRY pMsgEntry = (PMSG_ENTRY) pPacket->GetData();
			PMESSAGE pMsg = &(pMsgEntry->Message);
			//Dbg("ON GAME MSG:ACTION=%d, CMD=0x%X, SEQ=0x%X, LEN=%hd",
			//	pMsgEntry->Connection.dwAction, pMsg->uCmd, pMsg->cSequece, MAKE_LENGTH(pMsg));
			InvokeEventHandler(EVENT_GAME_MSG, (DWORD)pPacket->GetData(), pPacket->GetLength());
		}
		break;
	default:
		Dbg("Recv a unknown packet:");
	}
}

VOID CApp::OnClosed(INetConnection* piNetConnection)
{
	Dbg("client disconnected:");
	m_pCurrentConn = NULL;
	InvokeEventHandler(EVENT_CONN_CLOSED, 0, 0);

}

VOID CApp::OnNew(INetConnection* piNetConnection)
{
	Dbg("Accept a new client:");
	m_pCurrentConn = piNetConnection;
	InvokeEventHandler(EVENT_CONN_NEW, 0, 0);

	// 发送开始HOOK的请求
	CPacket packet(CMD_HOOK_FUN, sizeof(HOOK_INFO), (BYTE*)&m_HookInfo);
	packet.Send(piNetConnection);
}


VOID CApp::InvokeEventHandler(DWORD dwEventType, DWORD dwParam1, DWORD dwParam2)
{
	if(m_fnEventHandler!=NULL)
	{
		Dbg("Invoke Event Handler:%d, %X, %d", dwEventType, dwParam1, dwParam2);
		m_fnEventHandler(dwEventType, dwParam1, dwParam2);
	}
}