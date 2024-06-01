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

	//缓冲区不够封包头部大小， 尝试读满封包头部信息
	if(m_dwLength<6)
	{
		dwReadSize = min(6-m_dwLength, dwRemainSize);
		memcpy(ptrOut, ptrIn, dwReadSize);

		dwRemainSize -= dwReadSize;
		m_dwLength   += dwReadSize;
		ptrOut       += dwReadSize;
		ptrIn        += dwReadSize;
		dwNextPos    += dwReadSize;

		//如果当前缓冲区已经足够一个头部信息，表明这次读取一定读取了序号
		if(m_dwLength==6)	
		{
			BYTE* ptrInSeq  = ptrIn-1;		//ptrIn此时应该指向了封包数据部分，-1即为序号
			BYTE* ptrOutSeq = ptrOut-1;
			if(m_bGotSeq)	//已经获取到序号，直接修正原始封包序号为自己保存的序号
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

		//是否还有数据读取
		if(dwRemainSize==0)	return dwNextPos;
	}

	//如果上面没有返回，表明已经成功的读取了一个包头，使用PMESSAGE对象来方便的访问头部信息
	
	PMESSAGE pMsg    = (PMESSAGE)(m_pBuffer);
	DWORD dwDataSize = MAKE_LENGTH(pMsg);		//包数据长度
	dwReadSize       = min(dwDataSize-(m_dwLength-6), dwRemainSize);	//应该读取的数据长度
	//Dbg("Parse: Recv A Header, Cmd=%d, Seq=%hd, Length=%d",pMsg->uCmd, pMsg->cSequece, dwDataSize);

	//判断缓冲区空间是否可以足够，如果不够重新分配
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
	
	//复制数据
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