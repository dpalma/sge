/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_TERRAIN_H)
#define INCLUDED_TERRAIN_H

#include "sceneapi.h"

#include "comtools.h"
#include "vec2.h"
#include "vec3.h"
#include "quat.h"
#include "matrix4.h"
#include "str.h"

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

F_DECLARE_INTERFACE(IEditorTileSet);
F_DECLARE_INTERFACE(IHeightMap);
F_DECLARE_INTERFACE(IMaterial);
F_DECLARE_INTERFACE(IRenderDevice);
F_DECLARE_INTERFACE(IIndexBuffer);
F_DECLARE_INTERFACE(IReader);
F_DECLARE_INTERFACE(IWriter);

class cTerrain;
class cTerrainChunk;

const uint kInvalidUintValue = ~0;

/////////////////////////////////////////////////////////////////////////////

struct sTerrainVertex
{
   tVec2 uv1;
   uint32 color;
   tVec3 pos;
};

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrain
//

const int kDefaultStepSize = 32;
const int kTilesPerChunk = 32;

struct sTerrainQuad
{
   uint tile;
   sTerrainVertex verts[4];
};

typedef std::vector<sTerrainQuad> tTerrainQuads;

class cTerrain
{
public:
   cTerrain();
   ~cTerrain();

   tResult Read(IReader * pReader);
   tResult Write(IWriter * pWriter);

   tResult Init(uint nTilesX, uint nTilesZ, IEditorTileSet * pTileSet, IHeightMap * pHeightMap);
   static tResult InitQuads(uint nTilesX, uint nTilesZ, IHeightMap * pHeightMap, tTerrainQuads * pQuads);

   void GetDimensions(uint * pxd, uint * pzd) const;
   void GetExtents(uint * px, uint * pz) const;

   void GetTileIndices(float x, float z, uint * pix, uint * piz);

   tResult Render(IRenderDevice * pRenderDevice);

   uint SetTileTerrain(uint tx, uint tz, uint terrain);
   tResult GetTileVertices(uint tx, uint tz, tVec3 vertices[4]) const;

   tResult GetSceneEntities(std::vector<ISceneEntity *> * pSceneEntities);

protected:
   class cSceneEntity : public cComObject<IMPLEMENTS(ISceneEntity)>
   {
   public:
      cSceneEntity(cTerrain * pOuter);
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
      cTerrain * m_pOuter;
      tVec3 m_translation;
      tQuat m_rotation;
      tMatrix4 m_transform;
   };
   friend class cSceneEntity;

private:
   uint m_tileSize; // dimensions of a single tile in terrain space
   uint m_nTilesX, m_nTilesZ; // # of tiles in the x,z directions
   uint m_nChunksX, m_nChunksZ; // # of chunks in the x,z directions

   tTerrainQuads m_terrainQuads;

   cStr m_tileSetName;
   cAutoIPtr<IEditorTileSet> m_pTileSet;

   cSceneEntity m_sceneEntity;

   std::vector<cTerrainChunk *> m_chunks;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSplat
//

class cSplat
{
public:
   cSplat();
   ~cSplat();

private:
   cAutoIPtr<IMaterial> m_pMaterial;
   cAutoIPtr<IIndexBuffer> m_pIndexBuffer;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSplatBuilder
//

class cSplatBuilder
{
public:
   cSplatBuilder();
   ~cSplatBuilder();

   void AddTriangle(int i0, int i1, int i2);

   tResult GenerateSplat(cSplat * * ppSplat);

private:
   std::vector<int> m_indices;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainChunk
//

class cTerrainChunk : public cComObject<IMPLEMENTS(IUnknown)>
{
   cTerrainChunk(const cTerrainChunk &);
   const cTerrainChunk & operator =(const cTerrainChunk &);

public:
   cTerrainChunk();
   ~cTerrainChunk();

   static tResult Create(uint ix, uint iz, uint cx, uint cz,
      const tTerrainQuads & quads, uint nQuadsX, uint nQuadsZ,
      IEditorTileSet * pTileSet, cTerrainChunk * * ppChunk);

private:
};

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TERRAIN_H
