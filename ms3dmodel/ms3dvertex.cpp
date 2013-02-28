///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ms3dmodel/ms3dvertex.h"
#include "ms3dmodel/ms3dflags.h"

#include "engine/modeltypes.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dVertex
//

///////////////////////////////////////

cMs3dVertex::cMs3dVertex()
 : m_boneId(-1)
 , m_s(0)
 , m_t(0)
 , m_validFields(None)
{
   memset(m_vertex, 0, sizeof(m_vertex));
   memset(m_normal, 0, sizeof(m_normal));
}

///////////////////////////////////////

cMs3dVertex::cMs3dVertex(float x, float y, float z, int8 boneId)
 : m_boneId(boneId)
 , m_s(0)
 , m_t(0)
 , m_validFields(Position | BoneId)
{
   m_vertex[0] = x;
   m_vertex[1] = y;
   m_vertex[2] = z;
   memset(m_normal, 0, sizeof(m_normal));
}

///////////////////////////////////////

cMs3dVertex::cMs3dVertex(float x, float y, float z, float s, float t, int8 boneId)
 : m_boneId(boneId)
 , m_s(s)
 , m_t(t)
 , m_validFields(Position | TexCoords | BoneId)
{
   m_vertex[0] = x;
   m_vertex[1] = y;
   m_vertex[2] = z;
   memset(m_normal, 0, sizeof(m_normal));
}

///////////////////////////////////////

cMs3dVertex::cMs3dVertex(const cMs3dVertex & other)
 : m_boneId(other.m_boneId)
 , m_s(other.m_s)
 , m_t(other.m_t)
 , m_validFields(other.m_validFields)
{
   memcpy(m_vertex, other.m_vertex, sizeof(m_vertex));
   memcpy(m_normal, other.m_normal, sizeof(m_normal));
}

///////////////////////////////////////

cMs3dVertex::~cMs3dVertex()
{
}

///////////////////////////////////////

const cMs3dVertex & cMs3dVertex::operator =(const cMs3dVertex & other)
{
   m_boneId = other.m_boneId;
   memcpy(m_vertex, other.m_vertex, sizeof(m_vertex));
   memcpy(m_normal, other.m_normal, sizeof(m_normal));
   m_s = other.m_s;
   m_t = other.m_t;
   m_validFields = other.m_validFields;
   return *this;
}

///////////////////////////////////////

void cMs3dVertex::ToModelVertex(sModelVertex * pModelVertex) const
{
   if (HasPosition())
   {
      pModelVertex->pos = GetPosition();
   }

   if (HasBoneId())
   {
      pModelVertex->bone = GetBone();
   }

   if (HasNormal())
   {
      pModelVertex->normal = GetNormal();
   }

   if (HasTexCoords())
   {
      pModelVertex->u = GetS();
      pModelVertex->v = GetT();
   }
}

///////////////////////////////////////

tResult cReadWriteOps<cMs3dVertex>::Read(IReader * pReader, cMs3dVertex * pVertex)
{
   Assert(pReader != NULL);
   Assert(pVertex != NULL);

   tResult result = E_FAIL;

   do
   {
      byte ignoreFlags, ignoreReferenceCount;

      if (pReader->Read(&ignoreFlags, sizeof(ignoreFlags)) != S_OK
         || pReader->Read(pVertex->m_vertex, sizeof(pVertex->m_vertex)) != S_OK
         || pReader->Read(&pVertex->m_boneId, sizeof(pVertex->m_boneId)) != S_OK
         || pReader->Read(&ignoreReferenceCount, sizeof(ignoreReferenceCount)) != S_OK)
      {
         break;
      }

      pVertex->m_validFields |= cMs3dVertex::Position | cMs3dVertex::BoneId;

      result = S_OK;
   }
   while (0);

   return result;
}


///////////////////////////////////////////////////////////////////////////////
