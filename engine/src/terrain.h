/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_TERRAIN_H)
#define INCLUDED_TERRAIN_H

#include "terrainapi.h"

#include "vec2.h"
#include "vec3.h"
#include "techstring.h"

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainModel
//

class cTerrainModel : public cComObject<IMPLEMENTS(ITerrainModel)>
{
public:
   cTerrainModel();
   ~cTerrainModel();

   tResult Read(IReader * pReader);
   tResult Write(IWriter * pWriter);

   virtual tResult GetDimensions(uint * pxd, uint * pzd) const;
   virtual tResult GetExtents(uint * px, uint * pz) const;
   virtual tResult GetTileSet(IEditorTileSet * * ppTileSet);
   virtual const tTerrainQuads & GetTerrainQuads() const;
   virtual tTerrainQuads::const_iterator BeginTerrainQuads() const;
   virtual tTerrainQuads::const_iterator EndTerrainQuads() const;
   virtual tResult AddTerrainModelListener(ITerrainModelListener * pListener);
   virtual tResult RemoveTerrainModelListener(ITerrainModelListener * pListener);
   virtual tResult SetTileTerrain(uint tx, uint tz, uint terrain, uint * pFormer);
   virtual tResult GetTileIndices(float x, float z, uint * pix, uint * piz) const;
   virtual tResult GetTileVertices(uint tx, uint tz, tVec3 vertices[4]) const;

   tResult Init(const cMapSettings & mapSettings);
   static tResult InitQuads(uint nTilesX, uint nTilesZ, IHeightMap * pHeightMap, tTerrainQuads * pQuads);
   tResult RegenerateChunks();

private:
   void NotifyListeners();

   typedef std::vector<ITerrainModelListener *> tListeners;
   tListeners m_listeners;

   uint m_tileSize; // dimensions of a single tile in terrain space
   uint m_nTilesX, m_nTilesZ; // # of tiles in the x,z directions

   tTerrainQuads m_terrainQuads;

   cStr m_tileSetName;
   cAutoIPtr<IEditorTileSet> m_pTileSet;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TERRAIN_H
