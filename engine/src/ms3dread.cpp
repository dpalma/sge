///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ms3dread.h"

#include "matrix4.h"
#include "vec3.h"
#include "techhash.h"

#include <map>

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dVertexMapper
//

///////////////////////////////////////

cMs3dVertexMapper::cMs3dVertexMapper(const ms3d_vertex_t * pVertices, size_t nVertices)
 : m_nOriginalVertices(nVertices)
{
   Assert(pVertices != NULL);
   Assert(nVertices > 0);

   m_vertices.resize(nVertices);
   m_haveVertex.resize(nVertices, false);

   for (uint i = 0; i < nVertices; i++)
   {
      m_vertices[i].pos = pVertices[i].vertex;
      m_vertices[i].bone = pVertices[i].boneId;
   }
}

///////////////////////////////////////

cMs3dVertexMapper::cMs3dVertexMapper(const std::vector<ms3d_vertex_t> & vertices)
 : m_nOriginalVertices(vertices.size())
 , m_vertices(vertices.size())
 , m_haveVertex(vertices.size(), false)
{
   std::vector<ms3d_vertex_t>::const_iterator iter = vertices.begin();
   std::vector<ms3d_vertex_t>::const_iterator end = vertices.end();
   for (uint i = 0; iter != end; iter++, i++)
   {
      m_vertices[i].pos = iter->vertex;
      m_vertices[i].bone = iter->boneId;
   }
}

///////////////////////////////////////

static bool operator ==(const tVec3 & v1, const tVec3 & v2)
{
   return (v1.x == v2.x) && (v1.y == v2.y) && (v1.z == v2.z);
}

///////////////////////////////////////

uint cMs3dVertexMapper::MapVertex(uint originalIndex, const float normal[3], float s, float t)
{
   Assert(originalIndex < m_nOriginalVertices);
   Assert(m_nOriginalVertices == m_haveVertex.size());

   // Use the complement of what is actually stored in the Milkshape file
   t = 1 - t;

   if (!m_haveVertex[originalIndex])
   {
      m_haveVertex[originalIndex] = true;
      m_vertices[originalIndex].normal = normal;
      m_vertices[originalIndex].u = s;
      m_vertices[originalIndex].v = t;
      return originalIndex;
   }
   else if ((m_vertices[originalIndex].normal == normal)
      && (m_vertices[originalIndex].u == s)
      && (m_vertices[originalIndex].v == t))
   {
      return originalIndex;
   }
   else
   {
      sMs3dVertex newVertex = m_vertices[originalIndex];
      newVertex.normal = normal;
      newVertex.u = s;
      newVertex.v = t;
      uint h = Hash(&newVertex, sizeof(newVertex));
      std::map<uint, uint>::iterator f = m_remap.find(h);
      if (f != m_remap.end())
      {
         Assert(f->second < m_vertices.size());
         return f->second;
      }
      else
      {
         // TODO: Tacking the the duplicated vertex onto the end
         // is totally not optimal with respect to vertex caches
         m_vertices.push_back(newVertex);
         size_t newIndex = m_vertices.size() - 1;
         m_remap.insert(std::make_pair(h, newIndex));
         return newIndex;
      }
   }
}

///////////////////////////////////////

const void * cMs3dVertexMapper::GetVertexData() const
{
   return reinterpret_cast<const void *>(&m_vertices[0]);
}

///////////////////////////////////////

uint cMs3dVertexMapper::GetVertexCount() const
{
   return m_vertices.size();
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dGroup
//

///////////////////////////////////////

cMs3dGroup::cMs3dGroup()
 : flags(0), materialIndex(~0)
{
}

///////////////////////////////////////

cMs3dGroup::cMs3dGroup(const cMs3dGroup & other)
 : flags(other.flags)
 , triangleIndices(other.triangleIndices.size())
 , materialIndex(other.materialIndex)
{
   memcpy(name, other.name, sizeof(name));
   std::copy(other.triangleIndices.begin(), other.triangleIndices.end(), triangleIndices.begin());
}

///////////////////////////////////////

cMs3dGroup::~cMs3dGroup()
{
}

///////////////////////////////////////

const cMs3dGroup & cMs3dGroup::operator =(const cMs3dGroup & other)
{
   flags = other.flags;
   materialIndex = other.materialIndex;
   memcpy(name, other.name, sizeof(name));
   triangleIndices.resize(other.triangleIndices.size()),
   std::copy(other.triangleIndices.begin(), other.triangleIndices.end(), triangleIndices.begin());
   return *this;
}

///////////////////////////////////////

tResult cReadWriteOps<cMs3dGroup>::Read(IReader * pReader, cMs3dGroup * pGroup)
{
   Assert(pReader != NULL);
   Assert(pGroup != NULL);

   tResult result = E_FAIL;

   do
   {
      if (pReader->Read(&pGroup->flags, sizeof(pGroup->flags)) != S_OK)
         break;

      if (pReader->Read(pGroup->name, sizeof(pGroup->name)) != S_OK)
         break;

      uint16 nTriangles;
      if (pReader->Read(&nTriangles, sizeof(nTriangles)) != S_OK)
         break;

      pGroup->triangleIndices.resize(nTriangles);

      if (pReader->Read(&pGroup->triangleIndices[0], pGroup->triangleIndices.size() * sizeof(uint16)) != S_OK)
         break;

      if (pReader->Read(&pGroup->materialIndex, sizeof(pGroup->materialIndex)) != S_OK)
         break;

      result = S_OK;
   }
   while (0);

   return result;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dJoint
//

///////////////////////////////////////

cMs3dJoint::cMs3dJoint()
 : flags(0)
{
}

///////////////////////////////////////

cMs3dJoint::cMs3dJoint(const cMs3dJoint & other)
 : flags(other.flags)
 , keyFramesRot(other.keyFramesRot.size())
 , keyFramesTrans(other.keyFramesTrans.size())
{
   memcpy(name, other.name, sizeof(name));
   memcpy(parentName, other.parentName, sizeof(parentName));
   memcpy(rotation, other.rotation, sizeof(rotation));
   memcpy(position, other.position, sizeof(position));
   std::copy(other.keyFramesRot.begin(), other.keyFramesRot.end(), keyFramesRot.begin());
   std::copy(other.keyFramesTrans.begin(), other.keyFramesTrans.end(), keyFramesTrans.begin());
}

///////////////////////////////////////

cMs3dJoint::~cMs3dJoint()
{
}

///////////////////////////////////////

tResult cReadWriteOps<cMs3dJoint>::Read(IReader * pReader, cMs3dJoint * pJoint)
{
   Assert(pReader != NULL);
   Assert(pJoint != NULL);

   tResult result = E_FAIL;

   do
   {
      uint16 nKeyFramesRot;
      uint16 nKeyFramesTrans;

      if (pReader->Read(&pJoint->flags, sizeof(pJoint->flags)) != S_OK
         || pReader->Read(pJoint->name, sizeof(pJoint->name)) != S_OK
         || pReader->Read(pJoint->parentName, sizeof(pJoint->parentName)) != S_OK
         || pReader->Read(pJoint->rotation, sizeof(pJoint->rotation)) != S_OK
         || pReader->Read(pJoint->position, sizeof(pJoint->position)) != S_OK
         || pReader->Read(&nKeyFramesRot) != S_OK
         || pReader->Read(&nKeyFramesTrans) != S_OK)
      {
         break;
      }

      if (nKeyFramesRot != nKeyFramesTrans)
      {
         break;
      }

      if (nKeyFramesRot == 0)
      {
         return S_OK;
      }

      pJoint->keyFramesRot.resize(nKeyFramesRot);
      pJoint->keyFramesTrans.resize(nKeyFramesTrans);

      if (pReader->Read(&pJoint->keyFramesRot[0], nKeyFramesRot * sizeof(ms3d_keyframe_rot_t)) != S_OK
         || pReader->Read(&pJoint->keyFramesTrans[0], nKeyFramesTrans * sizeof(ms3d_keyframe_pos_t)) != S_OK)
      {
         break;
      }

      result = S_OK;
   }
   while (0);

   return result;
}


///////////////////////////////////////////////////////////////////////////////
