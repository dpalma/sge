/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "terrain.h"
#include "editorapi.h"
#include "heightmap.h"
#include "material.h"

#include "render.h"

#include "dbgalloc.h" // must be last header

static const int kTilesPerBlock = 6;

/////////////////////////////////////////////////////////////////////////////

sVertexElement g_mapVertexDecl[] =
{
   { kVDU_TexCoord, kVDT_Float2, 0 },
   { kVDU_Color, kVDT_Float3 },
   { kVDU_Position, kVDT_Float3 },
};

uint g_nMapVertexMembers = _countof(g_mapVertexDecl);

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrain
//

////////////////////////////////////////

cTerrain::cTerrain()
 : m_xDim(0),
   m_zDim(0),
   m_tileSize(0),
   m_xBlocks(0),
   m_zBlocks(0)
{
}

////////////////////////////////////////

cTerrain::~cTerrain()
{
}

////////////////////////////////////////

bool cTerrain::Create(uint xDim, uint zDim, int stepSize,
                      IEditorTileSet * pTileSet, uint defaultTile,
                      cHeightMap * pHeightMap)
{
   InitializeVertices(xDim, zDim, stepSize, pHeightMap);

   Assert(pTileSet != NULL);

   cAutoIPtr<IEditorTile> pTile;
   if (pTileSet->GetTile(defaultTile, &pTile) != S_OK)
   {
      return false;
   }

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

   int index = 0;

   float z1 = 0;
   float z2 = stepSize;

   for (int iz = 0; iz < zDim; iz++, z1 += stepSize, z2 += stepSize)
   {
      float x1 = 0;
      float x2 = stepSize;

      for (int ix = 0; ix < xDim; ix++, x1 += stepSize, x2 += stepSize)
      {
         uint tile = rand() & (nTileImages - 1);
         uint tileRow = tile / pTile->GetHorizontalImageCount();
         uint tileCol = tile % pTile->GetHorizontalImageCount();

         m_vertices[index].uv1 = tVec2(tileCol * tileTexWidth, tileRow * tileTexHeight);
         m_vertices[index++].rgb = tVec3(kRed,kGreen,kBlue);

         m_vertices[index].uv1 = tVec2((tileCol + 1) * tileTexWidth, tileRow * tileTexHeight);
         m_vertices[index++].rgb = tVec3(kRed,kGreen,kBlue);

         m_vertices[index].uv1 = tVec2((tileCol + 1) * tileTexWidth, (tileRow + 1) * tileTexHeight);
         m_vertices[index++].rgb = tVec3(kRed,kGreen,kBlue);

         m_vertices[index].uv1 = tVec2(tileCol * tileTexWidth, (tileRow + 1) * tileTexHeight);
         m_vertices[index++].rgb = tVec3(kRed,kGreen,kBlue);
      }
   }

   m_tileSize = stepSize;

   if (xDim > kTilesPerBlock)
   {
      m_xBlocks = xDim / kTilesPerBlock;
   }

   if (zDim > kTilesPerBlock)
   {
      m_zBlocks = zDim / kTilesPerBlock;
   }

   return true;
}

////////////////////////////////////////

void cTerrain::GetDimensions(uint * pxd, uint * pzd) const
{
   if (pxd != NULL)
   {
      *pxd = m_xDim;
   }

   if (pzd != NULL)
   {
      *pzd = m_zDim;
   }
}

////////////////////////////////////////

void cTerrain::GetExtents(uint * px, uint * pz) const
{
   if (px != NULL)
   {
      *px = m_tileSize * m_xDim;
   }

   if (pz != NULL)
   {
      *pz = m_tileSize * m_zDim;
   }
}

////////////////////////////////////////

const sMapVertex * cTerrain::GetVertexPointer() const
{
   return &m_vertices[0];
}

////////////////////////////////////////

size_t cTerrain::GetVertexCount() const
{
   return m_vertices.size();
}

////////////////////////////////////////

void cTerrain::Render(IRenderDevice * pRenderDevice)
{
}

////////////////////////////////////////

void cTerrain::InitializeVertices(uint xDim, uint zDim, int stepSize, cHeightMap * pHeightMap)
{
   uint nQuads = xDim * zDim;

   m_vertices.resize(nQuads * 4);

   int index = 0;

   float z1 = 0;
   float z2 = stepSize;

   for (int iz = 0; iz < zDim; iz++, z1 += stepSize, z2 += stepSize)
   {
      float x1 = 0;
      float x2 = stepSize;

      for (int ix = 0; ix < xDim; ix++, x1 += stepSize, x2 += stepSize)
      {
#define Height(xx,zz) ((pHeightMap != NULL) ? pHeightMap->Height(Round(xx),Round(zz)) : 0)

         m_vertices[index++].pos = tVec3(x1, Height(x1,z1), z1);
         m_vertices[index++].pos = tVec3(x2, Height(x2,z1), z1);
         m_vertices[index++].pos = tVec3(x2, Height(x2,z2), z2);
         m_vertices[index++].pos = tVec3(x1, Height(x1,z2), z2);

#undef Height
      }
   }

   m_xDim = xDim;
   m_zDim = zDim;
}

////////////////////////////////////////

bool cTerrain::CreateTerrainChunks()
{
   return false;
}

