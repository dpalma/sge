/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_TERRAIN_H)
#define INCLUDED_TERRAIN_H

#include "terrainapi.h"

#include "sceneapi.h"

#include "globalobj.h"
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
F_DECLARE_INTERFACE(ITexture);
F_DECLARE_INTERFACE(IReader);
F_DECLARE_INTERFACE(IWriter);

class cMapSettings;

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

const int kDefaultStepSize = 32;
const int kTilesPerChunk = 32;

struct sTerrainQuad
{
   uint tile;
   sTerrainVertex verts[4];
};

typedef std::vector<sTerrainQuad> tTerrainQuads;


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainGlobal
//

class cTerrainGlobal : public cGlobalObject<IMPLEMENTS(ITerrain)>
{
public:
   cTerrainGlobal();
   ~cTerrainGlobal();

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult Set(const cMapSettings & mapSettings);
   virtual tResult Reset();
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrain
//

class cTerrain
{
public:
   cTerrain();
   ~cTerrain();

   tResult Read(IReader * pReader);
   tResult Write(IWriter * pWriter);

   tResult Init(const cMapSettings & mapSettings);
   static tResult InitQuads(uint nTilesX, uint nTilesZ, IHeightMap * pHeightMap, tTerrainQuads * pQuads);
   tResult RegenerateChunks();

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

   typedef std::vector<cTerrainChunk *> tChunks;
   tChunks m_chunks;
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
   cSplatBuilder(ITexture * pTexture);
   ~cSplatBuilder();

   tResult GetTexture(ITexture * * ppTexture);

   void AddTriangle(uint i0, uint i1, uint i2);

   tResult CreateIndexBuffer(IIndexBuffer * * ppIndexBuffer);

   size_t GetIndexCount() const;
   const uint * GetIndexPtr() const;

private:
   std::vector<uint> m_indices;
   cAutoIPtr<ITexture> m_pTexture;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainChunk
//

class cTerrainChunk : public cComObject<IMPLEMENTS(ISceneEntity)>
{
   cTerrainChunk(const cTerrainChunk &);
   const cTerrainChunk & operator =(const cTerrainChunk &);

public:
   cTerrainChunk();
   ~cTerrainChunk();

   static tResult Create(uint ix, uint iz, uint cx, uint cz,
      const tTerrainQuads & quads, uint nQuadsX, uint nQuadsZ,
      IEditorTileSet * pTileSet, cTerrainChunk * * ppChunk);

   virtual ISceneEntity * AccessParent() { return NULL; }
   virtual tResult SetParent(ISceneEntity * pEntity) { return E_FAIL; }
   virtual tResult IsChild(ISceneEntity * pEntity) const { return E_FAIL; }
   virtual tResult AddChild(ISceneEntity * pEntity) { return E_FAIL; }
   virtual tResult RemoveChild(ISceneEntity * pEntity) { return E_FAIL; }

   virtual const tVec3 & GetLocalTranslation() const { static tVec3 vz(0,0,0); return vz; }
   virtual void SetLocalTranslation(const tVec3 & translation) {}
   virtual const tQuat & GetLocalRotation() const { static tQuat qz(0,0,0,1); return qz; }
   virtual void SetLocalRotation(const tQuat & rotation) {}
   virtual const tMatrix4 & GetLocalTransform() const { return tMatrix4::GetIdentity(); }

   virtual const tVec3 & GetWorldTranslation() const { return GetLocalTranslation(); }
   virtual const tQuat & GetWorldRotation() const { return GetLocalRotation(); }
   virtual const tMatrix4 & GetWorldTransform() const { return GetLocalTransform(); }

   virtual void Render(IRenderDevice * pRenderDevice);
   virtual float GetBoundingRadius() const { return 9999999; }

   virtual tResult Intersects(const cRay & ray) { return E_NOTIMPL; }

private:
   typedef std::vector<sTerrainVertex> tVertices;
   tVertices m_vertices;

   typedef std::vector<cSplatBuilder *> tSplatBuilders;
   tSplatBuilders m_splats;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TERRAIN_H
