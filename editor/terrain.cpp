/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "terrain.h"
#include "editorapi.h"
#include "terrainapi.h"

#include "materialapi.h"
#include "renderapi.h"
#include "textureapi.h"
#include "color.h"

#include "resourceapi.h"
#include "imagedata.h"
#include "readwriteapi.h"
#include "globalobj.h"

#include <GL/gl.h>

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////

static const uint kMaxTerrainHeight = 30;

/////////////////////////////////////////////////////////////////////////////

static const union
{
   byte b[sizeof(uint)];
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
 : m_tileSize(0),
   m_nTilesX(0),
   m_nTilesZ(0),
   m_nChunksX(0),
   m_nChunksZ(0),
   m_sceneEntity(this)
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

   if (pReader->Read(&m_tileSize) != S_OK ||
      pReader->Read(&m_nTilesX) != S_OK ||
      pReader->Read(&m_nTilesZ) != S_OK ||
      pReader->Read(&m_nChunksX) != S_OK ||
      pReader->Read(&m_nChunksZ) != S_OK ||
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
      pWriter->Write(m_tileSize) != S_OK ||
      pWriter->Write(m_nTilesX) != S_OK ||
      pWriter->Write(m_nTilesZ) != S_OK ||
      pWriter->Write(m_nChunksX) != S_OK ||
      pWriter->Write(m_nChunksZ) != S_OK ||
      pWriter->Write(m_tileSetName.c_str()) != S_OK)
   {
      return E_FAIL;
   }

   return S_OK;
}

////////////////////////////////////////

tResult cTerrain::Init(uint nTilesX, uint nTilesZ, IEditorTileSet * pTileSet, IHeightMap * pHeightMap)
{
   if (nTilesX == 0 || nTilesZ == 0)
   {
      return E_INVALIDARG;
   }

   if (pTileSet == NULL || pHeightMap == NULL)
   {
      return E_POINTER;
   }

   if (FAILED(InitQuads(nTilesX, nTilesZ, pHeightMap, &m_terrainQuads)))
   {
      return E_FAIL;
   }

   m_tileSize = kDefaultStepSize;
   m_nTilesX = nTilesX;
   m_nTilesZ = nTilesZ;

   Assert(!m_pTileSet);
   Assert(m_tileSetName.empty());

   m_pTileSet = CTAddRef(pTileSet);
   if (pTileSet->GetName(&m_tileSetName) != S_OK)
   {
      return E_FAIL;
   }

   return S_OK;
}

////////////////////////////////////////

tResult cTerrain::InitQuads(uint nTilesX, uint nTilesZ, IHeightMap * pHeightMap, tTerrainQuads * pQuads)
{
   if (nTilesX == 0 || nTilesZ == 0)
   {
      return E_INVALIDARG;
   }

   if (pHeightMap == NULL || pQuads == NULL)
   {
      return E_POINTER;
   }

   uint nQuads = nTilesX * nTilesZ;
   pQuads->resize(nQuads);

   static const uint stepSize = kDefaultStepSize;

   uint extentX = nTilesX * stepSize;
   uint extentZ = nTilesZ * stepSize;

   int iQuad = 0;
   float z = 0;
   float z2 = static_cast<float>(stepSize);
   for (uint iz = 0; iz < nTilesZ; iz++, z += stepSize, z2 += stepSize)
   {
      float x = 0;
      float x2 = static_cast<float>(stepSize);
      for (uint ix = 0; ix < nTilesX; ix++, x += stepSize, x2 += stepSize, iQuad++)
      {
         sTerrainQuad & tq = pQuads->at(iQuad);

         tq.tile = 0;

         tq.verts[0].color = ARGB(255,192,192,192);
         tq.verts[1].color = ARGB(255,192,192,192);
         tq.verts[2].color = ARGB(255,192,192,192);
         tq.verts[3].color = ARGB(255,192,192,192);

         tq.verts[0].uv1 = tVec2(0,0);
         tq.verts[1].uv1 = tVec2(1,0);
         tq.verts[2].uv1 = tVec2(1,1);
         tq.verts[3].uv1 = tVec2(0,1);

#define Height(xx,zz) (pHeightMap->GetNormalizedHeight((xx)/extentX,(zz)/extentZ)*kMaxTerrainHeight)
         tq.verts[0].pos = tVec3(x, Height(x,z), z);
         tq.verts[1].pos = tVec3(x2, Height(x2,z), z);
         tq.verts[2].pos = tVec3(x2, Height(x2,z2), z2);
         tq.verts[3].pos = tVec3(x, Height(x,z2), z2);
#undef Height
      }
   }

   return S_OK;
}

////////////////////////////////////////

void cTerrain::GetDimensions(uint * pxd, uint * pzd) const
{
   if (pxd != NULL)
   {
      *pxd = m_nTilesX;
   }

   if (pzd != NULL)
   {
      *pzd = m_nTilesZ;
   }
}

////////////////////////////////////////

void cTerrain::GetExtents(uint * px, uint * pz) const
{
   if (px != NULL)
   {
      *px = m_tileSize * m_nTilesX;
   }

   if (pz != NULL)
   {
      *pz = m_tileSize * m_nTilesZ;
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
   if (tx < m_nTilesX && tz < m_nTilesZ)
   {
      uint index = (tz * m_nTilesZ) + tx;
      if (index < m_terrainQuads.size())
      {
         uint formerTerrain = m_terrainQuads[index].tile;
         m_terrainQuads[index].tile = terrain;
         return formerTerrain;
      }
   }
   return kInvalidUintValue;
}

////////////////////////////////////////

tResult cTerrain::GetTileVertices(uint tx, uint tz, tVec3 vertices[4]) const
{
   if (vertices == NULL)
   {
      return E_POINTER;
   }
   if (tx < m_nTilesX && tz < m_nTilesZ)
   {
      uint index = (tz * m_nTilesZ) + tx;
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

tResult cTerrain::GetSceneEntity(ISceneEntity * * ppSceneEntity)
{
   if (ppSceneEntity == NULL)
   {
      return E_POINTER;
   }
   *ppSceneEntity = &m_sceneEntity;
   return S_OK;
}

////////////////////////////////////////

cTerrain::cSceneEntity::cSceneEntity(cTerrain * pOuter)
 : m_pOuter(pOuter),
   m_translation(0,0,0),
   m_rotation(0,0,0,1),
   m_transform(tMatrix4::GetIdentity())
{
}

////////////////////////////////////////

cTerrain::cSceneEntity::~cSceneEntity()
{
}

////////////////////////////////////////

void cTerrain::cSceneEntity::Render(IRenderDevice * pRenderDevice)
{
   Assert(m_pOuter != NULL);
   if (m_pOuter->Render(pRenderDevice) == S_OK)
   {
   }
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

tResult HeightMapCreateSimple(float heightValue, IHeightMap * * ppHeightMap)
{
   if (ppHeightMap == NULL)
   {
      return E_POINTER;
   }

   class cSimpleHeightMap : public cComObject<IMPLEMENTS(IHeightMap)>
   {
   public:
      cSimpleHeightMap(float heightValue) : m_heightValue(heightValue)
      {
      }

      virtual float GetNormalizedHeight(float /*nx*/, float /*nz*/) const
      {
         return m_heightValue;
      }

   private:
      float m_heightValue;
   };

   *ppHeightMap = static_cast<IHeightMap *>(new cSimpleHeightMap(heightValue));
   if (*ppHeightMap == NULL)
   {
      return E_OUTOFMEMORY;
   }

   return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

tResult HeightMapLoad(const tChar * pszHeightData, IHeightMap * * ppHeightMap)
{
   if (pszHeightData == NULL || ppHeightMap == NULL)
   {
      return E_POINTER;
   }

   class cHeightMap : public cComObject<IMPLEMENTS(IHeightMap)>
   {
   public:
      cHeightMap(cImageData * pHeightData)
       : m_pHeightData(pHeightData)
      {
         Assert(pHeightData != NULL);
      }

      ~cHeightMap()
      {
         delete m_pHeightData;
         m_pHeightData = NULL;
      }

      virtual float GetNormalizedHeight(float nx, float nz) const
      {
         Assert(m_pHeightData != NULL);

         // support only grayscale images for now
         if (m_pHeightData->GetPixelFormat() != kPF_Grayscale)
         {
            return 0;
         }

         if ((nx < 0) || (nx > 1) || (nz < 0) || (nz > 1))
         {
            return 0;
         }

         uint x = Round(nx * m_pHeightData->GetWidth());
         uint z = Round(nz * m_pHeightData->GetHeight());

         uint8 * pData = reinterpret_cast<uint8 *>(m_pHeightData);

         uint8 sample = pData[(z * m_pHeightData->GetWidth()) + x];

         return static_cast<float>(sample) / 255.0f;
      }

   private:
      cImageData * m_pHeightData;
   };

   cImageData * pHeightData = NULL;
   cAutoIPtr<IResource> pRes;
   UseGlobal(ResourceManager2);
   if (pResourceManager2->Load(tResKey(pszHeightData, kRC_Image), &pRes) == S_OK)
   {
      if ((pRes->GetData((void**)&pHeightData) != S_OK)
         || (pHeightData == NULL))
      {
         return E_FAIL;
      }
   }

   *ppHeightMap = static_cast<IHeightMap *>(new cHeightMap(pHeightData));
   if (*ppHeightMap == NULL)
   {
      return E_OUTOFMEMORY;
   }

   return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
