///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "tiledground.h"
#include "heightmap.h"

#include "render.h"
#include "material.h"
#include "textureapi.h"

#include "imagedata.h"
#include "globalobj.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

VERTEXDECL_BEGIN(g_terrainVertexDecl)
   VERTEXDECL_ELEMENT(kVDU_TexCoord, kVDT_Float2)
   VERTEXDECL_ELEMENT(kVDU_Color, kVDT_Float3)
   VERTEXDECL_ELEMENT(kVDU_Position, kVDT_Float3)
VERTEXDECL_END()

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTiledGround
//

///////////////////////////////////////

cTiledGround::cTiledGround()
 : m_nIndices(0),
   m_bInitialized(false)
{
}

///////////////////////////////////////

cTiledGround::~cTiledGround()
{
}

///////////////////////////////////////

bool cTiledGround::SetTexture(const char * pszTexture)
{
   bool result = false;

   cAutoIPtr<IMaterial> pNewMaterial = MaterialCreate();
   if (!!pNewMaterial)
   {
      if (pszTexture != NULL)
      {
         UseGlobal(TextureManager);
         cAutoIPtr<ITexture> pTexture;
         if (pTextureManager->GetTexture(pszTexture, &pTexture) == S_OK)
         {
            if (pNewMaterial->SetTexture(0, pTexture) == S_OK)
            {
               SafeRelease(m_pMaterial);
               m_pMaterial = CTAddRef(pNewMaterial);
               result = true;
            }
         }
      }
   }

   return result;
}

///////////////////////////////////////

static const int kStepSize = 16;
static const int kGroundSize = 1024;
static const int kNumQuadsPerSide = kGroundSize / kStepSize;
static const int kNumQuads = kNumQuadsPerSide * kNumQuadsPerSide;
static const int kNumVerts = kNumQuads * 4;
static const int kNumIndices = kNumQuads * 6;
static const float kRed = 1, kGreen = 1, kBlue = 1;

static const float kTileTexWidth = 0.125f;
static const float kTileTexHeight = 0.25f;

bool cTiledGround::Init(cHeightMap * pHeightMap)
{
   m_vertices.resize(kNumVerts);

   int index = 0;

   float z1 = 0;
   float z2 = kStepSize;

   for (int iz = 0; iz < kNumQuadsPerSide; iz++, z1 += kStepSize, z2 += kStepSize)
   {
      float x1 = 0;
      float x2 = kStepSize;

      for (int ix = 0; ix < kNumQuadsPerSide; ix++, x1 += kStepSize, x2 += kStepSize)
      {
         uint tile = rand() & 15;
         uint tileRow = tile % 4;
         uint tileCol = tile / 4;

         m_vertices[index].uv = tVec2(tileCol * kTileTexWidth, tileRow * kTileTexHeight);
         m_vertices[index].rgb = tVec3(kRed,kGreen,kBlue);
         m_vertices[index++].pos = tVec3(x1, pHeightMap->Height(Round(x1),Round(z1)), z1);

         m_vertices[index].uv = tVec2((tileCol + 1) * kTileTexWidth, tileRow * kTileTexHeight);
         m_vertices[index].rgb = tVec3(kRed,kGreen,kBlue);
         m_vertices[index++].pos = tVec3(x2, pHeightMap->Height(Round(x2),Round(z1)), z1);

         m_vertices[index].uv = tVec2((tileCol + 1) * kTileTexWidth, (tileRow + 1) * kTileTexHeight);
         m_vertices[index].rgb = tVec3(kRed,kGreen,kBlue);
         m_vertices[index++].pos = tVec3(x2, pHeightMap->Height(Round(x2),Round(z2)), z2);

         m_vertices[index].uv = tVec2(tileCol * kTileTexWidth, (tileRow + 1) * kTileTexHeight);
         m_vertices[index].rgb = tVec3(kRed,kGreen,kBlue);
         m_vertices[index++].pos = tVec3(x1, pHeightMap->Height(Round(x1),Round(z2)), z2);
      }
   }

   m_bInitialized = true;

   return true;
}

///////////////////////////////////////

bool cTiledGround::CreateBuffers(IRenderDevice * pRenderDevice)
{
   Assert(!m_pVertexBuffer);
   Assert(!m_pIndexBuffer);

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

   void * pVertexData = NULL;
   if (m_pVertexBuffer->Lock(kBL_Discard, (void * *)&pVertexData) == S_OK)
   {
      memcpy(pVertexData, &m_vertices[0], m_vertices.size() * sizeof(sTerrainVertex));
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

   m_nIndices = kNumIndices;

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

      return true;
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////
