// Parser.cpp: implementation of the CParser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Parser.h"
#include "App.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CParser::CParser()
{
	m_bGotSeq = FALSE;
	m_dwCapcity  = 1024;
	m_dwLength   = 0;
	m_pBuffer    = (BYTE*) malloc(m_dwCapcity); 
	CHECK_NULL(m_pBuffer);
}

CParser::~CParser()
{
	safe_free(m_pBuffer);
}


DWORD CParser::Parse(BYTE* pData, DWORD dwSize, DWORD dwNextPos)
{
	//Dbg("=======================================================================");
	//Dbg("Parse Start: m_dwCapcity=%d, m_dwLength=%d, DataSize=%d, NextPos=%d", 
	//	m_dwCapcity, m_dwLength, dwSize, dwNextPos);
	
	DWORD dwRemainSize = dwSize - dwNextPos;
	BYTE* ptrIn        = pData + dwNextPos;
	BYTE* ptrOut       = m_pBuffer + m_dwLength;
	DWORD dwReadSize   = 0;

	//�������������ͷ����С�� ���Զ������ͷ����Ϣ
	if(m_dwLength<6)
	{
		dwReadSize = min(6-m_dwLength, dwRemainSize);
		memcpy(ptrOut, ptrIn, dwReadSize);

		dwRemainSize -= dwReadSize;
		m_dwLength   += dwReadSize;
		ptrOut       += dwReadSize;
		ptrIn        += dwReadSize;
		dwNextPos    += dwReadSize;

		//�����ǰ�������Ѿ��㹻һ��ͷ����Ϣ��������ζ�ȡһ����ȡ�����
		if(m_dwLength==6)	
		{
			BYTE* ptrInSeq  = ptrIn-1;		//ptrIn��ʱӦ��ָ���˷�����ݲ��֣�-1��Ϊ���
			BYTE* ptrOutSeq = ptrOut-1;
			if(m_bGotSeq)	//�Ѿ���ȡ����ţ�ֱ������ԭʼ������Ϊ�Լ���������
			{
				UCHAR cParseSeq = ++m_cSequece;
				UCHAR CSendSeq  = *ptrInSeq;
				if(cParseSeq!=CSendSeq)
				{
					if(m_bFixSeq)
					{
						*ptrInSeq  = cParseSeq;
						*ptrOutSeq = *ptrInSeq; 
						m_dwExtra |= EXTRA_FIXED_SEQ;
					}
					else
					{
						m_dwExtra |= EXTRA_ERR_SEQ;
					}
				}
			
			}
			else
			{
				m_cSequece = *ptrInSeq;
				m_bGotSeq  = TRUE;
			}
		}

		//�Ƿ������ݶ�ȡ
		if(dwRemainSize==0)	return dwNextPos;
	}

	//�������û�з��أ������Ѿ��ɹ��Ķ�ȡ��һ����ͷ��ʹ��PMESSAGE����������ķ���ͷ����Ϣ
	
	PMESSAGE pMsg    = (PMESSAGE)(m_pBuffer);
	DWORD dwDataSize = MAKE_LENGTH(pMsg);		//�����ݳ���
	dwReadSize       = min(dwDataSize-(m_dwLength-6), dwRemainSize);	//Ӧ�ö�ȡ�����ݳ���
	//Dbg("Parse: Recv A Header, Cmd=%d, Seq=%hd, Length=%d",pMsg->uCmd, pMsg->cSequece, dwDataSize);

	//�жϻ������ռ��Ƿ�����㹻������������·���
	if(m_dwCapcity<m_dwLength+dwReadSize)
	{
		BYTE* newBuffer = (BYTE*) malloc(m_dwLength+dwReadSize);
		CHECK_NULL(newBuffer);
		memcpy(newBuffer, m_pBuffer, m_dwLength);

		m_pBuffer   = newBuffer;
		m_dwCapcity = m_dwLength+dwReadSize;
		ptrOut      = m_pBuffer+m_dwLength;

		//Dbg("Parse: Expand Buffer: %d", m_dwCapcity);
	}
	
	//��������
	memcpy(ptrOut, ptrIn, dwReadSize);

	dwRemainSize -= dwReadSize;
	m_dwLength   += dwReadSize;
	ptrOut       += dwReadSize;
	ptrIn        += dwReadSize;
	dwNextPos    += dwReadSize;

	//Dbg("Parse End: m_dwCapcity=%d, m_dwLength=%d, DataSize=%d, NextPos=%d, HasMsg=%d", 
	//	m_dwCapcity, m_dwLength, dwSize, dwNextPos, HasMessage());

	return dwNextPos;
}