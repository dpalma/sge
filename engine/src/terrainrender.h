/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_TERRAINRENDER_H)
#define INCLUDED_TERRAINRENDER_H

#include "terrainapi.h"

#include "globalobj.h"
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

class cTerrainRenderer : public cGlobalObject<IMPLEMENTS(ITerrainRenderer)>
{
public:
   cTerrainRenderer();
   ~cTerrainRenderer();

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult SetModel(ITerrainModel * pTerrainModel);
   virtual tResult GetModel(ITerrainModel * * ppTerrainModel);

   virtual tResult EnableBlending(bool bEnable);

   virtual void Render();
   virtual void Render(IDirect3DDevice9 * pD3dDevice);

private:
   bool IsBlendingEnabled() const;

   void RegenerateChunks();
   ITerrainModel * AccessModel() { return m_pModel; }

   class cTerrainModelListener : public cComObject<IMPLEMENTS(ITerrainModelListener)>
   {
   public:
      cTerrainModelListener(cTerrainRenderer * pOuter) : m_pOuter(pOuter) {}
      virtual void DeleteThis() {}
      virtual void OnTerrainChange()
      {
         if (m_pOuter != NULL)
         {
            m_pOuter->m_bTerrainChanged = true;
            m_pOuter->RegenerateChunks();
         }
      }
   private:
      cTerrainRenderer * m_pOuter;
   };
   friend class cTerrainModelListener;
   cTerrainModelListener m_tml;

   cAutoIPtr<ITerrainModel> m_pModel;

   uint m_nChunksX, m_nChunksZ; // # of chunks in the x,z directions
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
// CLASS: cSplatBuilder
//

class cSplatBuilder
{
   friend class cTerrainChunk;

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

   void BuildAlphaMap(const tTerrainQuads & quads, uint nQuadsX,
      uint nQuadsZ, uint iChunkX, uint iChunkZ);

private:
   uint m_tile;
   cStr m_tileTexture;
   std::vector<uint> m_indices;
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

   static tResult Create(const tTerrainQuads & quads, uint nQuadsX, uint nQuadsZ,
      uint iChunkX, uint iChunkZ, IEditorTileSet * pTileSet, cTerrainChunk * * ppChunk);

   void Render();

private:
   typedef std::vector<sTerrainVertex> tVertices;
   tVertices m_vertices;

   typedef std::vector<cSplatBuilder *> tSplatBuilders;
   tSplatBuilders m_splats;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TERRAINRENDER_H
