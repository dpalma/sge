///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "groundtiled.h"
#include "heightmap.h"

#include "ray.h"

#include "render.h"
#include "material.h"
#include "textureapi.h"
#include "imagedata.h"

#include "vec2.h"
#include "vec3.h"
#include "techmath.h"
#include "resmgr.h"
#include "globalobj.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

struct sTerrainVertex
{
   tVec2 uv;
   tVec3 rgb;
   tVec3 pos;
};

VERTEXDECL_BEGIN(g_terrainVertexDecl)
   VERTEXDECL_ELEMENT(kVDU_TexCoord, kVDT_Float2)
   VERTEXDECL_ELEMENT(kVDU_Color, kVDT_Float3)
   VERTEXDECL_ELEMENT(kVDU_Position, kVDT_Float3)
VERTEXDECL_END()

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTiledGround
//

class cTiledGround
{
public:
   cTiledGround();
   ~cTiledGround();

   bool Init(IRenderDevice * pRenderDevice, cHeightMap * pHeightMap, const char * pszTexture);

   void Render(IRenderDevice * pRenderDevice);

private:
   cAutoIPtr<IVertexBuffer> m_pVertexBuffer;
   cAutoIPtr<IIndexBuffer> m_pIndexBuffer;
   cAutoIPtr<IMaterial> m_pMaterial;
   int m_nVerts, m_nIndices;
};

///////////////////////////////////////

cTiledGround::cTiledGround()
 : m_nVerts(0),
   m_nIndices(0)
{
}

///////////////////////////////////////

cTiledGround::~cTiledGround()
{
}

///////////////////////////////////////

bool cTiledGround::Init(IRenderDevice * pRenderDevice, cHeightMap * pHeightMap, const char * pszTexture)
{
   const int kStepSize = 16;
   const int kGroundSize = 1024;
   const int kNumQuadsPerSide = kGroundSize / kStepSize;
   const int kNumQuads = kNumQuadsPerSide * kNumQuadsPerSide;
   const int kNumVerts = kNumQuads * 4;
   const int kNumIndices = kNumQuads * 6;

   float kRed = 1, kGreen = 1, kBlue = 1;

   m_nVerts = kNumVerts;
   m_nIndices = kNumIndices;

   m_pMaterial = MaterialCreate();
   if (!m_pMaterial)
   {
      return false;
   }

   if (pszTexture != NULL)
   {
      UseGlobal(TextureManager);
      cAutoIPtr<ITexture> pTexture;
      if (pTextureManager->GetTexture(pszTexture, &pTexture) == S_OK)
      {
         m_pMaterial->SetTexture(0, pTexture);
      }
   }
   else
   {
      kGreen = 0.5f;
      kRed = kBlue = 0;
   }

   cAutoIPtr<IVertexDeclaration> pVertexDecl;
   if (pRenderDevice->CreateVertexDeclaration(g_terrainVertexDecl,
      _countof(g_terrainVertexDecl), &pVertexDecl) != S_OK)
   {
      return false;
   }

   if (pRenderDevice->CreateVertexBuffer(kNumVerts,
      kBU_Default, pVertexDecl, kBP_Auto, &m_pVertexBuffer) != S_OK)
   {
      return false;
   }

   sTerrainVertex * pVertexData = NULL;
   if (m_pVertexBuffer->Lock(kBL_Discard, (void * *)&pVertexData) == S_OK)
   {
      int index = 0;

      float z1 = 0;
      float z2 = kStepSize;

      static const float kTileTexWidth = 0.125f;
      static const float kTileTexHeight = 0.25f;

      for (int iz = 0; iz < kNumQuadsPerSide; iz++, z1 += kStepSize, z2 += kStepSize)
      {
         float x1 = 0;
         float x2 = kStepSize;

         for (int ix = 0; ix < kNumQuadsPerSide; ix++, x1 += kStepSize, x2 += kStepSize)
         {
            uint tile = rand() & 15;
            uint tileRow = tile % 4;
            uint tileCol = tile / 4;

            pVertexData[index].uv = tVec2(tileCol * kTileTexWidth, tileRow * kTileTexHeight);
            pVertexData[index].rgb = tVec3(kRed,kGreen,kBlue);
            pVertexData[index++].pos = tVec3(x1, pHeightMap->Height(Round(x1),Round(z1)), z1);

            pVertexData[index].uv = tVec2((tileCol + 1) * kTileTexWidth, tileRow * kTileTexHeight);
            pVertexData[index].rgb = tVec3(kRed,kGreen,kBlue);
            pVertexData[index++].pos = tVec3(x2, pHeightMap->Height(Round(x2),Round(z1)), z1);

            pVertexData[index].uv = tVec2((tileCol + 1) * kTileTexWidth, (tileRow + 1) * kTileTexHeight);
            pVertexData[index].rgb = tVec3(kRed,kGreen,kBlue);
            pVertexData[index++].pos = tVec3(x2, pHeightMap->Height(Round(x2),Round(z2)), z2);

            pVertexData[index].uv = tVec2(tileCol * kTileTexWidth, (tileRow + 1) * kTileTexHeight);
            pVertexData[index].rgb = tVec3(kRed,kGreen,kBlue);
            pVertexData[index++].pos = tVec3(x1, pHeightMap->Height(Round(x1),Round(z2)), z2);
         }
      }

      m_pVertexBuffer->Unlock();
   }
   else
   {
      return false;
   }

   if (pRenderDevice->CreateIndexBuffer(kNumIndices, kBU_Default, kIBF_16Bit, kBP_System, &m_pIndexBuffer) != S_OK)
   {
      return false;
   }

   uint16 * pIndexData = NULL;
   if (m_pIndexBuffer->Lock(kBL_Discard, (void * *)&pIndexData) == S_OK)
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

void cTiledGround::Render(IRenderDevice * pRenderDevice)
{
   pRenderDevice->Render(kRP_Triangles, m_pMaterial, m_nIndices, m_pIndexBuffer, 0, m_pVertexBuffer);
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainNode
//

///////////////////////////////////////

cTerrainNode::cTerrainNode(IRenderDevice * pRenderDevice, cHeightMap * pHeightMap, const char * pszTexture)
 : m_pSceneEntity(SceneEntityCreate()),
   m_pHeightMap(pHeightMap),
   m_pGround(new cTiledGround)
{
   m_pGround->Init(pRenderDevice, pHeightMap, pszTexture);
}

///////////////////////////////////////

cTerrainNode::~cTerrainNode()
{
   delete m_pGround, m_pGround = NULL;
   delete m_pHeightMap, m_pHeightMap = NULL;
}

///////////////////////////////////////

float cTerrainNode::GetElevation(float nx, float nz) const
{
   Assert(m_pHeightMap != NULL);
   uint size = m_pHeightMap->GetSize() - 1;
   return m_pHeightMap->Height(Round(nx * size), Round(nz * size));
}

///////////////////////////////////////

tVec2 cTerrainNode::GetDimensions() const
{
   Assert(m_pHeightMap != NULL);
   float size = (float)m_pHeightMap->GetSize();
   return tVec2(size, size);
}

///////////////////////////////////////

void cTerrainNode::Render(IRenderDevice * pRenderDevice)
{
   if (m_pGround != NULL)
   {
      m_pGround->Render(pRenderDevice);
   }
}

///////////////////////////////////////

tResult cTerrainNode::Intersects(const cRay & ray)
{
   tVec3 intersect;
   if (ray.IntersectsPlane(tVec3(0,1,0), 0, &intersect))
   {
      DebugMsg3("Hit the ground at approximately (%.1f,%.1f,%.1f)\n",
         intersect.x, intersect.y, intersect.z);
   }

   return E_NOTIMPL;
}

///////////////////////////////////////

cTerrainNode * TerrainNodeCreate(IRenderDevice * pRenderDevice, const char * pszHeightData, float heightScale, const char * pszTexture)
{
   cHeightMap * pHeightMap = new cHeightMap(heightScale);

   if (!pHeightMap->Load(pszHeightData))
   {
      delete pHeightMap;
      return NULL;
   }

   return new cTerrainNode(pRenderDevice, pHeightMap, pszTexture);
}

///////////////////////////////////////////////////////////////////////////////
