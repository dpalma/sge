/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_TERRAINAPI_H)
#define INCLUDED_TERRAINAPI_H

#include "comtools.h"
#include "vec2.h"
#include "vec3.h"

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

class cMapSettings;

struct sTerrainVertex
{
   tVec2 uv1;
   tVec2 uv2;
   uint32 color;
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

interface ITerrainRenderer : IUnknown
{
   virtual tResult SetModel(ITerrainModel * pTerrainModel) = 0;
   virtual tResult GetModel(ITerrainModel * * ppTerrainModel) = 0;

   virtual tResult EnableBlending(bool bEnable) = 0;
};

////////////////////////////////////////

TERRAIN_API tResult TerrainRendererCreate();


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ITerrainModel
//

interface ITerrainModel : IUnknown
{
   virtual tResult AddTerrainModelListener(ITerrainModelListener * pListener) = 0;
   virtual tResult RemoveTerrainModelListener(ITerrainModelListener * pListener) = 0;

   virtual tResult GetDimensions(uint * pxd, uint * pzd) const = 0;
   virtual tResult GetExtents(uint * px, uint * pz) const = 0;

   virtual tResult GetTileSet(IEditorTileSet * * ppTileSet) = 0;

   // sort of a hack
   virtual const tTerrainQuads & GetTerrainQuads() const = 0;

   virtual tResult SetTileTerrain(uint tx, uint tz, uint terrain, uint * pFormer) = 0;

   /// @brief Part of hit-testing; Gets the tile indices for a given point on the 2D terrain plane
   virtual tResult GetTileIndices(float x, float z, uint * pix, uint * piz) const = 0;

   /// @brief Gets points that define a quad representing the tile at the given indices
   virtual tResult GetTileVertices(uint tx, uint tz, tVec3 vertices[4]) const = 0;
};

////////////////////////////////////////

TERRAIN_API tResult TerrainModelCreate(const cMapSettings & mapSettings, ITerrainModel * * ppTerrainModel);


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ITerrainModelListener
//

interface ITerrainModelListener : IUnknown
{
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

TERRAIN_API tResult HeightMapCreateSimple(float heightValue, IHeightMap * * ppHeightMap);
TERRAIN_API tResult HeightMapLoad(const tChar * pszHeightData, IHeightMap * * ppHeightMap);

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TERRAINAPI_H
