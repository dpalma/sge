/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_TERRAINRENDER_H)
#define INCLUDED_TERRAINRENDER_H

#include "terrainapi.h"

#include "globalobjdef.h"
#include "vec3.h"
#include "matrix4.h"
#include "techstring.h"

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

F_DECLARE_INTERFACE(IEditorTileSet);

class cTerrainChunk;

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainRenderer
//

class cTerrainRenderer : public cComObject2<IMPLEMENTS(ITerrainRenderer), IMPLEMENTS(IGlobalObject)>
{
public:
   cTerrainRenderer();
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
   bool IsBlendingEnabled() const;

   void RegenerateChunks();

   class cTerrainModelListener : public cComObject<IMPLEMENTS(ITerrainModelListener)>
   {
   public:
      cTerrainModelListener(cTerrainRenderer * pOuter);
      virtual void DeleteThis() { /* Do not delete; Non-pointer member of cTerrainRenderer */ }
      virtual void OnTerrainInitialize();
      virtual void OnTerrainClear();
      virtual void OnTerrainChange();
   private:
      cTerrainRenderer * m_pOuter;
   };
   friend class cTerrainModelListener;
   cTerrainModelListener m_terrainModelListener;

   uint m_nTilesPerChunk;
   typedef std::vector<cTerrainChunk *> tChunks;
   tChunks m_chunks;

   bool m_bEnableBlending;
   bool m_bTerrainChanged; // terrain changed while blending disabled?
};

////////////////////////////////////////

bool cTerrainRenderer::IsBlendingEnabled() const
{
   return m_bEnableBlending;
}


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
   virtual ~cTerrainChunk() = 0;

   virtual void Render() = 0;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainChunkSimple
//

class cTerrainChunkSimple : cTerrainChunk
{
   cTerrainChunkSimple(const cTerrainChunkSimple &);
   void operator =(const cTerrainChunkSimple &);

public:
   static tResult Create(uint iChunkX, uint iChunkZ, cTerrainChunk * * ppChunk);

   cTerrainChunkSimple();
   ~cTerrainChunkSimple();

   void Render();

private:
   struct sSimpleVertex
   {
      tVec2 uv;
      tVec3 pos;
   };
   typedef std::vector<sSimpleVertex> tVertices;
   tVertices m_vertices;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSplatBuilder
//

class cSplatBuilder
{
   friend class cTerrainChunkBlended;

   cSplatBuilder(const cSplatBuilder &);
   void operator =(const cSplatBuilder &);

public:
   cSplatBuilder(uint tile, const char * pszTexture);
   ~cSplatBuilder();

   tResult GetGlTexture(uint * pTexId);
   tResult GetAlphaMap(uint * pAlphaMapId);

   void AddTriangle(uint i0, uint i1, uint i2);

   size_t GetIndexCount() const;
   const uint * GetIndexPtr() const;

   void BuildAlphaMap(uint nQuadsX, uint nQuadsZ, uint iChunkX, uint iChunkZ);

private:
   uint m_tile;
   cStr m_tileTexture;
   std::vector<uint> m_indices;
   uint m_alphaMapId;
};



/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainChunkBlended
//

class cTerrainChunkBlended : cTerrainChunk
{
   cTerrainChunkBlended(const cTerrainChunkBlended &);
   void operator =(const cTerrainChunkBlended &);

public:
   cTerrainChunkBlended();
   ~cTerrainChunkBlended();

   static tResult Create(uint nQuadsX, uint nQuadsZ, uint iChunkX, uint iChunkZ,
      IEditorTileSet * pTileSet, cTerrainChunk * * ppChunk);

   void Render();

private:
   typedef std::vector<sTerrainVertex> tVertices;
   tVertices m_vertices;

   typedef std::vector<cSplatBuilder *> tSplatBuilders;
   tSplatBuilders m_splats;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TERRAINRENDER_H
