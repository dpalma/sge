/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_TERRAIN_H)
#define INCLUDED_TERRAIN_H

#include "comtools.h"
#include "vec2.h"
#include "vec3.h"
#include "str.h"

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

F_DECLARE_INTERFACE(IEditorTileSet);
F_DECLARE_INTERFACE(IMaterial);
F_DECLARE_INTERFACE(IRenderDevice);
F_DECLARE_INTERFACE(IVertexDeclaration);
F_DECLARE_INTERFACE(IReader);
F_DECLARE_INTERFACE(IWriter);
class cHeightMap;

class cTerrain;
class cTerrainChunk;

const uint kInvalidUintValue = ~0;
const uint kInvalidTerrain = ~0;

/////////////////////////////////////////////////////////////////////////////

struct sTerrainVertex
{
   tVec2 uv1;
   uint32 color;
   tVec3 pos;
};

tResult TerrainVertexDeclarationCreate(IRenderDevice * pRenderDevice,
                                       IVertexDeclaration * * ppVertexDecl);

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrain
//
// Simple terrain data class. For rendering tile-based terrain with artist-
// generated transition tiles. Not a LOD terrain renderer implementing the
// ROAM algorithm or anything like that.

const int kDefaultStepSize = 32;
const int kTilesPerChunk = 32;

struct sTerrainQuad
{
   uint tile;
   sTerrainVertex verts[4];
};

typedef std::vector<sTerrainQuad> tTerrainQuads;

class cTerrain
{
public:
   cTerrain();
   ~cTerrain();

   tResult Read(IReader * pReader);
   tResult Write(IWriter * pWriter);

   bool Create(uint xDim, uint zDim, int stepSize, IEditorTileSet * pTileSet,
      uint defaultTile, cHeightMap * pHeightMap);

   void GetDimensions(uint * pxd, uint * pzd) const;
   void GetExtents(uint * px, uint * pz) const;

   void GetTileIndices(float x, float z, uint * pix, uint * piz);

   tResult Render(IRenderDevice * pRenderDevice);

   uint SetTileTerrain(uint tx, uint tz, uint terrain);
   tResult GetTileVertices(uint tx, uint tz, tVec3 vertices[4]) const;

protected:
   void InitializeVertices(uint xDim, uint zDim, int stepSize, cHeightMap * pHeightMap);
   bool CreateTerrainChunks();

private:
   uint m_xDim, m_zDim;
   uint m_tileSize; // dimensions of a single tile in terrain space

   uint m_xChunks, m_zChunks; // # of terrain chunks in the x, z directions

   std::vector<sTerrainVertex> m_vertices;

   tTerrainQuads m_terrainQuads;

   cStr m_tileSetName;
   cAutoIPtr<IEditorTileSet> m_pTileSet;

   cAutoIPtr<IMaterial> m_pMaterial;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainChunk
//

class cTerrainChunk
{
public:
   cTerrainChunk();
   ~cTerrainChunk();

private:
   cAutoIPtr<IMaterial> m_pMaterial;
};


/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TERRAIN_H
