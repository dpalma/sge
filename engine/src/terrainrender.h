/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_TERRAINRENDER_H)
#define INCLUDED_TERRAINRENDER_H

#include "terrainapi.h"

#include "globalobjdef.h"
#include "vec3.h"
#include "matrix4.h"
#include "techstring.h"

#include <set>
#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

F_DECLARE_INTERFACE(IEditorTileSet);

class cTerrainChunk;

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

class cTerrainRenderer : public cComObject2<IMPLEMENTS(ITerrainRenderer), IMPLEMENTS(IGlobalObject)>
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
      virtual void OnTerrainTileChange(uint quadx, uint quadz, uint tile);
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
};

////////////////////////////////////////

bool cTerrainRenderer::IsBlendingEnabled() const
{
   return m_bEnableBlending;
}


/////////////////////////////////////////////////////////////////////////////

void BuildSplatAlphaMap(uint splatTile, const cRange<uint> xRange, const cRange<uint> zRange, uint * pAlphaMapId);

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSplatBuilder
//

class cSplatBuilder
{
   friend class cTerrainChunk;

   cSplatBuilder(const cSplatBuilder &);
   void operator =(const cSplatBuilder &);

public:
   cSplatBuilder(const cRange<uint> xRange, const cRange<uint> zRange, uint tile, uint alphaMapId);
   ~cSplatBuilder();

   tResult GetGlTexture(IEditorTileSet * pTileSet, uint * pTexId);
   tResult GetAlphaMap(uint * pAlphaMapId);

   void AddQuad(uint x, uint z);

   size_t GetIndexCount() const;
   const uint * GetIndexPtr() const;

private:
   typedef std::pair<uint, uint> tXZ;
   typedef std::set<tXZ> tSplatQuadSet;
   tSplatQuadSet m_quads;

   const cRange<uint> m_xRange, m_zRange;

   uint m_tile;
   mutable std::vector<uint> m_indices;
   uint m_alphaMapId;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainChunk
//

class cTerrainChunk
{
   cTerrainChunk(const cTerrainChunk &);
   void operator =(const cTerrainChunk &);

public:
   cTerrainChunk();
   ~cTerrainChunk();

   static tResult Create(const cRange<uint> xRange, const cRange<uint> zRange, bool bNoBlending, cTerrainChunk * * ppChunk);

   void BuildVertexBuffer(const cRange<uint> xRange, const cRange<uint> zRange);
   void BuildSplats(const cRange<uint> xRange, const cRange<uint> zRange, bool bNoBlending);

   void Render(IEditorTileSet *);

private:
   void RenderSplatMultiTexture(cSplatBuilder * pSplat, IEditorTileSet * pTileSet, const byte * pVertexData);

   typedef std::vector<sTerrainVertex> tVertices;
   tVertices m_vertices;

   typedef std::vector<cSplatBuilder *> tSplatBuilders;
   tSplatBuilders m_splats;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TERRAINRENDER_H
