///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "groundtiled.h"
#include "heightmap.h"
#include "scenenode.h"

#include "render.h"
#include "material.h"

#include "vec2.h"
#include "vec3.h"
#include "comtools.h"
#include "TechMath.h"

#include "ggl.h"

#include <climits>

#include "dbgalloc.h" // must be last header

extern IRenderDevice * AccessRenderDevice();

static const float kGroundScaleY = 0.25f;

cTerrainData g_terrainData(1024, 1024, 0.25f);

///////////////////////////////////////////////////////////////////////////////

struct sTerrainVertex
{
   float u, v;
   tVec3 pos;
};

sVertexElement g_terrainVertexDecl[] =
{
   { kVDU_TexCoord, kVDT_Float2 },
   { kVDU_Position, kVDT_Float3 }
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainData
//

cTerrainData::cTerrainData(ulong sizeX, ulong sizeZ, float heightScale)
 : m_sizeX(sizeX),
   m_sizeZ(sizeZ),
   m_heightScale(heightScale),
   m_pHeightMap(NULL)
{
}

cTerrainData::~cTerrainData()
{
   delete m_pHeightMap, m_pHeightMap = NULL;
}

bool cTerrainData::LoadHeightMap(const tChar * pszHeightMapFile)
{
   Assert(m_pHeightMap == NULL);

   cHeightMap * pHeightMap = new cHeightMap(m_heightScale);

   if (!pHeightMap->Load(pszHeightMapFile))
   {
      delete pHeightMap;
      return false;
   }

   m_pHeightMap = pHeightMap;

   return true;
}

float cTerrainData::GetElevation(float nx, float nz) const
{
   Assert(m_pHeightMap != NULL);
   return m_pHeightMap->Height(Round(nx * m_sizeX), Round(nz * m_sizeZ));
}

cVec2<float> cTerrainData::GetDimensions() const
{
   return cVec2<float>(m_sizeX, m_sizeZ);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTiledGround
//

class cTiledGround
{
public:
   cTiledGround();
   ~cTiledGround();

   bool Init();

   virtual void Render();

private:
   cAutoIPtr<IVertexDeclaration> m_pVertexDecl;
   cAutoIPtr<IVertexBuffer> m_pVertexBuffer;
   cAutoIPtr<IIndexBuffer> m_pIndexBuffer;
   cAutoIPtr<IMaterial> m_pMaterial;
   bool m_bInitialized;
   int m_nVerts, m_nIndices;
};

///////////////////////////////////////

cTiledGround::cTiledGround()
 : m_bInitialized(false),
   m_nVerts(0),
   m_nIndices(0)
{
}

///////////////////////////////////////

cTiledGround::~cTiledGround()
{
}

///////////////////////////////////////

bool cTiledGround::Init()
{
   const int kStepSize = 16;
   const int kGroundSize = 1024;
   const int kNumQuadsPerSide = kGroundSize / kStepSize;
   const int kNumQuads = kNumQuadsPerSide * kNumQuadsPerSide;
   const int kNumVerts = kNumQuads * 4;
   const int kNumIndices = kNumQuads * 6;

   m_nVerts = kNumVerts;
   m_nIndices = kNumIndices;

   m_pMaterial = MaterialCreate();
   if (!m_pMaterial)
   {
      return false;
   }

   if (AccessRenderDevice()->CreateVertexDeclaration(g_terrainVertexDecl,
      _countof(g_terrainVertexDecl), &m_pVertexDecl) != S_OK)
   {
      return false;
   }

   if (AccessRenderDevice()->CreateVertexBuffer(kNumVerts,
      m_pVertexDecl, kMP_Auto, &m_pVertexBuffer) != S_OK)
   {
      return false;
   }

   tVec2 dims = g_terrainData.GetDimensions();

   sTerrainVertex * pVertexData = NULL;
   if (m_pVertexBuffer->Lock((void * *)&pVertexData) == S_OK)
   {
      int index = 0;

      for (int iz = 0; iz < kNumQuadsPerSide; iz++)
      {
         float z1 = iz * kStepSize;
         float z2 = (iz + 1) * kStepSize;

         for (int ix = 0; ix < kNumQuadsPerSide; ix++)
         {
            float x1 = ix * kStepSize;
            float x2 = (ix + 1) * kStepSize;

            pVertexData[index].u = 0;
            pVertexData[index].v = 0;
            pVertexData[index++].pos = tVec3(x1, g_terrainData.GetElevation(x1 / dims.x, z1 / dims.y), z1);

            pVertexData[index].u = 1;
            pVertexData[index].v = 0;
            pVertexData[index++].pos = tVec3(x2, g_terrainData.GetElevation(x2 / dims.x, z1 / dims.y), z1);

            pVertexData[index].u = 1;
            pVertexData[index].v = 1;
            pVertexData[index++].pos = tVec3(x2, g_terrainData.GetElevation(x2 / dims.x, z2 / dims.y), z2);

            pVertexData[index].u = 0;
            pVertexData[index].v = 1;
            pVertexData[index++].pos = tVec3(x1, g_terrainData.GetElevation(x1 / dims.x, z2 / dims.y), z2);
         }
      }

      m_pVertexBuffer->Unlock();
   }
   else
   {
      return false;
   }

   if (AccessRenderDevice()->CreateIndexBuffer(kNumIndices, kMP_Auto, &m_pIndexBuffer) != S_OK)
   {
      return false;
   }

   uint * pIndexData = NULL;
   if (m_pIndexBuffer->Lock((void * *)&pIndexData) == S_OK)
   {
      int iQuad = 0;

      for (int iz = 0; iz < kNumQuadsPerSide; iz++)
      {
         for (int ix = 0; ix < kNumQuadsPerSide; ix++, iQuad++)
         {
            pIndexData[(iQuad * 6) + 0] = (iQuad * 4) + 0;
            pIndexData[(iQuad * 6) + 1] = (iQuad * 4) + 3;
            pIndexData[(iQuad * 6) + 2] = (iQuad * 4) + 2;

            pIndexData[(iQuad * 6) + 3] = (iQuad * 4) + 2;
            pIndexData[(iQuad * 6) + 4] = (iQuad * 4) + 1;
            pIndexData[(iQuad * 6) + 5] = (iQuad * 4) + 0;
         }
      }

      m_pIndexBuffer->Unlock();
   }
   else
   {
      return false;
   }

   return true;
}

///////////////////////////////////////

void cTiledGround::Render()
{
   if (!m_bInitialized)
      m_bInitialized = Init();
   AccessRenderDevice()->Render(kRP_Triangles, m_pMaterial, m_nIndices, m_pIndexBuffer, 0, m_nVerts, m_pVertexBuffer);
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGroundSceneNode
//

class cGroundSceneNode : public cSceneNode
{
public:
   cGroundSceneNode();
   ~cGroundSceneNode();

   virtual void Render();

   cTiledGround * m_pGround;
};

cGroundSceneNode::cGroundSceneNode()
 : m_pGround(new cTiledGround)
{
}

cGroundSceneNode::~cGroundSceneNode()
{
   delete m_pGround, m_pGround = NULL;
}

void cGroundSceneNode::Render()
{
   if (m_pGround != NULL)
   {
      glPushAttrib(GL_CURRENT_BIT);
      glColor3f(0,0.5,0);
      m_pGround->Render();
      glPopAttrib();
   }
}

cSceneNode * TerrainRootNodeCreate()
{
   return new cGroundSceneNode;
}

///////////////////////////////////////////////////////////////////////////////
