/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_TERRAINAPI_H)
#define INCLUDED_TERRAINAPI_H

#include "comtools.h"
#include "vec2.h"
#include "vec3.h"
#include "techstring.h"
#include "readwriteapi.h"

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define TERRAIN_API // temporary

/////////////////////////////////////////////////////////////////////////////

F_DECLARE_INTERFACE(IEditorTileSet);

F_DECLARE_INTERFACE(ITerrainRenderer);
F_DECLARE_INTERFACE(ITerrainModel);
F_DECLARE_INTERFACE(ITerrainModelListener);
F_DECLARE_INTERFACE(IHeightMap);


/////////////////////////////////////////////////////////////////////////////

struct sTerrainVertex
{
   tVec2 uv1;
   tVec2 uv2;
   tVec3 pos;
};

struct sTerrainQuad
{
   uint tile;
   sTerrainVertex verts[4];
};

typedef std::vector<sTerrainQuad> tTerrainQuads;


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ITerrainRenderer
//

////////////////////////////////////////

namespace TerrainRendererDefaults
{
   const uint kTerrainTilesPerChunk = 32;
}

////////////////////////////////////////

interface ITerrainRenderer : IUnknown
{
   virtual void SetTilesPerChunk(uint tilesPerChunk) = 0;
   virtual uint GetTilesPerChunk() const = 0;

   virtual tResult EnableBlending(bool bEnable) = 0;

   virtual void Render() = 0;
};

////////////////////////////////////////

TERRAIN_API tResult TerrainRendererCreate();


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainSettings
//

////////////////////////////////////////

enum eTerrainHeightData
{
   kTHD_Fixed,
   kTHD_Noise,
   kTHD_HeightMap,
};

////////////////////////////////////////

namespace TerrainSettingsDefaults
{
   const uint kTerrainTileSize      = 32;
   const uint kTerrainTileCountX    = 64;
   const uint kTerrainTileCountZ    = 64;
   const eTerrainHeightData kTerrainHeightData = kTHD_Fixed;
}

////////////////////////////////////////

class TERRAIN_API cTerrainSettings
{
   friend class cReadWriteOps<cTerrainSettings>;

public:
   cTerrainSettings();
   cTerrainSettings(const cTerrainSettings & other);
   ~cTerrainSettings();

   const cTerrainSettings & operator =(const cTerrainSettings & other);

   void SetTileSize(uint tileSize);
   uint GetTileSize() const;

   void SetTileCountX(uint tileCountX);
   uint GetTileCountX() const;

   void SetTileCountZ(uint tileCountZ);
   uint GetTileCountZ() const;

   void SetTileSet(const tChar * pszTileSet);
   const tChar * GetTileSet() const;

   void SetHeightData(eTerrainHeightData heightData);
   eTerrainHeightData GetHeightData() const;

   void SetHeightMap(const tChar * pszHeightMap);
   const tChar * GetHeightMap() const;

private:
   uint m_tileSize;
   uint m_nTilesX, m_nTilesZ;
   cStr m_tileSet;
   eTerrainHeightData m_heightData;
   cStr m_heightMap;
};

////////////////////////////////////////

template <>
class cReadWriteOps<cTerrainSettings>
{
public:
   static tResult Read(IReader * pReader, cTerrainSettings * pTerrainSettings);
   static tResult Write(IWriter * pWriter, const cTerrainSettings & terrainSettings);
};


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ITerrainModel
//

interface ITerrainModel : IUnknown
{
   virtual tResult Initialize(const cTerrainSettings & terrainSettings) = 0;
   virtual tResult Clear() = 0;

   virtual tResult Read(IReader * pReader) = 0;
   virtual tResult Write(IWriter * pWriter) = 0;

   virtual tResult GetTerrainSettings(cTerrainSettings * pTerrainSettings) const = 0;

   virtual tResult AddTerrainModelListener(ITerrainModelListener * pListener) = 0;
   virtual tResult RemoveTerrainModelListener(ITerrainModelListener * pListener) = 0;

   virtual tResult GetTileSet(IEditorTileSet * * ppTileSet) = 0;

   virtual tTerrainQuads::const_iterator BeginTerrainQuads() const = 0;
   virtual tTerrainQuads::const_iterator EndTerrainQuads() const = 0;

   virtual tResult SetQuadTile(uint quadx, uint quadz, uint tile, uint * pFormer) = 0;
   virtual tResult GetQuadTile(uint quadx, uint quadz, uint * pTile) const = 0;

   /// @brief Part of hit-testing; Gets the tile indices for a given point on the 2D terrain plane
   virtual tResult GetTileIndices(float x, float z, uint * pix, uint * piz) const = 0;

   virtual tResult GetQuadVertices(uint quadx, uint quadz, sTerrainVertex verts[4]) const = 0;
};

////////////////////////////////////////

TERRAIN_API tResult TerrainModelCreate();


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ITerrainModelListener
//

interface ITerrainModelListener : IUnknown
{
   virtual void OnTerrainInitialize() = 0;
   virtual void OnTerrainClear() = 0;
   virtual void OnTerrainChange() = 0;
};


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IHeightMap
//

interface IHeightMap : IUnknown
{
   virtual float GetNormalizedHeight(float nx, float nz) const = 0;
};

////////////////////////////////////////

TERRAIN_API tResult HeightMapCreateFixed(float heightValue, IHeightMap * * ppHeightMap);
TERRAIN_API tResult HeightMapLoad(const tChar * pszHeightData, IHeightMap * * ppHeightMap);

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TERRAINAPI_H
