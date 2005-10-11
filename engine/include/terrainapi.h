/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_TERRAINAPI_H)
#define INCLUDED_TERRAINAPI_H

#include "enginedll.h"
#include "comtools.h"
#include "vec3.h"
#include "techstring.h"
#include "readwriteapi.h"

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////

F_DECLARE_INTERFACE(ITerrainRenderer);
F_DECLARE_INTERFACE(IEnumTerrainQuads);
F_DECLARE_INTERFACE(ITerrainModel);
F_DECLARE_INTERFACE(ITerrainModelListener);
F_DECLARE_INTERFACE(ITerrainTileSet);
F_DECLARE_INTERFACE(IHeightMap);

class cRay;


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

ENGINE_API tResult TerrainRendererCreate();
ENGINE_API tResult TerrainRendererCreateForEditor();


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

class ENGINE_API cTerrainSettings
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

   void SetInitialTile(int initialTile);
   int GetInitialTile() const;

   void SetHeightData(eTerrainHeightData heightData);
   eTerrainHeightData GetHeightData() const;

   void SetHeightMap(const tChar * pszHeightMap);
   const tChar * GetHeightMap() const;

   void SetHeightMapScale(float scale);
   float GetHeightMapScale() const;

private:
   uint m_tileSize;
   uint m_nTilesX, m_nTilesZ;
   cStr m_tileSet;
   int m_initialTile;
   eTerrainHeightData m_heightData;
   cStr m_heightMap;
   float m_heightMapScale;
};


/////////////////////////////////////////////////////////////////////////////

DECLARE_HANDLE(HTERRAINQUAD);
DECLARE_HANDLE(HTERRAINVERTEX);

const HTERRAINQUAD INVALID_HTERRAINQUAD = reinterpret_cast<HTERRAINQUAD>(~0);
const HTERRAINVERTEX INVALID_HTERRAINVERTEX = reinterpret_cast<HTERRAINVERTEX>(~0);


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEnumTerrainQuads
//

interface IEnumTerrainQuads : IUnknown
{
   virtual tResult Next(ulong count, HTERRAINQUAD * pQuads, ulong * pnQuads) = 0;
   virtual tResult Skip(ulong count) = 0;
   virtual tResult Reset() = 0;
   virtual tResult Clone(IEnumTerrainQuads * * ppEnum) = 0;
};


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ITerrainModel
//

interface ITerrainModel : IUnknown
{
   virtual tResult Initialize(const cTerrainSettings & terrainSettings) = 0;
   virtual tResult Clear() = 0;

   virtual tResult GetTerrainSettings(cTerrainSettings * pTerrainSettings) const = 0;

   virtual tResult AddTerrainModelListener(ITerrainModelListener * pListener) = 0;
   virtual tResult RemoveTerrainModelListener(ITerrainModelListener * pListener) = 0;

   virtual tResult EnumTerrainQuads(IEnumTerrainQuads * * ppEnum) = 0;
   virtual tResult EnumTerrainQuads(uint xStart, uint xEnd, uint zStart, uint zEnd, IEnumTerrainQuads * * ppEnum) = 0;

   virtual tResult GetVertexFromHitTest(const cRay & ray, HTERRAINVERTEX * phVertex) const = 0;

   virtual tResult GetVertexPosition(HTERRAINVERTEX hVertex, tVec3 * pPosition) const = 0;
   virtual tResult ChangeVertexElevation(HTERRAINVERTEX hVertex, float elevDelta) = 0;
   virtual tResult SetVertexElevation(HTERRAINVERTEX hVertex, float elevation) = 0;

   virtual tResult GetQuadFromHitTest(const cRay & ray, HTERRAINQUAD * phQuad) const = 0;

   virtual tResult SetQuadTile(HTERRAINQUAD hQuad, uint tile) = 0;
   virtual tResult GetQuadTile(HTERRAINQUAD hQuad, uint * pTile) const = 0;

   virtual tResult GetQuadCorners(HTERRAINQUAD hQuad, tVec3 corners[4]) const = 0;

   virtual tResult GetQuadNeighbors(HTERRAINQUAD hQuad, HTERRAINQUAD neighbors[8]) const = 0;
};

////////////////////////////////////////

ENGINE_API tResult TerrainModelCreate();


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ITerrainModelListener
//

interface ITerrainModelListener : IUnknown
{
   virtual void OnTerrainInitialize() = 0;
   virtual void OnTerrainClear() = 0;
   virtual tResult OnTerrainTileChanging(HTERRAINQUAD hQuad, uint oldTile, uint newTile) = 0;
   virtual void OnTerrainTileChanged(HTERRAINQUAD hQuad) = 0;
   virtual void OnTerrainElevationChange(HTERRAINVERTEX hVertex) = 0;
};


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ITerrainTileSet
//

#define kRT_TerrainTileSet _T("TerrainTileSet") // resource type
ENGINE_API void TerrainRegisterResourceFormats();

enum eTerrainTileFlags
{
   kTTF_None            = 0,
   kTTF_Impassible      = (1<<0),
};

interface ITerrainTileSet : IUnknown
{
   virtual tResult GetName(cStr * pName) const = 0;

   virtual tResult GetTileCount(uint * pTileCount) const = 0;
   virtual tResult GetTileTexture(uint iTile, cStr * pTexture) const = 0;
   virtual tResult GetTileName(uint iTile, cStr * pName) const = 0;
   virtual tResult GetTileFlags(uint iTile, uint * pFlags) const = 0;
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

ENGINE_API tResult HeightMapCreateFixed(float heightValue, IHeightMap * * ppHeightMap);
ENGINE_API tResult HeightMapLoad(const tChar * pszHeightData, IHeightMap * * ppHeightMap);

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TERRAINAPI_H
