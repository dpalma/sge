/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "terrain.h"
#include "editorapi.h"
#include "heightmap.h"

#include "materialapi.h"
#include "renderapi.h"
#include "textureapi.h"
#include "color.h"

#include "readwriteapi.h"

#include <GL/gl.h>

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
   { kVDU_Color, kVDT_Color },
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
      pReader->Read(&m_tileSetName, 0) != S_OK)
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
      pWriter->Write(m_tileSetName.c_str()) != S_OK)
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

   pTileSet->GetName(&m_tileSetName);
   m_pTileSet = CTAddRef(pTileSet);

   static const uint32 color = ARGB(255,192,192,192);

   uint nTileImages = pTile->GetHorizontalImageCount() * pTile->GetVerticalImageCount();

   Assert(IsPowerOfTwo(nTileImages));

   if (!IsPowerOfTwo(nTileImages))
   {
      DebugMsg("Number of variations in a tile must be a power of two\n");
      return false;
   }

   float tileTexWidth = 1.0f / pTile->GetHorizontalImageCount();
   float tileTexHeight = 1.0f / pTile->GetVerticalImageCount();

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

void cTerrain::GetTileIndices(float x, float z, uint * pix, uint * piz)
{
   uint halfTile = m_tileSize >> 1;

   if (pix != NULL)
   {
      *pix = Round((x - halfTile) / m_tileSize);
   }

   if (piz != NULL)
   {
      *piz = Round((z - halfTile) / m_tileSize);
   }
}

////////////////////////////////////////

tResult cTerrain::Render(IRenderDevice * pRenderDevice)
{
   glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);

   glEnable(GL_COLOR_MATERIAL);

   //std::vector<cTerrainTile>::iterator iter;
   //for (iter = m_tiles.begin(); iter != m_tiles.end(); iter++)
   //{
   //   const sTerrainVertex * pVertices = iter->GetVertices();
   tTerrainQuads::iterator iter = m_terrainQuads.begin();
   tTerrainQuads::iterator end = m_terrainQuads.end();
   for (; iter != end; iter++)
   {
      const sTerrainVertex * pVertices = iter->verts;

      cAutoIPtr<IEditorTile> pEditorTile;
      if (m_pTileSet->GetTile(iter->tile, &pEditorTile) == S_OK)
      {
         cAutoIPtr<ITexture> pTexture;
         if (pEditorTile->GetTexture(&pTexture) == S_OK)
         {
            HANDLE tex;
            if (pTexture->GetTextureHandle(&tex) == S_OK)
            {
               glEnable(GL_TEXTURE_2D);
               glBindTexture(GL_TEXTURE_2D, (uint)tex);
            }
         }
      }

      glBegin(GL_QUADS);

      glNormal3f(1,1,1);

      glColor4ubv((const byte *)&pVertices[0].color);
      glTexCoord2fv(pVertices[0].uv1.v);
      glVertex3fv(pVertices[0].pos.v);

      glColor4ubv((const byte *)&pVertices[3].color);
      glTexCoord2fv(pVertices[3].uv1.v);
      glVertex3fv(pVertices[3].pos.v);

      glColor4ubv((const byte *)&pVertices[2].color);
      glTexCoord2fv(pVertices[2].uv1.v);
      glVertex3fv(pVertices[2].pos.v);

      glColor4ubv((const byte *)&pVertices[1].color);
      glTexCoord2fv(pVertices[1].uv1.v);
      glVertex3fv(pVertices[1].pos.v);

      glEnd();
   }

   glPopAttrib();

   return S_OK;
}

////////////////////////////////////////

uint cTerrain::SetTileTerrain(uint tx, uint tz, uint terrain)
{
   if (tx < m_xDim && tz < m_zDim)
   {
      uint index = (tz * m_zDim) + tx;
      if (index < m_terrainQuads.size())
      {
         uint formerTerrain = m_terrainQuads[index].tile;
         m_terrainQuads[index].tile = terrain;
         return formerTerrain;
      }
   }
   return kInvalidTerrain;
}

////////////////////////////////////////

tResult cTerrain::GetTileVertices(uint tx, uint tz, tVec3 vertices[4]) const
{
   if (vertices == NULL)
   {
      return E_POINTER;
   }
   if (tx < m_xDim && tz < m_zDim)
   {
      uint index = (tz * m_zDim) + tx;
      if (index < m_terrainQuads.size())
      {
         const sTerrainVertex * pVertices = m_terrainQuads[index].verts;
         vertices[0] = pVertices[0].pos;
         vertices[1] = pVertices[1].pos;
         vertices[2] = pVertices[2].pos;
         vertices[3] = pVertices[3].pos;
         return S_OK;
      }
   }
   return E_FAIL;
}

////////////////////////////////////////

void cTerrain::InitializeVertices(uint xDim, uint zDim, int stepSize, cHeightMap * pHeightMap)
{
   uint nQuads = xDim * zDim;

   m_terrainQuads.resize(nQuads);

   m_vertices.resize(nQuads * 4);

   int index = 0, iQuad = 0;

   float z = 0;
   float z2 = static_cast<float>(stepSize);
   for (uint iz = 0; iz < zDim; iz++, z += stepSize, z2 += stepSize)
   {
      float x = 0;
      float x2 = static_cast<float>(stepSize);
      for (uint ix = 0; ix < xDim; ix++, x += stepSize, x2 += stepSize, iQuad++)
      {
         m_terrainQuads[iQuad].tile = 0;

         m_terrainQuads[iQuad].verts[0].color = ARGB(255,192,192,192);
         m_terrainQuads[iQuad].verts[1].color = ARGB(255,192,192,192);
         m_terrainQuads[iQuad].verts[2].color = ARGB(255,192,192,192);
         m_terrainQuads[iQuad].verts[3].color = ARGB(255,192,192,192);

         m_terrainQuads[iQuad].verts[0].uv1 = tVec2(0,0);
         m_terrainQuads[iQuad].verts[1].uv1 = tVec2(1,0);
         m_terrainQuads[iQuad].verts[2].uv1 = tVec2(1,1);
         m_terrainQuads[iQuad].verts[3].uv1 = tVec2(0,1);

#define Height(xx,zz) ((pHeightMap != NULL) ? pHeightMap->Height(Round(xx),Round(zz)) : 0)

         m_terrainQuads[iQuad].verts[0].pos = tVec3(x, Height(x,z), z);
         m_terrainQuads[iQuad].verts[1].pos = tVec3(x2, Height(x2,z), z);
         m_terrainQuads[iQuad].verts[2].pos = tVec3(x2, Height(x2,z2), z2);
         m_terrainQuads[iQuad].verts[3].pos = tVec3(x, Height(x,z2), z2);

         m_vertices[index++].pos = tVec3(x, Height(x,z), z);
         m_vertices[index++].pos = tVec3(x2, Height(x2,z), z);
         m_vertices[index++].pos = tVec3(x2, Height(x2,z2), z2);
         m_vertices[index++].pos = tVec3(x, Height(x,z2), z2);

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

   for (uint i = 0; i < m_zChunks; i++)
   {
      for (uint j = 0; j < m_xChunks; j++)
      {
      }
   }

   return false;
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
