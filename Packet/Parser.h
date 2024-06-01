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
	BYTE*	m_pBuffer;		//�������ݻ�����
	DWORD	m_dwCapcity;	//���������ܴ�С
	DWORD   m_dwLength;		//�����������ܳ���

	BYTE	m_cSequece;		//�������Sequece
	BOOL	m_bGotSeq;		//�Ƿ�������������к�
	BOOL    m_bFixSeq;		//�Ƿ��Զ��޸����к�
	DWORD	m_dwExtra;		//һЩ������Ϣ
};

#endif // !defined(AFX_PARSER_H__927199CC_99C6_41B6_A5FD_17FD5C8A344E__INCLUDED_)
