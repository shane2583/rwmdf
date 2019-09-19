/*********************************************************************************************
  Copyright 2011 Michael Bührer & Bernd Sparrer. All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are
  permitted provided that the following conditions are met:

     1. Redistributions of source code must retain the above copyright notice, this list of
        conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright notice, this list
        of conditions and the following disclaimer in the documentation and/or other materials
        provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY Michael Bührer  & Bernd Sparrer ``AS IS'' AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Michael Bührer OR Bernd Sparrer
  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  The views and conclusions contained in the software and documentation are those of the
  authors and should not be interpreted as representing official policies, either expressed
  or implied, of Michael Bührer & Bernd Sparrer.
***********************************************************************************************/
#include "stdafx.h"
#include "mdFile.h"

void *mymemcpy(void *pDst, const void *pSrc, size_t nSize);

//-----------------------------------------------------------------
// class mDirectFile
// Note: Multithreaded support?
//-----------------------------------------------------------------
BOOL mDirectFile::ReadAt(M_LINK lAt,M_UINT32 szBytes,void * Into)
{
  if (isOpen())
  {
    DWORD dwRead;
    Position(lAt);
#ifdef _DEBUG
    BOOL bSuccess = ReadFile(m_hFile,Into,szBytes,&dwRead,NULL);
		if (!bSuccess)
		{
			DWORD dwError = GetLastError();
			TRACE("ReadFile() returned error %X\n",dwError);
		}
		if (dwRead==szBytes)
#else
    if (ReadFile(m_hFile,Into,szBytes,&dwRead,NULL) && dwRead==szBytes)
#endif
    {
      m_cp+=szBytes;
      return TRUE;
    }
  }
  return FALSE;
}
BOOL mDirectFile::WriteAt(M_LINK lAt,M_UINT32 szBytes,const void * Data)
{
  if (isOpen())
  {
    DWORD dwWritten;
    Position(lAt);
    if (WriteFile(m_hFile,Data,szBytes,&dwWritten,NULL) && dwWritten==szBytes)
    {
      m_cp+=szBytes;
      if (m_cp>m_Size) m_Size=m_cp;
      return TRUE;
    }
  }
  return FALSE;
}

M_LINK mDirectFile::Request(M_SIZE szBytes,bool bWrite/*=false*/)
{
  M_LINK lEnd=m_Size;
	lEnd = (lEnd+7)/8;
	lEnd *= 8;
	szBytes += lEnd - m_Size;
  if (bWrite)
  {
    LARGE_INTEGER sr;
    mymemcpy(&sr,&szBytes,sizeof(sr));
    SetFilePointerEx(m_hFile,sr,&sr,FILE_END);
    SetEndOfFile(m_hFile);
    mymemcpy(&m_Size,&sr,sizeof(sr));
    m_cp=m_Size;
  }
  else
  {
    m_Size+=szBytes;
  }
  return lEnd;
}
void mDirectFile::Position(M_LINK dwTo)
{
  if (isOpen() /* && m_cp!=dwTo */)
  {
    LARGE_INTEGER sr, np;
    mymemcpy(&sr,&dwTo,sizeof(sr));
    m_cp=dwTo;
    BOOL bSuccess = SetFilePointerEx(m_hFile,sr,&np,FILE_BEGIN);
		if (!bSuccess)
		{
			DWORD dwError = GetLastError();
			TRACE("SetFilePointerEx() returned error %X\n",dwError);
		}
  }
}
void mDirectFile::SetEnd(M_INT64 end)
{
  LARGE_INTEGER sr;
  mymemcpy(&sr,&end,sizeof(sr));
  SetFilePointerEx(m_hFile,sr,&sr,FILE_END);
  SetEndOfFile(m_hFile);
  mymemcpy(&m_Size,&sr,sizeof(sr));
  m_cp=m_Size;
}
