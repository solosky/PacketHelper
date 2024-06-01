#pragma once
#ifndef NET_H
#define NET_H
//#include <windows.h>
#include <WINSOCK2.H>
#include <vector>

#pragma comment(lib, "ws2_32.lib")


using namespace std;

/////////////////////////////////////////////////////////////////////////////////

#define PACKET_HEADER_LEN			8			//��ͷ������
#define PACKET_DEFAULT_CAPCITY		256			//Ĭ������������
#define BUFFER_SIZE					1024		//��ȡ����д�뻺������С
#define PIPE_MAX_WAIT_TIME			3000		//PIPE�ȵ��ܵ����õ����ʱ��
#define PIPE_MAX_CLIENT				100			//���ͻ�������

/////////////////////////////////////////////////////////////////////////////
VOID NET_DBG(const char* szFormat, ...);
class INetConnection;
class INetAcceptor;

/////////////////////////////CPacket//////////////////////////////////////////
/*���������ݰ��࣬�����ϵ��������ݶ���ͨ�����ݰ����͵ģ��������Ƚϳ���ʵ����ֻ�Ƕ����˰�ͷ�����ݰ��ľ������ݿ������ж���*/
class CPacket
{
private:
	DWORD m_dwPosition;		//���ݴ�С
	DWORD m_dwCapcity;		//����������
	BYTE* m_pData;			//���������ڶ�ȡ�Ĺ����л����Ű��Ĵ�С�Զ����䣬�������С


public:
	/*����һ��Ĭ�ϴ�С�Ļ�����,ͨ��������ȡ����*/
	CPacket();
	
	/*��ϸ�Ĺ���һ��������������*/
	CPacket(DWORD dwCmd, DWORD dwLength, BYTE* pData);
	
	/*Ĭ����������*/
	~CPacket();

	/*��һ���������н�������������һ�ζ�ȡλ��*/
	DWORD Parse(BYTE* pData, DWORD dwSize, DWORD dwPos);
	
	/*��������ݰ����ͳ�ȥ,�����Ƿ��ͳɹ�*/
	BOOL Send(INetConnection* piNetConnection);

	/*���ð�״̬��������һ�ν�������*/
	VOID Reset();
	
	/*�ж�������ݰ��Ƿ��ȡ���*/
	BOOL IsReady();

	/*���������*/
	DWORD GetCmd();

	/*���س���*/
	DWORD GetLength();

	/*��������*/
	BYTE* GetData();

	VOID Set(DWORD dwCmd, DWORD dwLength, BYTE* pData);

private:
	BOOL CheckCapcity(DWORD dwSize);
};

///////////////////////////INetHandler/////////////////////////////////
/*�����¼�����ӿڣ�������Ҫʵ������ӿ���������ݰ������ӵĻص�*/
class INetHandler
{
	public:
	/*�����Ѿ�������*/
	virtual VOID OnConnected(INetConnection* piNetConnection ) = 0;

	/*���ӳ����쳣�����ɻָ�*/
	virtual VOID OnError(INetConnection* piNetConnection) = 0;

	/*���ӱ������ر�*/
	virtual VOID OnClosed(INetConnection* piNetConnection) = 0;

	/*���յ����µ����ݰ�*/
	virtual VOID OnPacket(INetConnection* piNetConnection, CPacket *pPacket) = 0;

	/*���յ��µ����ӣ����ڷ������˻ص�*/
	virtual VOID OnNew(INetConnection* piNetConnection) = 0;
};

///////////////////////////INetFactory////////////////////////////////////
/*��������������տͻ��˵�����*/
class INetAcceptor
{
public:
	/*��һ����Դ��ַ�ϼ����ͻ�������*/
	virtual BOOL Listen() = 0;
	
	/*�����ͻ��˵����ӣ�������µ����Ӿͷ����µ����Ӷ���*/
	virtual INetConnection* Accept() = 0;

	/*ֹͣ����*/
	virtual BOOL Stop() = 0;
};

////////////////////////INetProvider//////////////////////////////////
/*�������Ӷ���ָ����һ�����������*/
class INetConnection
{
	public:
	/*ִ�����Ӳ���*/
	virtual BOOL Connect() = 0;
	
	/*�����Ķ�ȡ���ݣ�����ʵ�ʶ�ȡ���ݵĳ���*/
	virtual INT Recv(BYTE* piBuffer, DWORD dwSize) = 0;

	/*�����ķ�������*/
	virtual BOOL Send(BYTE* piBuffer, DWORD dwSize) = 0;

	/*�����ر�����*/
	virtual VOID Close() = 0;
};
 

//////////////////////////CNetWorker///////////////////////////////////
/*�������Ӵ�������������һ�����紦���߼�*/
class CNetWorker
{
private:
	INetConnection* m_piNetConnection;
	INetHandler*  m_piNetHandler;
	BOOL m_bNeedConnect;			//�Ƿ���Ҫ���Ӳ���
	BOOL m_bExited;					//�Ƿ��Ѿ��˳�
	DWORD m_dwExitCode;				//�˳��Ĵ��룬�����˳�Ϊ0
	HANDLE m_hThread;				//��ȡ�̵߳ľ��
	BYTE m_biBuffer[BUFFER_SIZE];	//��ȡ������
	CPacket		m_pkPacket;			//��ȡ���ݰ�

public:
	/*���캯��*/
	CNetWorker(INetConnection* piNetConnection, INetHandler* piNetHandler, BOOL bNeedConnect);
	
	/*��������*/
	~CNetWorker();

	/*��ʼִ�й���,�ᴴ��һ���µ��߳�ִ�й����������������������*/
	BOOL Start();

	/*ǿ��ֹͣ����*/
	VOID Stop(DWORD dwExitCode=-1);

	/*������ִ�о���Ĺ����������߳��з���*/
	DWORD Work();

	/*��ȡ�˳�����*/
	DWORD GetExitCode(){ return m_dwExitCode; }

	/*�Ƿ��Ѿ��˳�*/
	BOOL IsExited(){ return m_bExited; }

	/*������Ӷ���*/
	INetConnection* GetNetConnection() {return m_piNetConnection;}

	/*������Ӵ������*/
	INetHandler* GetNetHandler(){return m_piNetHandler;}

private:
	/*�̹߳��̣�ֱ�ӵ���Work����ִ������*/
	static DWORD WINAPI WorkProc(LPVOID lpParam);
};

///////////////////////////CSimpleNetHandler///////////////////////////
/*һ���յ�INetHandler��ʵ�֣�Ĭ��ʲôҲ����*/
class CSimpleNetHandler: public INetHandler
{
	public:
	/*�����Ѿ�������*/
	virtual VOID OnConnected(INetConnection* piNetConnection );

	/*���ӳ����쳣�����ɻָ�*/
	virtual VOID OnError(INetConnection* piNetConnection);

	/*���ӱ������ر�*/
	virtual VOID OnClosed(INetConnection* piNetConnection);

	/*���յ����µ����ݰ�*/
	virtual VOID OnPacket(INetConnection* piNetConnection, CPacket *pPacket);

	/*���յ��µ����ӣ����ڷ������˻ص�*/
	virtual VOID OnNew(INetConnection* piNetConnection);
};

//////////////////////////CNetServer//////////////////////////////////
/*�������ӷ�����������������߼�*/
class CNetServer
{
private:
	INetAcceptor* m_piNetAcceptor;		//���ӹ����ӿ�
	INetHandler* m_piNetHandler;		//�����¼�����ӿ�
	vector<CNetWorker*> m_vtrWorkers;	//��Ծ�����ӣ���װ��Woker������
	HANDLE m_hAcceptorThread;			//�����߳�

public:
	/*���캯��*/
	CNetServer(INetAcceptor* piNetAcceptor, INetHandler* piNetHandler);
	/*��������*/
	~CNetServer();

	/*����������*/
	BOOL Start();

	/*�رշ�����*/
	VOID Stop();

	/*ѭ��ִ�м������������ڽ����߳��з��ʡ�*/
	DWORD Accepting();

	/*����Acceptor*/
	INetAcceptor* GetNetAcceptor(){ return m_piNetAcceptor; }

private:
	/*�����߳��壬������ѭ���ȴ��ͻ���������*/
static DWORD WINAPI AcceptProc(LPVOID lpParam);
};


/////////////////////////////CTcpAcceptor///////////////////////////////////
/* ʵ����TCP��������*/
class CTcpAcceptor: public INetAcceptor
{
private :
	SHORT m_uPort;				//�����˿�
	SOCKET m_serverSocket;		//�����Socket
public:
	CTcpAcceptor(SHORT uPort);
	~CTcpAcceptor();
	virtual BOOL Listen();
	virtual INetConnection* Accept();
	virtual BOOL Stop();
};

//////////////////////////CTcpConnection///////////////////////////////////////
/*TCP��������*/
class CTcpConnection: public INetConnection
{
private:
	SOCKET m_socket;	//�ͻ���SCOKET
	TCHAR m_szHost[32];	//��������ַ
	SHORT m_uPort;		//�������˿�

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
/*Pipe��������*/
class CBlockPipeConnection: public INetConnection
{
private:
	TCHAR m_szName[32];			//�ܵ�������
	HANDLE m_hPipeHandle;		//��ǰ������PIPE���
	BOOL m_bServerSide;			//�Ƿ��Ƿ������˵�����

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
/*�����ܵ���������ʵ��2,ʹ��Nio����*/
class CPipeAcceptor: public INetAcceptor
{
private:
	TCHAR m_szName[32];			//�ܵ�������
	HANDLE m_hNowPipeHandle;		//��ǰ���ڼ����Ĺܵ����
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
/*Pipe��������2,ʹ��NIO��ʵ���첽�Ķ�д*/
class CNioPipeConnection: public INetConnection
{
private:
	TCHAR m_szName[32];			//�ܵ�������
	HANDLE m_hPipeHandle;		//��ǰ������PIPE���
	BOOL m_bServerSide;			//�Ƿ��Ƿ������˵�����
	HANDLE m_hReadEvent;		//���¼�
	HANDLE m_hWriteEvent;		//д�¼�
	OVERLAPPED m_olpReadOverlap;	//�����
	OVERLAPPED m_olpWriteOverlap;	//д���


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