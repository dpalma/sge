/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "terrain.h"
#include "editorapi.h"
#include "heightmap.h"

#include "material.h"
#include "render.h"

#include "readwriteapi.h"

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////

static const union
{
   byte b[4];
   uint v;
}
kTerrainFileIdGenerator = { { 's', 'g', 'e', 'm' } };

static const kTerrainFileId = kTerrainFileIdGenerator.v;

static const uint kTerrainFileVersion = MAKELONG(1,0);

/////////////////////////////////////////////////////////////////////////////

sVertexElement g_terrainVertexDecl[] =
{
   { kVDU_TexCoord, kVDT_Float2, 0 },
   { kVDU_Color, kVDT_Float3 },
   { kVDU_Position, kVDT_Float3 },
};

uint g_nTerrainVertexMembers = _countof(g_terrainVertexDecl);

////////////////////////////////////////

tResult TerrainVertexDeclarationCreate(IRenderDevice * pRenderDevice,
                                       IVertexDeclaration * * ppVertexDecl)
{
   if (pRenderDevice == NULL || ppVertexDecl == NULL)
   {
      return E_POINTER;
   }

   return pRenderDevice->CreateVertexDeclaration(g_terrainVertexDecl,
                                                 g_nTerrainVertexMembers,
                                                 ppVertexDecl);
}


/////////////////////////////////////////////////////////////////////////////

typedef std::vector<sTerrainVertex> tTerrainVertexVector;

template <>
class cReadWriteOps<tTerrainVertexVector>
{
public:
   static tResult Read(IReader * pReader, tTerrainVertexVector * pVertices);
   static tResult Write(IWriter * pWriter, const tTerrainVertexVector & vertices);
};

////////////////////////////////////////

tResult cReadWriteOps<tTerrainVertexVector>::Read(IReader * pReader,
                                                  tTerrainVertexVector * pVertices)
{
   Assert(pReader != NULL);
   Assert(pVertices != NULL);

   uint count = 0;
   if (pReader->Read(&count) != S_OK)
   {
      return E_FAIL;
   }

   pVertices->resize(count);

   for (uint i = 0; i < count; i++)
   {
      sTerrainVertex vertex;
      if (pReader->Read(&vertex, sizeof(sTerrainVertex)) != S_OK)
      {
         return E_FAIL;
      }

      (*pVertices)[i] = vertex;
   }

   return S_OK;
}

////////////////////////////////////////

tResult cReadWriteOps<tTerrainVertexVector>::Write(IWriter * pWriter,
                                                   const tTerrainVertexVector & vertices)
{
   Assert(pWriter != NULL);

   if (pWriter->Write(vertices.size()) != S_OK)
   {
      return E_FAIL;
   }

   std::vector<sTerrainVertex>::const_iterator iter;
   for (iter = vertices.begin(); iter != vertices.end(); iter++)
   {
      if (pWriter->Write((void *)&(*iter), sizeof(sTerrainVertex)) != S_OK)
      {
         return E_FAIL;
      }
   }

   return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrain
//

////////////////////////////////////////

cTerrain::cTerrain()
 : m_xDim(0),
   m_zDim(0),
   m_tileSize(0),
   m_xChunks(0),
   m_zChunks(0)
{
}

////////////////////////////////////////

cTerrain::~cTerrain()
{
}

////////////////////////////////////////

tResult cTerrain::Read(IReader * pReader)
{
   if (pReader == NULL)
   {
      return E_POINTER;
   }

   uint terrainFileId = 0;
   if (pReader->Read(&terrainFileId) != S_OK)
   {
      return E_FAIL;
   }

   if (terrainFileId != kTerrainFileId)
   {
      DebugMsg("Not a terrain map file\n");
      return E_FAIL;
   }

   uint terrainFileVer = 0;
   if (pReader->Read(&terrainFileVer) != S_OK)
   {
      return E_FAIL;
   }

   if (terrainFileVer != kTerrainFileVersion)
   {
      DebugMsg("Incorrect version in file\n");
      return E_FAIL;
   }

   if (pReader->Read(&m_xDim) != S_OK ||
      pReader->Read(&m_zDim) != S_OK ||
      pReader->Read(&m_tileSize) != S_OK ||
      pReader->Read(&m_xChunks) != S_OK ||
      pReader->Read(&m_zChunks) != S_OK ||
      pReader->Read(&m_vertices) != S_OK ||
      pReader->Read(&m_tilesetName, 0) != S_OK)
   {
      return E_FAIL;
   }

   return S_OK;
}

////////////////////////////////////////

tResult cTerrain::Write(IWriter * pWriter)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (pWriter->Write(kTerrainFileId) != S_OK ||
      pWriter->Write(kTerrainFileVersion) != S_OK ||
      pWriter->Write(m_xDim) != S_OK ||
      pWriter->Write(m_zDim) != S_OK ||
      pWriter->Write(m_tileSize) != S_OK ||
      pWriter->Write(m_xChunks) != S_OK ||
      pWriter->Write(m_zChunks) != S_OK ||
      pWriter->Write(m_vertices) != S_OK ||
      pWriter->Write(m_tilesetName.c_str()) != S_OK)
   {
      return E_FAIL;
   }

   return S_OK;
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

   pTileSet->GetName(&m_tilesetName);

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

   if (xDim > kTilesPerChunk)
   {
      m_xChunks = xDim / kTilesPerChunk;
   }

   if (zDim > kTilesPerChunk)
   {
      m_zChunks = zDim / kTilesPerChunk;
   }

   CreateTerrainChunks();

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

const sTerrainVertex * cTerrain::GetVertexPointer() const
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
   Assert(m_xChunks > 0 && m_zChunks > 0);

   for (int i = 0; i < m_zChunks; i++)
   {
      for (int j = 0; j < m_xChunks; j++)
      {
      }
   }

   return false;
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainTile
//

////////////////////////////////////////

cTerrainTile::cTerrainTile()
{
}

////////////////////////////////////////

cTerrainTile::~cTerrainTile()
{
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainChunk
//

////////////////////////////////////////

cTerrainChunk::cTerrainChunk()
{
}

////////////////////////////////////////

cTerrainChunk::~cTerrainChunk()
{
}

/////////////////////////////////////////////////////////////////////////////
