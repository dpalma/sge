///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ms3dread.h"

#include "tech/matrix4.h"
#include "tech/vec3.h"
#include "tech/techhash.h"

#include <map>

#include "tech/dbgalloc.h" // must be last header


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
