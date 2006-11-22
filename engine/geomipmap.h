/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GEOMIPMAP_H
#define INCLUDED_GEOMIPMAP_H

#include "engine/terrainapi.h"
#include "engine/saveloadapi.h"

#include "tech/globalobjdef.h"

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGMMTerrainGrid
//

class cGMMTerrainGrid
{
public:

private:
   std::vector<tVec3> m_vertices;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGMMTerrainBlock
//

class cGMMTerrainBlock
{
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGMMTerrain
//

class cGMMTerrain : public cComObject4<IMPLEMENTS(ITerrainModel),
                                       IMPLEMENTS(ITerrainRenderer),
                                       IMPLEMENTS(IGlobalObject),
                                       IMPLEMENTS(ISaveLoadParticipant)>
{
   enum
   {
      kTerrainBlockSize = 17, // should be 2^n + 1
   };

public:
   cGMMTerrain();
   ~cGMMTerrain();

   DECLARE_NAME(GMMTerrain)
   DECLARE_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   // ITerrainModel methods
   virtual tResult Initialize(const cTerrainSettings & terrainSettings);
   virtual tResult Clear();

   virtual tResult GetTerrainSettings(cTerrainSettings * pTerrainSettings) const;

   virtual tResult AddTerrainModelListener(ITerrainModelListener * pListener);
   virtual tResult RemoveTerrainModelListener(ITerrainModelListener * pListener);

   virtual tResult EnumTerrainQuads(IEnumTerrainQuads * * ppEnum);
   virtual tResult EnumTerrainQuads(uint xStart, uint xEnd, uint zStart, uint zEnd, IEnumTerrainQuads * * ppEnum);

   virtual tResult GetVertexFromHitTest(const cRay & ray, HTERRAINVERTEX * phVertex) const;
   virtual tResult GetVertexPosition(HTERRAINVERTEX hVertex, tVec3 * pPosition) const;
   virtual tResult ChangeVertexElevation(HTERRAINVERTEX hVertex, float elevDelta);
   virtual tResult SetVertexElevation(HTERRAINVERTEX hVertex, float elevation);

   virtual tResult GetQuadFromHitTest(const cRay & ray, HTERRAINQUAD * phQuad) const;
   virtual tResult SetQuadTile(HTERRAINQUAD hQuad, uint tile);
   virtual tResult GetQuadTile(HTERRAINQUAD hQuad, uint * pTile) const;
   virtual tResult GetQuadCorners(HTERRAINQUAD hQuad, tVec3 corners[4]) const;
   virtual tResult GetQuadNeighbors(HTERRAINQUAD hQuad, HTERRAINQUAD neighbors[8]) const;

   virtual tResult GetPointOnTerrain(float nx, float nz, tVec3 * pLocation) const;

   // ITerrainRenderer methods
   virtual void SetTilesPerChunk(uint tilesPerChunk);
   virtual uint GetTilesPerChunk() const;

   virtual tResult EnableBlending(bool bEnable);

   virtual void Render();

   virtual tResult HighlightTerrainQuad(HTERRAINQUAD hQuad);
   virtual tResult HighlightTerrainVertex(HTERRAINVERTEX hVertex);
   virtual tResult ClearHighlight();

   // ISaveLoadParticipant methods
   virtual tResult Save(IWriter *);
   virtual tResult Load(IReader *, int version);
   virtual void Reset();

private:
   tResult GetTileIndices(float x, float z, uint * pix, uint * piz) const;
   tResult GetQuadCorners(uint quadx, uint quadz, tVec3 corners[4]) const;

   cTerrainSettings m_terrainSettings;

   std::vector<tVec3> m_vertices;
   //tUints m_quadTiles;
};


/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GEOMIPMAP_H
