
//�����ṹ��ͺ궨�壬�����޸�

#ifndef COMMON_H
#define COMMON_H

#include <list>
#include <winsock2.h>
using namespace std;


//���û�ж���BOOL,TRUE,FALSE�����¶���
#ifndef	BOOL
	#define	BOOL	int
#endif
#ifndef	TRUE
	#define	TRUE	1
#endif
#ifndef	FALSE
	#define	FALSE	0
#endif

#define PIPE_NAME "\\\\.\\Pipe\\PacketHelper"
//#define PIPE_NAME "\\\\.\\Pipe\\solosky\\TLBB"

#define safe_free(x) if(x!=NULL) free(x); x=NULL;
#define safe_delete(x) if(x!=NULL) delete x; x=NULL;
#define CHECK_NULL(x) if(x==NULL) { \
						MessageBox(NULL,"malloc memory failed.", NULL, MB_OK); \
						CApp::GetSingleton()->Exit();}

#define MAKE_LENGTH(msg) msg->uLength+(((DWORD)msg->cLengthEx)<<16)
#define MAKE_LENGTH_X(msg) msg.uLength+(((DWORD)msg.cLengthEx)<<16)
#define MSG_LENGTH(msg) MAKE_LENGTH(msg)+6
#define MSG_LENGTH_X(msg) MAKE_LENGTH_X(msg)+6

typedef int (WINAPI * FunRecv)(SOCKET a0,char* a1,int a2,int a3);
typedef int (WINAPI * FunSend)(SOCKET a0, CONST char* a1, int a2, int a3);
typedef VOID (* FnEventHandler) (DWORD dwEventType, DWORD pParam1, DWORD pParam2);


//��Ϸ���ݰ��ṹ
#pragma pack(1)
typedef struct __TAG_MESSAGE
{
	WORD	uCmd;
	WORD	uLength;
	BYTE	cLengthEx;
	BYTE	cSequece;
	BYTE    pData[1];
} MESSAGE, *PMESSAGE;
#pragma pack()

//������Ϣ�ṹ
typedef struct __TAG_CONNECTION
{
	DWORD		dwPid;
	DWORD		dwSocket;
	DWORD		dwAddr;
	USHORT		uPort;
	DWORD		dwAction;
	DWORD		dwExtra;
} CONNECTION, *PCONNECTION;

//ͬʱ�������Ӻ����ݽṹ
typedef struct __TAG_MSG_ENTRY
{
	CONNECTION Connection;
	MESSAGE	Message;
} MSG_ENTRY, *PMSG_ENTRY;

typedef struct __TAG_HOOK_INFO
{
	DWORD dwHookType;
	DWORD dwFixSequece;
} HOOK_INFO, *PHOOK_INFO;


//��Ϣ����
enum CMD {
	CMD_GAME_MSG,
	CMD_SEND_MSG,
	CMD_HOOK_FUN
};

//��������
enum ACTION{
	ACTION_SEND,
	ACTION_RECV,
	ACTION_CONNECT,
	ACTION_CLOSE
};

//������Ϣ����
enum EXTRA{
	EXTRA_FIXED_SEQ
};


//Recevier���ݸ�GUI���¼�
enum EVENT{
	EVENT_GAME_MSG,
	EVENT_CONN_NEW,
	EVENT_CONN_CLOSED,
};

//���ö��
enum RET{
	RET_OK,
	RET_ERR
};


VOID Dbg(const char * szFormat, ...);
#endif