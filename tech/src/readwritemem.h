///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_READWRITEMEM_H
#define INCLUDED_READWRITEMEM_H

#include "readwriteapi.h"

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
   cMemReader(byte * pMem, size_t memSize, bool bOwn);
   virtual ~cMemReader();

   virtual void OnFinalRelease();

   virtual long Tell();
   virtual tResult Seek(long pos, eSeekOrigin origin);

   virtual tResult Read(cStr * pValue, char stop);
   virtual tResult Read(void * pv, size_t cb, size_t * pcbRead = NULL);

private:
   byte * m_pMem;
   size_t m_memSize;
   bool m_bOwn;
   long m_readPos;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_READWRITEMEM_H
