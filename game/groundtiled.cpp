///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "groundtiled.h"
#include "heightmap.h"

#include "render.h"
#include "material.h"

#include "vec2.h"
#include "vec3.h"
#include "techmath.h"

#include "dbgalloc.h" // must be last header

extern IRenderDevice * AccessRenderDevice();

///////////////////////////////////////////////////////////////////////////////

struct sTerrainVertex
{
   float u, v;
   float r, g, b;
   tVec3 pos;
};

sVertexElement g_terrainVertexDecl[] =
{
   { kVDU_TexCoord, kVDT_Float2 },
   { kVDU_Color, kVDT_Float3 },
   { kVDU_Position, kVDT_Float3 }
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTiledGround
//

class cTiledGround
{
public:
   cTiledGround();
   ~cTiledGround();

   bool Init(cHeightMap * pHeightMap);

   virtual void Render();

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

static const float kRed = 0;
static const float kGreen = 0.5f;
static const float kBlue = 0;

bool cTiledGround::Init(cHeightMap * pHeightMap)
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

   cAutoIPtr<IVertexDeclaration> pVertexDecl;
   if (AccessRenderDevice()->CreateVertexDeclaration(g_terrainVertexDecl,
      _countof(g_terrainVertexDecl), &pVertexDecl) != S_OK)
   {
      return false;
   }

   if (AccessRenderDevice()->CreateVertexBuffer(kNumVerts,
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

      for (int iz = 0; iz < kNumQuadsPerSide; iz++, z1 += kStepSize, z2 += kStepSize)
      {
         float x1 = 0;
         float x2 = kStepSize;

         for (int ix = 0; ix < kNumQuadsPerSide; ix++, x1 += kStepSize, x2 += kStepSize)
         {
            pVertexData[index].u = 0;
            pVertexData[index].v = 0;
            pVertexData[index].r = kRed;
            pVertexData[index].g = kGreen;
            pVertexData[index].b = kBlue;
            pVertexData[index++].pos = tVec3(x1, pHeightMap->Height(Round(x1),Round(z1)), z1);

            pVertexData[index].u = 1;
            pVertexData[index].v = 0;
            pVertexData[index].r = kRed;
            pVertexData[index].g = kGreen;
            pVertexData[index].b = kBlue;
            pVertexData[index++].pos = tVec3(x2, pHeightMap->Height(Round(x2),Round(z1)), z1);

            pVertexData[index].u = 1;
            pVertexData[index].v = 1;
            pVertexData[index].r = kRed;
            pVertexData[index].g = kGreen;
            pVertexData[index].b = kBlue;
            pVertexData[index++].pos = tVec3(x2, pHeightMap->Height(Round(x2),Round(z2)), z2);

            pVertexData[index].u = 0;
            pVertexData[index].v = 1;
            pVertexData[index].r = kRed;
            pVertexData[index].g = kGreen;
            pVertexData[index].b = kBlue;
            pVertexData[index++].pos = tVec3(x1, pHeightMap->Height(Round(x1),Round(z2)), z2);
         }
      }

      m_pVertexBuffer->Unlock();
   }
   else
   {
      return false;
   }

   if (AccessRenderDevice()->CreateIndexBuffer(kNumIndices, kBU_Default, kIBF_16Bit, kBP_System, &m_pIndexBuffer) != S_OK)
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

void cTiledGround::Render()
{
   AccessRenderDevice()->Render(kRP_Triangles, m_pMaterial, m_nIndices, m_pIndexBuffer, 0, m_pVertexBuffer);
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainNode
//

///////////////////////////////////////

cTerrainNode::cTerrainNode(cHeightMap * pHeightMap)
 : m_pHeightMap(pHeightMap),
   m_pGround(new cTiledGround)
{
   m_pGround->Init(pHeightMap);
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

void cTerrainNode::Render()
{
   if (m_pGround != NULL)
   {
      m_pGround->Render();
   }
}

///////////////////////////////////////

cTerrainNode * TerrainNodeCreate(const char * pszHeightData, float heightScale)
{
   cHeightMap * pHeightMap = new cHeightMap(heightScale);

   if (!pHeightMap->Load(pszHeightData))
   {
      delete pHeightMap;
      return NULL;
   }

   return new cTerrainNode(pHeightMap);
}

///////////////////////////////////////////////////////////////////////////////
