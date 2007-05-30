///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ms3dmodel/ms3dtriangle.h"
#include "ms3dmodel/ms3dflags.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dTriangle
//

///////////////////////////////////////

cMs3dTriangle::cMs3dTriangle()
 : m_flags(kMs3dFlagNone)
 , m_smoothingGroup(0)
 , m_groupIndex(0)
{
   memset(m_vertexIndices, 0, sizeof(m_vertexIndices));
   memset(m_vertexNormals, 0, sizeof(m_vertexNormals));
   memset(m_s, 0, sizeof(m_s));
   memset(m_t, 0, sizeof(m_t));
}

///////////////////////////////////////

cMs3dTriangle::cMs3dTriangle(const cMs3dTriangle & other)
 : m_flags(other.m_flags)
 , m_smoothingGroup(other.m_smoothingGroup)
 , m_groupIndex(other.m_groupIndex)
{
   memcpy(m_vertexIndices, other.m_vertexIndices, sizeof(m_vertexIndices));
   memcpy(m_vertexNormals, other.m_vertexNormals, sizeof(m_vertexNormals));
   memcpy(m_s, other.m_s, sizeof(m_s));
   memcpy(m_t, other.m_t, sizeof(m_t));
}

///////////////////////////////////////

cMs3dTriangle::~cMs3dTriangle()
{
}

///////////////////////////////////////

const cMs3dTriangle & cMs3dTriangle::operator =(const cMs3dTriangle & other)
{
   m_flags = other.m_flags;
   memcpy(m_vertexIndices, other.m_vertexIndices, sizeof(m_vertexIndices));
   memcpy(m_vertexNormals, other.m_vertexNormals, sizeof(m_vertexNormals));
   memcpy(m_s, other.m_s, sizeof(m_s));
   memcpy(m_t, other.m_t, sizeof(m_t));
   m_smoothingGroup = other.m_smoothingGroup;
   m_groupIndex = other.m_groupIndex;
   return *this;
}

///////////////////////////////////////

tResult cReadWriteOps<cMs3dTriangle>::Read(IReader * pReader, cMs3dTriangle * pTriangle)
{
   Assert(pReader != NULL);
   Assert(pTriangle != NULL);

   tResult result = E_FAIL;

   do
   {
      if (pReader->Read(&pTriangle->m_flags, sizeof(pTriangle->m_flags)) != S_OK
         || pReader->Read(pTriangle->m_vertexIndices, sizeof(pTriangle->m_vertexIndices)) != S_OK
         || pReader->Read(pTriangle->m_vertexNormals, sizeof(pTriangle->m_vertexNormals)) != S_OK
         || pReader->Read(pTriangle->m_s, sizeof(pTriangle->m_s)) != S_OK
         || pReader->Read(pTriangle->m_t, sizeof(pTriangle->m_t)) != S_OK
         || pReader->Read(&pTriangle->m_smoothingGroup, sizeof(pTriangle->m_smoothingGroup)) != S_OK
         || pReader->Read(&pTriangle->m_groupIndex, sizeof(pTriangle->m_groupIndex)) != S_OK)
      {
         break;
      }

      result = S_OK;
   }
   while (0);

   return result;
}


///////////////////////////////////////////////////////////////////////////////
