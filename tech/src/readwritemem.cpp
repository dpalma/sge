////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "readwritemem.h"
#include "str.h"

#include "dbgalloc.h" // must be last header

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMemReader
//

////////////////////////////////////////

cMemReader::cMemReader(byte * pMem, size_t memSize, bool bOwn)
 : m_pMem(pMem),
   m_memSize(memSize),
   m_bOwn(bOwn),
   m_readPos(0)
{
}

////////////////////////////////////////

cMemReader::~cMemReader()
{
   Assert(m_pMem == NULL);
}

////////////////////////////////////////

void cMemReader::OnFinalRelease()
{
   if (m_pMem != NULL)
   {
      if (m_bOwn)
      {
         delete [] m_pMem;
      }
      m_pMem = NULL;
      m_memSize = 0;
   }
}

////////////////////////////////////////

tResult cMemReader::Tell(ulong * pPos)
{
   if (pPos == NULL)
   {
      return E_POINTER;
   }
   if (m_pMem == NULL)
   {
      *pPos = 0;
      return S_FALSE;
   }
   else
   {
      *pPos = m_readPos;
      return S_OK;
   }
}

////////////////////////////////////////

tResult cMemReader::Seek(long pos, eSeekOrigin origin)
{
   switch (origin)
   {
   case kSO_Set: m_readPos = pos; break;
   case kSO_End: m_readPos = m_memSize + pos; break;
   case kSO_Cur: m_readPos += pos; break;
   }
   return S_OK;
}

////////////////////////////////////////

tResult cMemReader::Read(cStr * pValue, char stop)
{
   if (pValue == NULL)
   {
      return E_POINTER;
   }

   if (m_pMem == NULL)
   {
      return E_FAIL;
   }

   if (m_readPos >= m_memSize)
   {
      return S_FALSE;
   }

   ulong pos;
   if (SUCCEEDED(Tell(&pos)))
   {
      for (; pos < m_memSize; pos++)
      {
         if (m_pMem[pos] == stop)
         {
            break;
         }
      }
   }

   size_t len = pos - m_readPos + 1;

   char * pszBuffer = (char *)alloca(len + 1);

   memcpy(pszBuffer, m_pMem + m_readPos, len);

   m_readPos += len;

   pszBuffer[len] = 0; // ensure always null-terminated
   *pValue = pszBuffer;

   if (m_readPos <= m_memSize)
   {
      return S_OK;
   }
   else
   {
      return S_FALSE;
   }
}

////////////////////////////////////////

tResult cMemReader::Read(void * pv, size_t nBytes, size_t * pnBytesRead)
{
   if (pv == NULL)
   {
      return E_POINTER;
   }

   if (m_pMem == NULL)
   {
      return E_FAIL;
   }

   size_t nBytesRead = Min(m_memSize - m_readPos, nBytes);

   memcpy(pv, m_pMem + m_readPos, nBytesRead);

   m_readPos += nBytesRead;

   if (pnBytesRead != NULL)
   {
      *pnBytesRead = nBytesRead;
   }

   if (m_readPos <= m_memSize)
   {
      return S_OK;
   }
   else
   {
      return S_FALSE;
   }
}

////////////////////////////////////////

tResult ReaderCreateMem(byte * pMem, size_t memSize, bool bOwn, IReader * * ppReader)
{
   if (pMem == NULL || ppReader == NULL)
   {
      return E_POINTER;
   }

   *ppReader = static_cast<IReader *>(new cMemReader(pMem, memSize, bOwn));

   if (*ppReader == NULL)
   {
      return E_OUTOFMEMORY;
   }

   return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
