///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_VERTEXMAPPER_H
#define INCLUDED_VERTEXMAPPER_H

#include "ms3dmodel/ms3dvertex.h"

#include "engine/modeltypes.h"

#include <map>
#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cVertexMapper
//

class cVertexMapper
{
public:
   cVertexMapper(const std::vector<cMs3dVertex> & vertices);

   uint MapVertex(uint originalIndex, float nx, float ny, float nz, float s, float t);
   uint MapVertex(uint originalIndex, const float normal[3], float s, float t);

   const std::vector<sModelVertex> & GetMappedVertices() const;

private:
   uint m_nOriginalVertices;
   std::vector<sModelVertex> m_vertices;
   std::vector<bool> m_haveVertex;
   std::map<uint, uint> m_remap;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_VERTEXMAPPER_H
