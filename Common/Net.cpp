
#include "stdafx.h"
#include "Net.h"
#include <tchar.h>

//用 DbgView.exe输出调试信息
VOID NET_DBG(const char* szFormat, ...)
{
	va_list	  arg; 
	char szTemp[1024];
	va_start(arg, szFormat);
	vsprintf(szTemp, szFormat, arg); 
	va_end(arg); 
	OutputDebugString(szTemp);
}


///////////////////CPacket///////////////////
CPacket::CPacket()
{
	m_dwPosition = 0;
	m_dwCapcity  = PACKET_DEFAULT_CAPCITY;
	m_pData    = (BYTE*) malloc(PACKET_DEFAULT_CAPCITY);
	//ASSERT(m_pData!=NULL);
}

CPacket::CPacket(DWORD dwCmd, DWORD dwLength, BYTE* pData)
{
	m_pData      = NULL;
	m_dwCapcity  = 0;
	m_dwPosition = 0;
	CheckCapcity(PACKET_HEADER_LEN + dwLength);
	
	DWORD* ptr = (DWORD*) m_pData;
	ptr[0] = dwCmd;
	ptr[1] = dwLength;

	if(dwLength>0)
	{
		memcpy(m_pData+PACKET_HEADER_LEN, pData, dwLength);
	}
	m_dwPosition    = dwLength+PACKET_HEADER_LEN;
}

CPacket::~CPacket()
{
	free(m_pData);
}

VOID CPacket::Reset()
{
	m_dwPosition = 0;
}

DWORD CPacket::GetCmd()
{
	DWORD* ptr = (DWORD*) m_pData;
	return ptr[0];
}

DWORD CPacket::GetLength()
{
	DWORD* ptr = (DWORD*) m_pData;
	return ptr[1];
}

BOOL CPacket::IsReady()
{
	return m_dwPosition>0 && ((GetLength() + PACKET_HEADER_LEN) == m_dwPosition);
}

BYTE* CPacket::GetData()
{
	return m_pData + PACKET_HEADER_LEN;
}

VOID CPacket::Set(DWORD dwCmd, DWORD dwLength, BYTE* pData)
{
	m_dwPosition = 0;
	CheckCapcity(PACKET_HEADER_LEN + dwLength);
	
	DWORD* ptr = (DWORD*) m_pData;
	ptr[0] = dwCmd;
	ptr[1] = dwLength;

	if(dwLength>0)
	{
		memcpy(m_pData+PACKET_HEADER_LEN, pData, dwLength);
	}
	m_dwPosition    = dwLength+PACKET_HEADER_LEN;
}


DWORD CPacket::Parse(BYTE* pData, DWORD dwSize, DWORD dwNextPos)
{
	//NET_DBG("========================================================================================");
	//NET_DBG("Packet::Parse Start: m_dwCapcity=%d, m_dwPosition=%d, DataSize=%d, NextPos=%d, IsReady=%d", 
	//	m_dwCapcity, m_dwPosition, dwSize, dwNextPos, IsReady());
	
	DWORD dwRemainSize = dwSize - dwNextPos;
	BYTE* ptrIn        = pData+dwNextPos;
	BYTE* ptrOut       = m_pData+m_dwPosition;
	DWORD dwReadSize   = 0;

	//缓冲区不够封包头部大小， 尝试读满封包头部信息
	if(m_dwPosition<PACKET_HEADER_LEN)
	{
		dwReadSize = min(PACKET_HEADER_LEN-m_dwPosition, dwRemainSize);
		memcpy(ptrOut, ptrIn, dwReadSize);

		dwRemainSize -= dwReadSize;
		m_dwPosition += dwReadSize;
		ptrOut       += dwReadSize;
		ptrIn        += dwReadSize;
		dwNextPos    += dwReadSize;

		//是否还有数据读取
		if(dwRemainSize==0)	return dwNextPos;
	}

	//如果上面没有返回，表明已经成功的读取了一个命令和长度信息
	
	DWORD dwDataSize = GetLength();		//包数据长度
	dwReadSize       = min(dwDataSize-(m_dwPosition-PACKET_HEADER_LEN), dwRemainSize);	//应该读取的数据长度
	//NET_DBG("Packet::Recv A Header, Cmd=%d, Length=%d, dw_position=%d, dwReadSize=%d",
	//	GetCmd(), GetLength(), m_dwPosition, dwReadSize);

	//判断缓冲区空间是否可以足够，如果不够重新分配
	CheckCapcity(dwDataSize+PACKET_HEADER_LEN);

	ptrIn        = pData+dwNextPos;
	ptrOut       = m_pData+m_dwPosition;
	
	//复制数据
	memcpy(ptrOut, ptrIn, dwReadSize);

	dwRemainSize -= dwReadSize;
	m_dwPosition += dwReadSize;
	ptrOut       += dwReadSize;
	ptrIn        += dwReadSize;
	dwNextPos    += dwReadSize;

	//NET_DBG("Packet::Parse End: m_dwCapcity=%d, m_dwPosition=%d, DataSize=%d, NextPos=%d, DwLength=%d, IsReady=%d", 
	//	m_dwCapcity, m_dwPosition, dwSize, dwNextPos,GetLength(), IsReady());

	return dwNextPos;
}

BOOL CPacket::CheckCapcity(DWORD dwDataSize)
{
	//判断缓冲区大小是否可以放下这个包，如果不可以，重新分配缓冲区
	if(m_dwCapcity<dwDataSize)
	{
		DWORD dwNewCapcity = m_dwCapcity;
		while(dwNewCapcity<dwDataSize)
			dwNewCapcity += PACKET_DEFAULT_CAPCITY;

		BYTE* pNewData = (BYTE*)malloc(dwNewCapcity);
		if(pNewData==NULL)
		{
			return FALSE;
		}

		if(m_pData!=NULL)
		{
			memcpy(pNewData, m_pData, m_dwPosition);	//复制数据到新的缓冲区
			free(m_pData);
		}

		m_pData     = pNewData;
		m_dwCapcity = dwNewCapcity;

		//NET_DBG("Packet::Expand Buffer: %d", m_dwCapcity);
	}
	return TRUE;
}

BOOL CPacket::Send(INetConnection* piNetConnection)
{
	return piNetConnection->Send(m_pData, m_dwPosition);
}


///////////////////////////CSimpleNetHandler///////////////////////////
/*一个空的INetHandler的实现，默认什么也不做*/
VOID CSimpleNetHandler::OnConnected(INetConnection* piNetConnection)
{
}
VOID CSimpleNetHandler::OnError(INetConnection* piNetConnection)
{
}
VOID CSimpleNetHandler::OnClosed(INetConnection* piNetConnection)
{
}
VOID CSimpleNetHandler::OnPacket(INetConnection* piNetConnection, CPacket *pPacket)
{
}
VOID CSimpleNetHandler::OnNew(INetConnection* piNetConnection)
{
}
///////////////////////////////////////////////////////////////////////////////////////

CNetWorker::CNetWorker(INetConnection* piNetConnection, INetHandler* piNetHandler, BOOL bNeedConnect)
{
	m_piNetConnection = piNetConnection;
	m_piNetHandler = piNetHandler;
	m_bNeedConnect = bNeedConnect;
	m_dwExitCode = 0;
	m_bExited = FALSE;
	m_hThread = INVALID_HANDLE_VALUE;
}

CNetWorker::~CNetWorker()
{
	if(!m_bExited)
	{
		Stop(-3);		//强制结束
	}
}

BOOL CNetWorker::Start()
{
	m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) CNetWorker::WorkProc, (LPVOID)this, 0, NULL);
	return m_hThread!=INVALID_HANDLE_VALUE;
}

VOID CNetWorker::Stop(DWORD dwExitCode)
{
	if(!m_bExited && m_hThread!=INVALID_HANDLE_VALUE )
	{
		m_piNetConnection->Close();
		WaitForSingleObject(m_hThread, INFINITE);
		m_dwExitCode = dwExitCode;
		m_hThread = INVALID_HANDLE_VALUE;
		m_bExited = TRUE;
		//TerminateThread(m_hThread, -1);
	}
}

DWORD CNetWorker::Work()
{
	if(m_bNeedConnect)	//需要连接，执行连接操作
	{
		if(!m_piNetConnection->Connect())
		{
			m_bExited = TRUE;
			m_dwExitCode = -2;	//TODO ..定义为常量
			m_piNetHandler->OnError(m_piNetConnection);
			return m_dwExitCode;
		}
		else
		{
			m_piNetHandler->OnConnected(m_piNetConnection);
		}
	}

	//循环读取数据，直到出错为止
	while(!m_bExited)
	{
		INT nReaded = m_piNetConnection->Recv(m_biBuffer, BUFFER_SIZE);
		if(nReaded<0)		//读取失败
		{
			m_bExited = TRUE;
			m_dwExitCode = -1;
		}
		else	
		{
			//读取成功，循环解析数据，直到缓冲区的数据读取完毕
			DWORD dwNextPos = 0;
			while((dwNextPos = m_pkPacket.Parse(m_biBuffer, nReaded, dwNextPos))<=nReaded)
			{
				//判断是否读取了一个完整的包
				if(m_pkPacket.IsReady())
				{
					m_piNetHandler->OnPacket(m_piNetConnection, &m_pkPacket);	//处理这个数据包
					m_pkPacket.Reset();		//重置，便于下一次读取
				}
				if(dwNextPos==nReaded)	break;	// 已经读取完毕
			}
		}
	}
	m_piNetHandler->OnClosed(m_piNetConnection);
	NET_DBG("CNetWorker::Work() done.");
	return m_dwExitCode;
}

DWORD WINAPI CNetWorker::WorkProc(LPVOID lpParam)
{ 
	 CNetWorker* pWorker = (CNetWorker*)lpParam; 
	 return pWorker->Work(); 
}


////////////////////////////////////////CNetServer////////////////////////////////////
CNetServer::CNetServer(INetAcceptor* piNetAcceptor, INetHandler* piNetHandler)
{
	m_piNetAcceptor = piNetAcceptor;
	m_piNetHandler = piNetHandler;
	m_hAcceptorThread = INVALID_HANDLE_VALUE;
}

CNetServer::~CNetServer()
{
	if(m_hAcceptorThread!=INVALID_HANDLE_VALUE)
	{
		Stop();
		m_hAcceptorThread = INVALID_HANDLE_VALUE;
	}
}

BOOL CNetServer::Start()
{
	if(m_piNetAcceptor->Listen())
	{
		m_hAcceptorThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) CNetServer::AcceptProc, (LPVOID)this, 0, NULL);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

VOID CNetServer::Stop()
{
	m_piNetAcceptor->Stop();	//调用这个方法后，Acceptor应该立即返回，线程应该自动结束，所以不需要强制结束线程
	m_hAcceptorThread =  INVALID_HANDLE_VALUE;
}

DWORD CNetServer::Accepting()
{
	INetConnection* piNetConnection = NULL;
	CNetWorker* pNetWorker = NULL;
	//循环监听连接
	while((piNetConnection = m_piNetAcceptor->Accept())!=NULL)
	{
		pNetWorker = new CNetWorker(piNetConnection, m_piNetHandler, FALSE);
		if(pNetWorker!=NULL)
		{
			m_vtrWorkers.push_back(pNetWorker);
			m_piNetHandler->OnNew(piNetConnection);
			pNetWorker->Start();
		}

		//清空已经完成的线程，仅在接收到连接后调用，不是最好的解决方法  
		//这段代码有问题，改天调试
		/*vector<CNetWorker*>::iterator it;
		for(it=m_vtrWorkers.begin();it!=m_vtrWorkers.end(); it++)
		{
			pNetWorker = *it;
			if(pNetWorker->IsExited())
			{
				m_vtrWorkers.erase(it);
				delete pNetWorker;
			}
		}*/
	}

	return 0;
}

DWORD WINAPI CNetServer::AcceptProc(LPVOID lpParam)
{
	CNetServer* pNetServer = (CNetServer*) lpParam;
	return pNetServer->Accepting();
}

////////////////////////CTcpAcceptor////////////////////////////////////
CTcpAcceptor::CTcpAcceptor(SHORT uPort)
{
	m_uPort = uPort;
}

CTcpAcceptor::~CTcpAcceptor()
{
}

BOOL CTcpAcceptor::Listen()
{
	WSADATA  ws;
	struct sockaddr_in addrLocal;
	DWORD dwAddrLen = 0;

	 //Init Windows Socket
	if ( WSAStartup(MAKEWORD(2,2), &ws) != 0 ) return FALSE;

	//Create Socket
	m_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ( m_serverSocket == INVALID_SOCKET ) return FALSE;

	addrLocal.sin_family = AF_INET;
    addrLocal.sin_addr.s_addr = INADDR_ANY;
	addrLocal.sin_port = htons(m_uPort);
    memset(addrLocal.sin_zero, 0x00, 8);

    //Bind Socket
    if(bind(m_serverSocket, (sockaddr*)&addrLocal, sizeof(addrLocal))!=0) return FALSE;

	//Listen
	if(listen(m_serverSocket, 100)!=0) return FALSE;

	return TRUE;
}

INetConnection* CTcpAcceptor::Accept()
{
	SOCKET clientSocket;
	struct sockaddr_in clientAddr;
	int iAddrLen = 0;

	iAddrLen = sizeof(clientAddr);
	clientSocket = accept(m_serverSocket, (struct sockaddr*)&clientAddr, &iAddrLen);
	return clientSocket != INVALID_SOCKET ? new CTcpConnection(clientSocket): NULL;
}

BOOL CTcpAcceptor::Stop()
{
	return closesocket(m_serverSocket);
}

///////////////////////CTcpConnection////////////////////////////////////////
CTcpConnection::CTcpConnection(SOCKET socket)
{
	m_socket = socket;
}

CTcpConnection::CTcpConnection(TCHAR *szHost, SHORT uPort)
{
	strncpy(m_szHost, szHost, min(strlen(szHost), 32));
	m_uPort = uPort;
	m_socket = INVALID_SOCKET;
}

CTcpConnection::~CTcpConnection()
{
}


BOOL CTcpConnection::Connect()
{
	 WSADATA  ws;
	 struct	sockaddr_in	serverAddr;
	 int nRet =	0;
	 int nAddrLen =	0;

	//Init	Windows	Socket
	if( WSAStartup(MAKEWORD(2,2),&ws) !=0) return FALSE;
 
	//Create Socket
	m_socket =socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if( m_socket == INVALID_SOCKET) return FALSE;
 
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr	= inet_addr(m_szHost);
	serverAddr.sin_port = htons(m_uPort);
	memset(serverAddr.sin_zero, 0x00, 8);
 
	if(connect(m_socket,(struct sockaddr*)&serverAddr, sizeof(serverAddr))==SOCKET_ERROR) return FALSE;
	
	return TRUE;
}

INT CTcpConnection::Recv(BYTE* piBuffer, DWORD dwSize)
{
	return recv(m_socket, (CHAR*)piBuffer, dwSize, 0);
}

BOOL CTcpConnection::Send(BYTE* piBuffer, DWORD dwSize)
{
	return send(m_socket, (CHAR*)piBuffer, dwSize, 0);
}

VOID CTcpConnection::Close()
{
	closesocket(m_socket);
	m_socket = INVALID_SOCKET;
}

////////////////////CBlockPipeConnection///////////////////////////
CBlockPipeConnection::CBlockPipeConnection(TCHAR* szName)
{
	ZeroMemory(m_szName, sizeof(m_szName));
	strncpy(m_szName, szName, min(sizeof(m_szName), (_tcslen(szName))));
	m_hPipeHandle = INVALID_HANDLE_VALUE;
	m_bServerSide = FALSE;
}

CBlockPipeConnection::CBlockPipeConnection(HANDLE hPipeHandle)
{
	m_hPipeHandle = hPipeHandle;
	ZeroMemory(m_szName, sizeof(m_szName));
	m_bServerSide = TRUE;
}

CBlockPipeConnection::~CBlockPipeConnection()
{
}

BOOL CBlockPipeConnection::Connect()
{
	if(!WaitNamedPipe(m_szName, PIPE_MAX_WAIT_TIME))	return FALSE;
	m_hPipeHandle = CreateFile(m_szName, GENERIC_READ | GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, FILE_FLAG_OVERLAPPED,(HANDLE) NULL);

	return m_hPipeHandle!=INVALID_HANDLE_VALUE;
}
INT CBlockPipeConnection::Recv(BYTE* piBuffer, DWORD dwSize)
{
	DWORD dwBytesRead = 0;
	if(ReadFile(m_hPipeHandle, (LPVOID) piBuffer, dwSize, &dwBytesRead, NULL))
		return dwBytesRead;
	else
		return -1;
}

BOOL CBlockPipeConnection::Send(BYTE* piBuffer, DWORD dwSize)
{
	DWORD dwBytesWritten = 0;
	return WriteFile(m_hPipeHandle, (LPVOID) piBuffer, dwSize, &dwBytesWritten, NULL);
}

VOID CBlockPipeConnection::Close()
{
	if(m_bServerSide) DisconnectNamedPipe(m_hPipeHandle);
	CloseHandle(m_hPipeHandle);
	m_hPipeHandle = INVALID_HANDLE_VALUE;
}



///////////////////CPipeAcceptor///////////////////////////
CPipeAcceptor::CPipeAcceptor(TCHAR* szName)
{
	ZeroMemory(m_szName, sizeof(m_szName));
	_tcsncpy(m_szName, szName, min(sizeof(m_szName), _tcslen(szName)));
}

CPipeAcceptor::~CPipeAcceptor()
{
}

BOOL CPipeAcceptor::Listen()
{
	//DO Nothing
	return TRUE;
}

INetConnection* CPipeAcceptor::Accept()
{
	m_hNowPipeHandle = CreateNamedPipe(m_szName, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
		PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
		PIPE_MAX_CLIENT, BUFFER_SIZE, BUFFER_SIZE, PIPE_MAX_WAIT_TIME, NULL);

	if(m_hNowPipeHandle==INVALID_HANDLE_VALUE)	return NULL;

	HANDLE m_hNowEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if(m_hNowEvent==NULL)	return NULL;

	ZeroMemory(&m_lapNowOvlap, sizeof(OVERLAPPED));
	m_lapNowOvlap.hEvent = m_hNowEvent;

	if( ConnectNamedPipe(m_hNowPipeHandle, &m_lapNowOvlap) == 0
		&&GetLastError() != ERROR_IO_PENDING)	return NULL;

	return WAIT_OBJECT_0==WaitForSingleObject(m_hNowEvent, INFINITE) ?
			new CNioPipeConnection(m_hNowPipeHandle) : NULL;
}

BOOL CPipeAcceptor::Stop()
{
	if(m_hNowPipeHandle!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hNowPipeHandle);
		m_hNowPipeHandle = INVALID_HANDLE_VALUE;
	}

	return TRUE;
}


////////////////////CPipeConnection///////////////////////////
CNioPipeConnection::CNioPipeConnection(TCHAR* szName)
{
	ZeroMemory(m_szName, sizeof(m_szName));
	strncpy(m_szName, szName, min(sizeof(m_szName), (_tcslen(szName))));
	m_hPipeHandle = INVALID_HANDLE_VALUE;
	m_bServerSide = FALSE;

	m_hReadEvent  = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hWriteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	ZeroMemory(&m_olpReadOverlap, sizeof(OVERLAPPED));
	ZeroMemory(&m_olpWriteOverlap, sizeof(OVERLAPPED));

	m_olpReadOverlap.hEvent = m_hReadEvent;
	m_olpWriteOverlap.hEvent = m_hWriteEvent;
}

CNioPipeConnection::CNioPipeConnection(HANDLE hPipeHandle)
{
	m_hPipeHandle = hPipeHandle;
	ZeroMemory(m_szName, sizeof(m_szName));
	m_bServerSide = TRUE;

	m_hReadEvent  = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hWriteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	ZeroMemory(&m_olpReadOverlap, sizeof(OVERLAPPED));
	ZeroMemory(&m_olpWriteOverlap, sizeof(OVERLAPPED));

	m_olpReadOverlap.hEvent = m_hReadEvent;
	m_olpWriteOverlap.hEvent = m_hWriteEvent;
}

CNioPipeConnection::~CNioPipeConnection()
{
}

BOOL CNioPipeConnection::Connect()
{
	if(!WaitNamedPipe(m_szName, PIPE_MAX_WAIT_TIME))
	{
		NET_DBG("CNioPipeConnction::Connect Wait Error:%d\n", GetLastError());	
		return FALSE;
	}
	m_hPipeHandle = CreateFile(m_szName, GENERIC_READ | GENERIC_WRITE, 
		0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, (HANDLE) NULL);
	
	if(m_hPipeHandle==INVALID_HANDLE_VALUE)
	{
		NET_DBG("CNioPipeConnction::Connect CreateFile Error:%d\n", GetLastError());
	}

	return m_hPipeHandle!=INVALID_HANDLE_VALUE;
}
INT CNioPipeConnection::Recv(BYTE* piBuffer, DWORD dwSize)
{
	if (ReadFile(m_hPipeHandle, piBuffer,dwSize, NULL, &m_olpReadOverlap) == 0
		&& GetLastError() != ERROR_IO_PENDING) 
	{
		NET_DBG("CNioPipeConnction::Recv ReadFile Error:%d\n", GetLastError());
		return -1;
	}
	else
	{
		if(WAIT_OBJECT_0==WaitForSingleObject(m_hReadEvent, INFINITE))
		{
			ResetEvent(m_hReadEvent);
			DWORD dwBytesRead = 0;
			return GetOverlappedResult(m_hPipeHandle, &m_olpReadOverlap, &dwBytesRead, TRUE) == 0?-1:dwBytesRead;
		}
		else
		{
			NET_DBG("CNioPipeConnction::Recv Wait Error:%d\n", GetLastError());
			return -1;
		}
	}
}

BOOL CNioPipeConnection::Send(BYTE* piBuffer, DWORD dwSize)
{
	if (WriteFile(m_hPipeHandle, piBuffer,dwSize, NULL, &m_olpWriteOverlap) == 0
		&& GetLastError() != ERROR_IO_PENDING) 
	{
		NET_DBG("CNioPipeConnction::Send Data Error:%d\n", GetLastError());
		return FALSE;
	}
	else
	{
		if(WAIT_OBJECT_0==WaitForSingleObject(m_hWriteEvent, INFINITE))
		{
			ResetEvent(m_hWriteEvent);
			DWORD dwBytesWritten = 0;
			return GetOverlappedResult(m_hPipeHandle, &m_olpWriteOverlap, &dwBytesWritten, TRUE) != 0;
		}
		else
		{
			NET_DBG("CNioPipeConnction::Send Wait Error:%d\n", GetLastError());
			return FALSE;
		}
	}
}

VOID CNioPipeConnection::Close()
{
	if(m_bServerSide) DisconnectNamedPipe(m_hPipeHandle);
	CloseHandle(m_hPipeHandle);
	m_hPipeHandle = INVALID_HANDLE_VALUE;
}
