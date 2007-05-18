///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MS3DREAD_H
#define INCLUDED_MS3DREAD_H

#include "ms3d.h"
#include "ms3dvertex.h"

#include "tech/readwriteapi.h"
#include "tech/vec3.h"

#include <map>
#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dVertexMapper
//

class cMs3dVertexMapper
{
public:
   cMs3dVertexMapper(const ms3d_vertex_t * pVertices, size_t nVertices);
   cMs3dVertexMapper(const std::vector<ms3d_vertex_t> & vertices);

   uint MapVertex(uint originalIndex, const float normal[3], float s, float t);

   const void * GetVertexData() const;
   uint GetVertexCount() const;

private:
   uint m_nOriginalVertices;
   std::vector<sMs3dVertex> m_vertices;
   std::vector<bool> m_haveVertex;
   std::map<uint, uint> m_remap;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MS3DREAD_H
