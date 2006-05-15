/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_TERRAINRENDER_H)
#define INCLUDED_TERRAINRENDER_H

#include "terrainapi.h"
#include "renderapi.h"
#include "saveloadapi.h"

#include "globalobjdef.h"
#include "vec2.h"
#include "vec3.h"
#include "matrix4.h"
#include "techstring.h"

#include <map>
#include <set>
#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class cTerrainChunk;

typedef std::map<HTERRAINQUAD, int> tQuadVertexMap;

/////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cRange
//

template <typename T>
class cRange
{
public:
   cRange()
   {
   }

   cRange(T start, T end)
     : m_start(start), m_end(end)
   {
   }

   cRange(const cRange & other)
     : m_start(other.m_start), m_end(other.m_end)
   {
   }

   const cRange & operator =(const cRange & other)
   {
      m_start = other.m_start;
      m_end = other.m_end;
   }

   T GetStart() const
   {
      return m_start;
   }

   T GetEnd() const
   {
      return m_end;
   }

   T GetLength(bool bIncludeEnd = false) const
   {
      return bIncludeEnd ? (m_end - m_start + 1) : (m_end - m_start);
   }

   T GetPrev(T t, T bound, T oobval) const
   {
      if ((t > GetStart()) || ((t == GetStart()) && (GetStart() > bound)))
      {
         return t - 1;
      }
      else
      {
         return oobval;
      }
   }

   T GetNext(T t, T bound, T oobval, bool bIncludeEnd = false) const
   {
      T end = bIncludeEnd ? GetEnd() : GetEnd() - 1;
      if ((t < end) || ((t == end) && (GetEnd() < bound)))
      {
         return t + 1;
      }
      else 
      {
         return oobval;
      }
   }

private:
   T m_start, m_end;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainRenderer
//

class cTerrainRenderer : public cComObject3<IMPLEMENTS(ITerrainRenderer),
                                            IMPLEMENTS(IGlobalObject),
                                            IMPLEMENTS(ISaveLoadParticipant)>
{
public:
   cTerrainRenderer(bool bForEditor);
   ~cTerrainRenderer();

   DECLARE_NAME(TerrainRenderer)
   DECLARE_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

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
   bool RunningInEditor() const { return m_bInEditor; }

   bool IsBlendingEnabled() const;

   void RegenerateChunks();
   void ClearChunks();

   class cTerrainModelListener : public cComObject<IMPLEMENTS(ITerrainModelListener)>
   {
   public:
      cTerrainModelListener(cTerrainRenderer * pOuter);
      virtual void DeleteThis() { /* Do not delete; Non-pointer member of cTerrainRenderer */ }
      virtual void OnTerrainInitialize();
      virtual void OnTerrainClear();
      virtual tResult OnTerrainTileChanging(HTERRAINQUAD hQuad, uint oldTile, uint newTile);
      virtual void OnTerrainTileChanged(HTERRAINQUAD hQuad);
      virtual void OnTerrainElevationChange(HTERRAINVERTEX hVertex);
   private:
      cTerrainRenderer * m_pOuter;
   };
   friend class cTerrainModelListener;
   cTerrainModelListener m_terrainModelListener;

   uint m_nTilesPerChunk;
   typedef std::vector<cTerrainChunk *> tChunks;
   tChunks m_chunks;

   cTerrainChunk * m_pWholeTerrainChunk;

   bool m_bInEditor; // Is this terrain renderer running in the editor
   bool m_bEnableBlending;
   bool m_bTerrainChanged; // terrain changed while blending disabled?

   typedef std::map<uint, uint> tTileCountMap;
   tTileCountMap m_tileCountMap;

   uint m_baseTile;

   HTERRAINQUAD m_highlightQuad;
   HTERRAINVERTEX m_highlightVertex;
};

////////////////////////////////////////

bool cTerrainRenderer::IsBlendingEnabled() const
{
   return m_bEnableBlending;
}


/////////////////////////////////////////////////////////////////////////////

tResult BuildSplatAlphaMap(uint splatTile, const cRange<uint> xRange, const cRange<uint> zRange, uint * pAlphaMapId);

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSplatBuilder
//

class cSplatBuilder
{
   cSplatBuilder(const cSplatBuilder &);
   void operator =(const cSplatBuilder &);

public:
   cSplatBuilder(uint tile);
   ~cSplatBuilder();

   inline uint GetTile() const { return m_tile; }

   void AddQuad(HTERRAINQUAD hQuad);

   tResult BuildIndexBuffer(const tQuadVertexMap & qvm, std::vector<uint> * pIndices) const;

private:
   uint m_tile;
   std::set<HTERRAINQUAD> m_quads;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSplat
//

class cSplat
{
   cSplat(const cSplat &);
   void operator =(const cSplat &);

public:
   cSplat(const cStr & texture, uint alphaMap, const std::vector<uint> indices, ePrimitiveType primitive);
   ~cSplat();

   const cStr & GetTexture() const { return m_texture; }
   uint GetAlphaMap() const { return m_alphaMap; }
   const uint * GetIndexPtr() const { return &m_indices[0]; }
   uint GetIndexCount() const { return m_indices.size(); }
   ePrimitiveType GetPrimitive() const { return m_primitive; }

private:
   const cStr m_texture;
   const uint m_alphaMap;
   const std::vector<uint> m_indices;
   ePrimitiveType m_primitive;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainChunk
//

struct sTerrainVertex
{
   tVec2 uv1;
   tVec2 uv2;
   tVec3 pos;
};

class cTerrainChunk
{
   cTerrainChunk(const cTerrainChunk &);
   void operator =(const cTerrainChunk &);

public:
   cTerrainChunk();
   ~cTerrainChunk();

   static tResult Create(const cRange<uint> xRange, const cRange<uint> zRange, ITerrainTileSet * pTileSet, uint baseTile, bool bNoBlending, cTerrainChunk * * ppChunk);

   tResult BuildVertexBuffer(const cRange<uint> xRange, const cRange<uint> zRange, tQuadVertexMap * pQuadVertexMap);
   void BuildSplats(const cRange<uint> xRange, const cRange<uint> zRange, ITerrainTileSet * pTileSet, uint baseTile, bool bNoBlending);

   void Render();

private:
   typedef std::vector<sTerrainVertex> tVertices;
   tVertices m_vertices;

   typedef std::vector<cSplat *> tSplats;
   tSplats m_splats;

   tQuadVertexMap m_quadVertexMap;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TERRAINRENDER_H
