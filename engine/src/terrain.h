/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_TERRAIN_H)
#define INCLUDED_TERRAIN_H

#include "terrainapi.h"
#include "saveloadapi.h"

#include "globalobjdef.h"

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef std::vector<tVec3> tVec3s;
typedef std::vector<uint> tUints;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEnumTerrainQuads
//

class cEnumTerrainQuads : public cComObject<IMPLEMENTS(IEnumTerrainQuads)>
{
   cEnumTerrainQuads(const cEnumTerrainQuads &);
   void operator =(const cEnumTerrainQuads &);

   friend class cTerrainModel;

   cEnumTerrainQuads(uint xStart, uint xEnd, uint zStart, uint zEnd);
   ~cEnumTerrainQuads();

public:
   virtual tResult Next(ulong count, HTERRAINQUAD * pQuads, ulong * pnQuads);
   virtual tResult Skip(ulong count);
   virtual tResult Reset();
   virtual tResult Clone(IEnumTerrainQuads * * ppEnum);

private:
   uint m_xStart, m_xEnd, m_zStart, m_zEnd;
   uint m_x, m_z;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainModel
//

class cTerrainModel : public cComObject3<IMPLEMENTS(ITerrainModel),
                                         IMPLEMENTS(IGlobalObject),
                                         IMPLEMENTS(ISaveLoadParticipant)>
{
public:
   cTerrainModel();
   ~cTerrainModel();

   DECLARE_NAME(TerrainModel)
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

   tResult SetQuadTile(uint quadx, uint quadz, uint tile, uint * pFormer);
   tResult GetQuadTile(uint quadx, uint quadz, uint * pTile) const;
   tResult GetTileIndices(float x, float z, uint * pix, uint * piz) const;
   tResult GetQuadCorners(uint quadx, uint quadz, tVec3 corners[4]) const;

   // ISaveLoadParticipant methods
   virtual tResult Save(IWriter *);
   virtual tResult Load(IReader *, int version);

private:
   void NotifyListeners(void (ITerrainModelListener::*pfnListenerMethod)());

   typedef std::vector<ITerrainModelListener *> tListeners;
   tListeners m_listeners;

   cTerrainSettings m_terrainSettings;

   tVec3s m_vertices;
   tUints m_quadTiles;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS cTerrainTileSet
//

class cTerrainTileSet : public cComObject<IMPLEMENTS(ITerrainTileSet)>
{
public:
   cTerrainTileSet(const tChar * pszName);
   ~cTerrainTileSet();

   virtual tResult GetName(cStr * pName) const;

   virtual tResult GetTileCount(uint * pTileCount) const;
   virtual tResult GetTileTexture(uint iTile, cStr * pTexture) const;
   virtual tResult GetTileName(uint iTile, cStr * pName) const;
   virtual tResult GetTileFlags(uint iTile, uint * pFlags) const;

private:
   friend void * TerrainTileSetFromXml(void * pData, int dataLength, void * param);
   tResult AddTile(const tChar * pszName, const tChar * pszTexture, uint flags);

   /////////////////////////////////////

   cStr m_name; // name of the tile set

   struct sTerrainTileInfo
   {
      cStr name;
      cStr texture;
      uint flags;
   };

   std::vector<sTerrainTileInfo> m_tiles;
};


/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TERRAIN_H
