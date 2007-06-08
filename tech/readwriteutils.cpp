///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "tech/readwriteutils.h"

#include "tech/dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

tResult cReadWriteOps<tQuat>::Read(IReader * pReader, tQuat * pQ)
{
   if (pReader == NULL || pQ == NULL)
   {
      return E_POINTER;
   }

   if (pReader->Read(&pQ->x) == S_OK
      && pReader->Read(&pQ->y) == S_OK
      && pReader->Read(&pQ->z) == S_OK
      && pReader->Read(&pQ->w) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}

tResult cReadWriteOps<tQuat>::Write(IWriter * pWriter, const tQuat & q)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (pWriter->Write(q.x) == S_OK
      && pWriter->Write(q.y) == S_OK
      && pWriter->Write(q.z) == S_OK
      && pWriter->Write(q.w) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////

tResult cReadWriteOps<tVec3>::Read(IReader * pReader, tVec3 * pV)
{
   if (pReader == NULL || pV == NULL)
   {
      return E_POINTER;
   }

   if (pReader->Read(&pV->x) == S_OK
      && pReader->Read(&pV->y) == S_OK
      && pReader->Read(&pV->z) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}

tResult cReadWriteOps<tVec3>::Write(IWriter * pWriter, const tVec3 & v)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (pWriter->Write(v.x) == S_OK
      && pWriter->Write(v.y) == S_OK
      && pWriter->Write(v.z) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}

////////////////////////////////////////////////////////////////////////////////

tResult cReadWriteOps<GUID>::Read(IReader * pReader, GUID * pGUID)
{
   if (pReader == NULL || pGUID == NULL)
   {
      return E_POINTER;
   }

   if (pReader->Read(&pGUID->Data1) == S_OK
      && pReader->Read(&pGUID->Data2) == S_OK
      && pReader->Read(&pGUID->Data3) == S_OK
      && pReader->Read(&pGUID->Data4[0], sizeof(pGUID->Data4)) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}

tResult cReadWriteOps<GUID>::Write(IWriter * pWriter, const GUID & guid)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (pWriter->Write(guid.Data1) == S_OK
      && pWriter->Write(guid.Data2) == S_OK
      && pWriter->Write(guid.Data3) == S_OK
      && pWriter->Write((void*)guid.Data4, sizeof(guid.Data4)) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAutoBuffer
//

////////////////////////////////////////

cAutoBuffer::cAutoBuffer()
 : m_pBuffer(NULL)
 , m_bufferSize(0)
{
}

////////////////////////////////////////

cAutoBuffer::~cAutoBuffer()
{
   Free();
}

////////////////////////////////////////

tResult cAutoBuffer::Malloc(size_t bufferSize, void * * ppBuffer)
{
   if (bufferSize == 0)
   {
      return E_INVALIDARG;
   }
   if (ppBuffer == NULL)
   {
      return E_POINTER;
   }
   Free();
   Assert(m_pBuffer == NULL);
   m_pBuffer = reinterpret_cast<byte*>(malloc(bufferSize));
   if (m_pBuffer == NULL)
   {
      return E_OUTOFMEMORY;
   }
   *ppBuffer = m_pBuffer;
   m_bufferSize = bufferSize;
   return S_OK;
}

////////////////////////////////////////

tResult cAutoBuffer::Free()
{
   if (m_pBuffer != NULL)
   {
      free(m_pBuffer);
      m_pBuffer = NULL;
      m_bufferSize = 0;
      return S_OK;
   }
   return S_FALSE;
}

////////////////////////////////////////////////////////////////////////////////
