///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "tiledground.h"
#include "heightmap.h"
#include "editorapi.h"

#include "render.h"
#include "material.h"
#include "textureapi.h"

#include "imagedata.h"
#include "globalobj.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

sVertexElement g_terrainVertexDecl[] =
{
   { kVDU_TexCoord, kVDT_Float2, 0 },
//   { kVDU_TexCoord, kVDT_Float2, 1 },
//   { kVDU_TexCoord, kVDT_Float2, 2 },
//   { kVDU_TexCoord, kVDT_Float2, 3 },
   { kVDU_Color, kVDT_Float3 },
   { kVDU_Position, kVDT_Float3 },
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTiledGround
//

///////////////////////////////////////

cTiledGround::cTiledGround()
 : m_xDim(0),
   m_zDim(0),
   m_nIndices(0)
{
}

///////////////////////////////////////

cTiledGround::~cTiledGround()
{
}

///////////////////////////////////////

bool cTiledGround::Init(uint xDim, uint zDim,
                        IEditorTileSet * pTileSet,
                        uint defaultTile,
                        cHeightMap * pHeightMap)
{
   Assert(pTileSet != NULL);

   cAutoIPtr<IEditorTile> pTile;
   if (pTileSet->GetTile(defaultTile, &pTile) != S_OK)
   {
      return false;
   }

   Assert(!m_pMaterial);
   if (pTileSet->GetMaterial(&m_pMaterial) != S_OK)
   {
      return false;
   }

   static const int kStepSize = 16;
   static const float kRed = 0.75f, kGreen = 0.75f, kBlue = 0.75f;

   uint nTileImages = pTile->GetHorizontalImageCount() * pTile->GetVerticalImageCount();

   Assert(IsPowerOfTwo(nTileImages));

   if (!IsPowerOfTwo(nTileImages))
   {
      DebugMsg("Number of variations in a tile must be a power of two\n");
      return false;
   }

   float tileTexWidth = 1.0f / pTile->GetHorizontalImageCount();
   float tileTexHeight = 1.0f / pTile->GetVerticalImageCount();

   m_xDim = xDim;
   m_zDim = zDim;

   uint nQuads = xDim * zDim;
   m_nIndices = nQuads * 6;

   m_vertices.resize(nQuads * 4);

   int index = 0;

   float z1 = 0;
   float z2 = kStepSize;

   for (int iz = 0; iz < zDim; iz++, z1 += kStepSize, z2 += kStepSize)
   {
      float x1 = 0;
      float x2 = kStepSize;

      for (int ix = 0; ix < xDim; ix++, x1 += kStepSize, x2 += kStepSize)
      {
         uint tile = rand() & (nTileImages - 1);
         uint tileRow = tile / pTile->GetHorizontalImageCount();
         uint tileCol = tile % pTile->GetHorizontalImageCount();

#define Height(xx,zz) ((pHeightMap != NULL) ? pHeightMap->Height(Round(xx),Round(zz)) : 0)

         m_vertices[index].uv1 = tVec2(tileCol * tileTexWidth, tileRow * tileTexHeight);
//         m_vertices[index].uv2 = tVec2(0,0);
//         m_vertices[index].uv3 = tVec2(0,0);
//         m_vertices[index].uv4 = tVec2(0,0);
         m_vertices[index].rgb = tVec3(kRed,kGreen,kBlue);
         m_vertices[index++].pos = tVec3(x1, Height(x1,z1), z1);

         m_vertices[index].uv1 = tVec2((tileCol + 1) * tileTexWidth, tileRow * tileTexHeight);
//         m_vertices[index].uv2 = tVec2(0,0);
//         m_vertices[index].uv3 = tVec2(0,0);
//         m_vertices[index].uv4 = tVec2(0,0);
         m_vertices[index].rgb = tVec3(kRed,kGreen,kBlue);
         m_vertices[index++].pos = tVec3(x2, Height(x2,z1), z1);

         m_vertices[index].uv1 = tVec2((tileCol + 1) * tileTexWidth, (tileRow + 1) * tileTexHeight);
//         m_vertices[index].uv2 = tVec2(0,0);
//         m_vertices[index].uv3 = tVec2(0,0);
//         m_vertices[index].uv4 = tVec2(0,0);
         m_vertices[index].rgb = tVec3(kRed,kGreen,kBlue);
         m_vertices[index++].pos = tVec3(x2, Height(x2,z2), z2);

         m_vertices[index].uv1 = tVec2(tileCol * tileTexWidth, (tileRow + 1) * tileTexHeight);
//         m_vertices[index].uv2 = tVec2(0,0);
//         m_vertices[index].uv3 = tVec2(0,0);
//         m_vertices[index].uv4 = tVec2(0,0);
         m_vertices[index].rgb = tVec3(kRed,kGreen,kBlue);
         m_vertices[index++].pos = tVec3(x1, Height(x1,z2), z2);

#undef Height
      }
   }

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

   if (pRenderDevice->CreateVertexBuffer(m_vertices.size(),
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

   if (pRenderDevice->CreateIndexBuffer(m_nIndices, kBU_Default, kIBF_16Bit, kBP_System, &m_pIndexBuffer) != S_OK)
   {
      return false;
   }

   uint16 * pIndexData = NULL;
   if (m_pIndexBuffer->Lock(kBL_Discard, (void * *)&pIndexData) == S_OK)
   {
      int iQuad = 0;

      for (int iz = 0; iz < m_zDim; iz++)
      {
         for (int ix = 0; ix < m_xDim; ix++, iQuad++)
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
