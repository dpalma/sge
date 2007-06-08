///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_READWRITEMEM_H
#define INCLUDED_READWRITEMEM_H

#include "tech/readwriteapi.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMemReader
//

class cMemReader : public cComObject<IMPLEMENTS(IReader)>
{
public:
   cMemReader(const byte * pMem, size_t memSize, bool bOwn);
   virtual ~cMemReader();

   virtual void OnFinalRelease();

   virtual tResult Tell(ulong * pPos);
   virtual tResult Seek(long pos, eSeekOrigin origin);

   virtual tResult ReadLine(std::string * pLine);
   virtual tResult ReadLine(std::wstring * pLine);

   virtual tResult Read(void * pv, size_t cb, size_t * pcbRead = NULL);

private:
   const byte * m_pMem;
   size_t m_memSize;
   bool m_bOwn;
   size_t m_readPos;
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMemWriter
//

class cMemWriter : public cComObject<IMPLEMENTS(IWriter)>
{
public:
   cMemWriter(byte * pMem, size_t memSize);
   virtual ~cMemWriter();

   virtual tResult Tell(ulong * pPos);
   virtual tResult Seek(long pos, eSeekOrigin origin);

   virtual tResult Write(const void * pValue, size_t cbValue,
                         size_t * pcbWritten = NULL);

private:
   byte * m_pMem;
   size_t m_memSize;
   size_t m_writePos;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_READWRITEMEM_H
