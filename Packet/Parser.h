// Parser.h: interface for the CParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARSER_H__927199CC_99C6_41B6_A5FD_17FD5C8A344E__INCLUDED_)
#define AFX_PARSER_H__927199CC_99C6_41B6_A5FD_17FD5C8A344E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Common.h"

class CParser  
{
	
public:
	CParser();
	virtual ~CParser();

	DWORD		Parse(BYTE* pData, DWORD dwSize, DWORD dwNextPos);
	PMESSAGE	GetMessage(){ return (PMESSAGE) m_pBuffer; }
	BOOL		HasMessage(){ return  m_dwLength>=6 ? MSG_LENGTH(GetMessage())==m_dwLength: FALSE;}
	VOID		ResetMessage(){ m_dwLength=0; m_dwExtra=0; }
	VOID		ResetSequece(){m_bGotSeq = FALSE;}
	VOID		SetFixSequece(BOOL bFixSeq) {m_bFixSeq = bFixSeq;}
	VOID		SetExtra(DWORD dwExtra){m_dwExtra = dwExtra;}
	DWORD		GetExtra(){ return m_dwExtra;}
private:
	BYTE*	m_pBuffer;		//解析数据缓冲区
	DWORD	m_dwCapcity;	//缓冲区的总大小
	DWORD   m_dwLength;		//缓冲区数据总长度

	BYTE	m_cSequece;		//修正后的Sequece
	BOOL	m_bGotSeq;		//是否解析出来的序列号
	BOOL    m_bFixSeq;		//是否自动修复序列号
	DWORD	m_dwExtra;		//一些额外信息
};

#endif // !defined(AFX_PARSER_H__927199CC_99C6_41B6_A5FD_17FD5C8A344E__INCLUDED_)
