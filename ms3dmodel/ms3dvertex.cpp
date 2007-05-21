///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ms3dvertex.h"
#include "ms3dflags.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dVertex
//

///////////////////////////////////////

cMs3dVertex::cMs3dVertex()
 : m_flags(kMs3dFlagNone)
 , m_boneId(-1)
 , m_referenceCount(0)
{
   memset(m_vertex, 0, sizeof(m_vertex));
}

///////////////////////////////////////

cMs3dVertex::cMs3dVertex(const cMs3dVertex & other)
 : m_flags(other.m_flags)
 , m_boneId(other.m_boneId)
 , m_referenceCount(other.m_referenceCount)
{
   memcpy(m_vertex, other.m_vertex, sizeof(m_vertex));
}

///////////////////////////////////////

cMs3dVertex::~cMs3dVertex()
{
}

///////////////////////////////////////

const cMs3dVertex & cMs3dVertex::operator =(const cMs3dVertex & other)
{
   m_flags = other.m_flags;
   m_boneId = other.m_boneId;
   m_referenceCount = other.m_referenceCount;
   memcpy(m_vertex, other.m_vertex, sizeof(m_vertex));
   return *this;
}

///////////////////////////////////////

tResult cReadWriteOps<cMs3dVertex>::Read(IReader * pReader, cMs3dVertex * pVertex)
{
   Assert(pReader != NULL);
   Assert(pVertex != NULL);

   tResult result = E_FAIL;

   do
   {
      if (pReader->Read(&pVertex->m_flags, sizeof(pVertex->m_flags)) != S_OK
         || pReader->Read(pVertex->m_vertex, sizeof(pVertex->m_vertex)) != S_OK
         || pReader->Read(&pVertex->m_boneId, sizeof(pVertex->m_boneId)) != S_OK
         || pReader->Read(&pVertex->m_referenceCount, sizeof(pVertex->m_referenceCount)) != S_OK)
      {
         break;
      }

      result = S_OK;
   }
   while (0);

   return result;
}


///////////////////////////////////////////////////////////////////////////////
