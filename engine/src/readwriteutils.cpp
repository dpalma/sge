///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "readwriteutils.h"

#include "dbgalloc.h" // must be last header

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
