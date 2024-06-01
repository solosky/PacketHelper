
//公共结构体和宏定义，便于修改

#ifndef COMMON_H
#define COMMON_H

#include <list>
#include <winsock2.h>
using namespace std;


//如果没有定义BOOL,TRUE,FALSE就重新定义
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


//游戏数据包结构
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

//连接信息结构
typedef struct __TAG_CONNECTION
{
	DWORD		dwPid;
	DWORD		dwSocket;
	DWORD		dwAddr;
	USHORT		uPort;
	DWORD		dwAction;
	DWORD		dwExtra;
} CONNECTION, *PCONNECTION;

//同时保存连接和数据结构
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


//消息命令
enum CMD {
	CMD_GAME_MSG,
	CMD_SEND_MSG,
	CMD_HOOK_FUN
};

//操作类型
enum ACTION{
	ACTION_SEND,
	ACTION_RECV,
	ACTION_CONNECT,
	ACTION_CLOSE
};

//额外消息类型
enum EXTRA{
	EXTRA_FIXED_SEQ
};


//Recevier传递给GUI的事件
enum EVENT{
	EVENT_GAME_MSG,
	EVENT_CONN_NEW,
	EVENT_CONN_CLOSED,
};

//结果枚举
enum RET{
	RET_OK,
	RET_ERR
};


VOID Dbg(const char * szFormat, ...);
#endif