///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_TILEDGROUND_H
#define INCLUDED_TILEDGROUND_H

#include "comtools.h"

#include "vec2.h"
#include "vec3.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

class cHeightMap;

F_DECLARE_INTERFACE(IEditorTileSet);

F_DECLARE_INTERFACE(IRenderDevice);
F_DECLARE_INTERFACE(IVertexBuffer);
F_DECLARE_INTERFACE(IIndexBuffer);
F_DECLARE_INTERFACE(IMaterial);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTiledGround
//

struct sTerrainVertex
{
   tVec2 uv1;
//   tVec2 uv2;
//   tVec2 uv3;
//   tVec2 uv4;
   tVec3 rgb;
   tVec3 pos;
};

class cTiledGround
{
public:
   cTiledGround();
   ~cTiledGround();

   bool Init(uint xDim, uint zDim, IEditorTileSet * pTileSet, uint defaultTile, cHeightMap * pHeightMap);
   bool CreateBuffers(IRenderDevice * pRenderDevice);

   size_t GetVertexCount() const { return m_vertices.size(); }
   size_t GetIndexCount() const { return m_nIndices; }

   IVertexBuffer * AccessVertexBuffer() { return m_pVertexBuffer; }
   IIndexBuffer * AccessIndexBuffer() { return m_pIndexBuffer; }
   IMaterial * AccessMaterial() { return m_pMaterial; }

   inline uint GetXDimension() const { return m_xDim; }
   inline uint GetZDimension() const { return m_zDim; }

private:
   uint m_xDim, m_zDim;
   cAutoIPtr<IVertexBuffer> m_pVertexBuffer;
   cAutoIPtr<IIndexBuffer> m_pIndexBuffer;
   cAutoIPtr<IMaterial> m_pMaterial;
   int m_nIndices;
   std::vector<sTerrainVertex> m_vertices;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TILEDGROUND_H
