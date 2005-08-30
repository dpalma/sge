/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_TERRAIN_H)
#define INCLUDED_TERRAIN_H

#include "terrainapi.h"

#include "vec2.h"
#include "vec3.h"
#include "techstring.h"
#include "globalobjdef.h"

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainModel
//

typedef std::vector<sTerrainQuad> tTerrainQuads;

class cTerrainModel : public cComObject2<IMPLEMENTS(ITerrainModel), IMPLEMENTS(IGlobalObject)>
{
public:
   cTerrainModel();
   ~cTerrainModel();

   DECLARE_NAME(TerrainModel)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult Initialize(const cTerrainSettings & terrainSettings);
   virtual tResult Clear();

   virtual tResult Read(IReader * pReader);
   virtual tResult Write(IWriter * pWriter);

   virtual tResult GetTerrainSettings(cTerrainSettings * pTerrainSettings) const;

   virtual tResult GetTileSet(IEditorTileSet * * ppTileSet);
   virtual tTerrainQuads::const_iterator BeginTerrainQuads() const;
   virtual tTerrainQuads::const_iterator EndTerrainQuads() const;
   virtual tResult AddTerrainModelListener(ITerrainModelListener * pListener);
   virtual tResult RemoveTerrainModelListener(ITerrainModelListener * pListener);
   virtual tResult SetQuadTile(uint quadx, uint quadz, uint tile, uint * pFormer);
   virtual tResult GetQuadTile(uint quadx, uint quadz, uint * pTile) const;
   virtual tResult GetTileIndices(float x, float z, uint * pix, uint * piz) const;
   virtual tResult GetQuadVertices(uint quadx, uint quadz, sTerrainVertex verts[4]) const;

   static tResult InitQuads(uint nTilesX, uint nTilesZ, uint tile, IHeightMap * pHeightMap, tTerrainQuads * pQuads);
   tResult RegenerateChunks();

private:
   void NotifyListeners(void (ITerrainModelListener::*pfnListenerMethod)());

   typedef std::vector<ITerrainModelListener *> tListeners;
   tListeners m_listeners;

   cTerrainSettings m_terrainSettings;

   tTerrainQuads m_terrainQuads;

   cAutoIPtr<IEditorTileSet> m_pTileSet;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TERRAIN_H
