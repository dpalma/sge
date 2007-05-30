///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "vertexmapper.h"

#include "ms3dmodel/ms3dflags.h"

#include "tech/vec3.h"
#include "tech/techhash.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include "tech/dbgalloc.h" // must be last header

using namespace std;


///////////////////////////////////////////////////////////////////////////////

static bool operator ==(const tVec3 & v1, const tVec3 & v2)
{
   return (v1.x == v2.x) && (v1.y == v2.y) && (v1.z == v2.z);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cVertexMapper
//

///////////////////////////////////////

cVertexMapper::cVertexMapper(const vector<cMs3dVertex> & vertices)
 : m_nOriginalVertices(vertices.size())
 , m_vertices(vertices.size())
 , m_haveVertex(vertices.size(), false)
{
   vector<cMs3dVertex>::const_iterator iter = vertices.begin();
   vector<cMs3dVertex>::const_iterator end = vertices.end();
   for (uint i = 0; iter != end; iter++, i++)
   {
      iter->ToModelVertex(&m_vertices[i]);
   }
}

///////////////////////////////////////

uint cVertexMapper::MapVertex(uint originalIndex, float nx, float ny, float nz, float s, float t)
{
   float normal[3] = { nx, ny, nz };
   return MapVertex(originalIndex, normal, s, t);
}

///////////////////////////////////////

uint cVertexMapper::MapVertex(uint originalIndex, const float normal[3], float s, float t)
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
      sModelVertex newVertex = m_vertices[originalIndex];
      newVertex.normal = normal;
      newVertex.u = s;
      newVertex.v = t;
      uint h = Hash(&newVertex, sizeof(newVertex));
      map<uint, uint>::iterator f = m_remap.find(h);
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
         m_remap.insert(make_pair(h, newIndex));
         return newIndex;
      }
   }
}

///////////////////////////////////////

uint cVertexMapper::MapVertex(uint originalIndex, float nx, float ny, float nz)
{
   float normal[3] = { nx, ny, nz };
   return MapVertex(originalIndex, normal);
}

///////////////////////////////////////

uint cVertexMapper::MapVertex(uint originalIndex, const float normal[3])
{
   Assert(originalIndex < m_nOriginalVertices);
   Assert(m_nOriginalVertices == m_haveVertex.size());

   if (!m_haveVertex[originalIndex])
   {
      m_haveVertex[originalIndex] = true;
      m_vertices[originalIndex].normal = normal;
      return originalIndex;
   }
   else if (m_vertices[originalIndex].normal == normal)
   {
      return originalIndex;
   }
   else
   {
      sModelVertex newVertex = m_vertices[originalIndex];
      newVertex.normal = normal;
      uint h = Hash(&newVertex, sizeof(newVertex));
      map<uint, uint>::iterator f = m_remap.find(h);
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
         m_remap.insert(make_pair(h, newIndex));
         return newIndex;
      }
   }
}

///////////////////////////////////////

const std::vector<sModelVertex> & cVertexMapper::GetMappedVertices() const
{
   return m_vertices;
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

SUITE(Ms3dVertexMapper)
{
   TEST(RemapByNormalAndTexCoords)
   {
      vector<cMs3dVertex> verts;
      verts.push_back(cMs3dVertex(1, 1, 1, 0));

      cVertexMapper vm(verts);

      // First reference to vertex zero
      CHECK_EQUAL(0, vm.MapVertex(0, .5, .5, .5, .1f, .1f));

      // Refer to vertex zero, but with a different normal
      CHECK_EQUAL(1, vm.MapVertex(0, .4f, .4f, .4f, .1f, .1f));

      // Refer to vertex zero again, but with a different texture coordinates
      CHECK_EQUAL(2, vm.MapVertex(0, .5, .5, .5, .2f, .2f));

      // Make sure referring to vertex zero again with same info
      // as vertex one maps to vertex one
      CHECK_EQUAL(1, vm.MapVertex(0, .4f, .4f, .4f, .1f, .1f));
   }

   TEST(RemapByNormalOnly)
   {
      vector<cMs3dVertex> verts;
      verts.push_back(cMs3dVertex(1, 1, 1, .05f, .05f, -1));

      cVertexMapper vm(verts);

      // First reference to vertex zero
      CHECK_EQUAL(0, vm.MapVertex(0, .5, .5, .5));

      // Refer to vertex zero, but with a different normal
      CHECK_EQUAL(1, vm.MapVertex(0, .4f, .4f, .4f));

      // Refer to vertex zero again with same info
      CHECK_EQUAL(1, vm.MapVertex(0, .4f, .4f, .4f));
   }
}

#endif // HAVE_UNITTESTPP

///////////////////////////////////////////////////////////////////////////////
