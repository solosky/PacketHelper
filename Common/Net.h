#pragma once
#ifndef NET_H
#define NET_H
//#include <windows.h>
#include <WINSOCK2.H>
#include <vector>

#pragma comment(lib, "ws2_32.lib")


using namespace std;

/////////////////////////////////////////////////////////////////////////////////

#define PACKET_HEADER_LEN			8			//包头部长度
#define PACKET_DEFAULT_CAPCITY		256			//默认数据区长度
#define BUFFER_SIZE					1024		//读取或者写入缓冲区大小
#define PIPE_MAX_WAIT_TIME			3000		//PIPE等到管道可用的最大时间
#define PIPE_MAX_CLIENT				100			//最大客户端数量

/////////////////////////////////////////////////////////////////////////////
VOID NET_DBG(const char* szFormat, ...);
class INetConnection;
class INetAcceptor;

/////////////////////////////CPacket//////////////////////////////////////////
/*定义了数据包类，网络上的所有数据都是通过数据包传送的，这个定义比较抽象，实际上只是定义了包头，数据包的具体内容可以自行定义*/
class CPacket
{
private:
	DWORD m_dwPosition;		//数据大小
	DWORD m_dwCapcity;		//数据区容量
	BYTE* m_pData;			//数据区，在读取的过程中会随着包的大小自动扩充，但不会减小


public:
	/*构造一个默认大小的缓冲区,通常用来读取数据*/
	CPacket();
	
	/*详细的构造一个包，给定数据*/
	CPacket(DWORD dwCmd, DWORD dwLength, BYTE* pData);
	
	/*默认析构函数*/
	~CPacket();

	/*从一个缓冲区中解析包，返回下一次读取位置*/
	DWORD Parse(BYTE* pData, DWORD dwSize, DWORD dwPos);
	
	/*把这个数据包发送出去,返回是否发送成功*/
	BOOL Send(INetConnection* piNetConnection);

	/*重置包状态，方便下一次接收数据*/
	VOID Reset();
	
	/*判断这个数据包是否读取完毕*/
	BOOL IsReady();

	/*返回命令号*/
	DWORD GetCmd();

	/*返回长度*/
	DWORD GetLength();

	/*返回数据*/
	BYTE* GetData();

	VOID Set(DWORD dwCmd, DWORD dwLength, BYTE* pData);

private:
	BOOL CheckCapcity(DWORD dwSize);
};

///////////////////////////INetHandler/////////////////////////////////
/*网络事件处理接口，程序需要实现这个接口来完成数据包和连接的回调*/
class INetHandler
{
	public:
	/*连接已经被建立*/
	virtual VOID OnConnected(INetConnection* piNetConnection ) = 0;

	/*连接出现异常，不可恢复*/
	virtual VOID OnError(INetConnection* piNetConnection) = 0;

	/*连接被正常关闭*/
	virtual VOID OnClosed(INetConnection* piNetConnection) = 0;

	/*接收到了新的数据包*/
	virtual VOID OnPacket(INetConnection* piNetConnection, CPacket *pPacket) = 0;

	/*接收到新的连接，仅在服务器端回调*/
	virtual VOID OnNew(INetConnection* piNetConnection) = 0;
};

///////////////////////////INetFactory////////////////////////////////////
/*网络监听器，接收客户端的连接*/
class INetAcceptor
{
public:
	/*在一个资源地址上监听客户端连接*/
	virtual BOOL Listen() = 0;
	
	/*监听客户端的连接，如果有新的连接就返回新的连接对象*/
	virtual INetConnection* Accept() = 0;

	/*停止监听*/
	virtual BOOL Stop() = 0;
};

////////////////////////INetProvider//////////////////////////////////
/*网络连接对象，指代了一个具体的连接*/
class INetConnection
{
	public:
	/*执行连接操作*/
	virtual BOOL Connect() = 0;
	
	/*阻塞的读取数据，返回实际读取数据的长度*/
	virtual INT Recv(BYTE* piBuffer, DWORD dwSize) = 0;

	/*阻塞的发送数据*/
	virtual BOOL Send(BYTE* piBuffer, DWORD dwSize) = 0;

	/*主动关闭连接*/
	virtual VOID Close() = 0;
};
 

//////////////////////////CNetWorker///////////////////////////////////
/*网络连接处理对象，用来完成一个网络处理逻辑*/
class CNetWorker
{
private:
	INetConnection* m_piNetConnection;
	INetHandler*  m_piNetHandler;
	BOOL m_bNeedConnect;			//是否需要连接操作
	BOOL m_bExited;					//是否已经退出
	DWORD m_dwExitCode;				//退出的代码，正常退出为0
	HANDLE m_hThread;				//读取线程的句柄
	BYTE m_biBuffer[BUFFER_SIZE];	//读取缓冲区
	CPacket		m_pkPacket;			//读取数据包

public:
	/*构造函数*/
	CNetWorker(INetConnection* piNetConnection, INetHandler* piNetHandler, BOOL bNeedConnect);
	
	/*析构函数*/
	~CNetWorker();

	/*开始执行工作,会创建一个新的线程执行工作，这个函数会立即返回*/
	BOOL Start();

	/*强制停止工作*/
	VOID Stop(DWORD dwExitCode=-1);

	/*阻塞的执行具体的工作，仅在线程中访问*/
	DWORD Work();

	/*获取退出代码*/
	DWORD GetExitCode(){ return m_dwExitCode; }

	/*是否已经退出*/
	BOOL IsExited(){ return m_bExited; }

	/*获得连接对象*/
	INetConnection* GetNetConnection() {return m_piNetConnection;}

	/*获得连接处理对象*/
	INetHandler* GetNetHandler(){return m_piNetHandler;}

private:
	/*线程过程，直接调用Work方法执行任务*/
	static DWORD WINAPI WorkProc(LPVOID lpParam);
};

///////////////////////////CSimpleNetHandler///////////////////////////
/*一个空的INetHandler的实现，默认什么也不做*/
class CSimpleNetHandler: public INetHandler
{
	public:
	/*连接已经被建立*/
	virtual VOID OnConnected(INetConnection* piNetConnection );

	/*连接出现异常，不可恢复*/
	virtual VOID OnError(INetConnection* piNetConnection);

	/*连接被正常关闭*/
	virtual VOID OnClosed(INetConnection* piNetConnection);

	/*接收到了新的数据包*/
	virtual VOID OnPacket(INetConnection* piNetConnection, CPacket *pPacket);

	/*接收到新的连接，仅在服务器端回调*/
	virtual VOID OnNew(INetConnection* piNetConnection);
};

//////////////////////////CNetServer//////////////////////////////////
/*网络连接服务器，处理服务器逻辑*/
class CNetServer
{
private:
	INetAcceptor* m_piNetAcceptor;		//连接工厂接口
	INetHandler* m_piNetHandler;		//网络事件处理接口
	vector<CNetWorker*> m_vtrWorkers;	//活跃的连接，封装成Woker并保存
	HANDLE m_hAcceptorThread;			//监听线程

public:
	/*构造函数*/
	CNetServer(INetAcceptor* piNetAcceptor, INetHandler* piNetHandler);
	/*析构函数*/
	~CNetServer();

	/*启动服务器*/
	BOOL Start();

	/*关闭服务器*/
	VOID Stop();

	/*循环执行监听操作，仅在接收线程中访问。*/
	DWORD Accepting();

	/*返回Acceptor*/
	INetAcceptor* GetNetAcceptor(){ return m_piNetAcceptor; }

private:
	/*监听线程体，阻塞的循环等待客户端来连接*/
static DWORD WINAPI AcceptProc(LPVOID lpParam);
};


/////////////////////////////CTcpAcceptor///////////////////////////////////
/* 实现用TCP监听连接*/
class CTcpAcceptor: public INetAcceptor
{
private :
	SHORT m_uPort;				//监听端口
	SOCKET m_serverSocket;		//服务端Socket
public:
	CTcpAcceptor(SHORT uPort);
	~CTcpAcceptor();
	virtual BOOL Listen();
	virtual INetConnection* Accept();
	virtual BOOL Stop();
};

//////////////////////////CTcpConnection///////////////////////////////////////
/*TCP网络连接*/
class CTcpConnection: public INetConnection
{
private:
	SOCKET m_socket;	//客户端SCOKET
	TCHAR m_szHost[32];	//服务器地址
	SHORT m_uPort;		//服务器端口

public:
	CTcpConnection(SOCKET socket);
	CTcpConnection(TCHAR* szHost, SHORT dwPort);
	~CTcpConnection();

	virtual BOOL Connect();
	virtual INT Recv(BYTE* piBuffer, DWORD dwSize);
	virtual BOOL Send(BYTE* piBuffer, DWORD dwSize);
	virtual VOID Close();
};

///////////////////////////CBlockPipeConnection///////////////////////////////////
/*Pipe网络连接*/
class CBlockPipeConnection: public INetConnection
{
private:
	TCHAR m_szName[32];			//管道的名字
	HANDLE m_hPipeHandle;		//当前监听的PIPE句柄
	BOOL m_bServerSide;			//是否是服务器端的连接

public:
	CBlockPipeConnection(TCHAR* szName);
	CBlockPipeConnection(HANDLE hPipeHandle);
	~CBlockPipeConnection();

	virtual BOOL Connect();
	virtual INT Recv(BYTE* piBuffer, DWORD dwSize);
	virtual BOOL Send(BYTE* piBuffer, DWORD dwSize);
	virtual VOID Close();
};

/////////////////////////////////////////////////////////////////////
/*命名管道监听器的实现2,使用Nio处理*/
class CPipeAcceptor: public INetAcceptor
{
private:
	TCHAR m_szName[32];			//管道的名字
	HANDLE m_hNowPipeHandle;		//当前正在监听的管道句柄
	HANDLE m_hNowEvent;
	OVERLAPPED m_lapNowOvlap;

public:
	CPipeAcceptor(TCHAR* szName);
	~CPipeAcceptor();

	virtual BOOL Listen();
	virtual INetConnection* Accept();
	virtual BOOL Stop();
};

////////////////////////////////////////////////////
/*Pipe网络连接2,使用NIO来实现异步的读写*/
class CNioPipeConnection: public INetConnection
{
private:
	TCHAR m_szName[32];			//管道的名字
	HANDLE m_hPipeHandle;		//当前监听的PIPE句柄
	BOOL m_bServerSide;			//是否是服务器端的连接
	HANDLE m_hReadEvent;		//读事件
	HANDLE m_hWriteEvent;		//写事件
	OVERLAPPED m_olpReadOverlap;	//读结果
	OVERLAPPED m_olpWriteOverlap;	//写结果


public:
	CNioPipeConnection(TCHAR* szName);
	CNioPipeConnection(HANDLE hPipeHandle);
	~CNioPipeConnection();

	virtual BOOL Connect();
	virtual INT Recv(BYTE* piBuffer, DWORD dwSize);
	virtual BOOL Send(BYTE* piBuffer, DWORD dwSize);
	virtual VOID Close();
};
#endif