/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_TERRAINRENDER_H)
#define INCLUDED_TERRAINRENDER_H

#include "terrainapi.h"

#include "sceneapi.h"

#include "globalobj.h"
#include "vec3.h"
#include "quat.h"
#include "matrix4.h"
#include "techstring.h"

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

F_DECLARE_INTERFACE(IEditorTileSet);

F_DECLARE_INTERFACE(IRenderDevice);

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

private:
   bool IsBlendingEnabled() const;

   void RegenerateChunks();
   ITerrainModel * AccessModel() { return m_pModel; }

   class cSceneEntity : public cComObject<IMPLEMENTS(ISceneEntity)>
   {
   public:
      cSceneEntity(cTerrainRenderer * pOuter);
      virtual ~cSceneEntity();

      virtual void DeleteThis() {}

      virtual ISceneEntity * AccessParent() { return NULL; }
      virtual tResult SetParent(ISceneEntity * pEntity) { return E_FAIL; }
      virtual tResult IsChild(ISceneEntity * pEntity) const { return E_FAIL; }
      virtual tResult AddChild(ISceneEntity * pEntity) { return E_FAIL; }
      virtual tResult RemoveChild(ISceneEntity * pEntity) { return E_FAIL; }

      virtual const tVec3 & GetLocalTranslation() const { return m_translation; }
      virtual void SetLocalTranslation(const tVec3 & translation) { m_translation = translation; }
      virtual const tQuat & GetLocalRotation() const { return m_rotation; }
      virtual void SetLocalRotation(const tQuat & rotation) { m_rotation = rotation; }
      virtual const tMatrix4 & GetLocalTransform() const { return m_transform; }

      virtual const tVec3 & GetWorldTranslation() const { return GetLocalTranslation(); }
      virtual const tQuat & GetWorldRotation() const { return GetLocalRotation(); }
      virtual const tMatrix4 & GetWorldTransform() const { return GetLocalTransform(); }

      virtual void Render(IRenderDevice * pRenderDevice);
      virtual float GetBoundingRadius() const { return 9999999; }

      virtual tResult Intersects(const cRay & ray) { return E_NOTIMPL; }

   private:
      cTerrainRenderer * m_pOuter;
      tVec3 m_translation;
      tQuat m_rotation;
      tMatrix4 m_transform;
   };
   friend class cSceneEntity;
   cSceneEntity m_sceneEntity;

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
   cSplatBuilder(const cSplatBuilder &);
   void operator =(const cSplatBuilder &);

public:
   cSplatBuilder(IEditorTileSet * pTileSet, uint tile);
   ~cSplatBuilder();

   tResult GetGlTexture(uint * pTexId);
   tResult GetAlphaMap(uint * pAlphaMapId);

   void AddTriangle(uint i0, uint i1, uint i2);

   size_t GetIndexCount() const;
   const uint * GetIndexPtr() const;

   void BuildAlphaMap(const tTerrainQuads & quads, uint nQuadsX, uint nQuadsZ, 
                      uint iChunkX, uint iChunkZ);

private:
   cAutoIPtr<IEditorTileSet> m_pTileSet;
   uint m_tile;
   std::vector<uint> m_indices;
   cStr m_texture;
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
