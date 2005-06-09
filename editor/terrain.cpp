/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "terrain.h"
#include "editorapi.h"
#include "terrainapi.h"
#include "editorTypes.h"

#include "resourceapi.h"
#include "imagedata.h"
#include "readwriteapi.h"
#include "globalobj.h"
#include "connptimpl.h"

#include <GL/gl.h>
#include <algorithm>
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

const int kDefaultStepSize = 32;

static const uint kMaxTerrainHeight = 30;

/////////////////////////////////////////////////////////////////////////////

static const union
{
   byte b[8];
   uint32 id1, id2;
}
kTerrainFileIdGenerator = { { 's', 'g', 'e', 0, 'm', 'a', 'p', 0 } };

static const kTerrainFileId1 = kTerrainFileIdGenerator.id1;
static const kTerrainFileId2 = kTerrainFileIdGenerator.id2;

static const uint32 kTerrainFileVersion = 1;

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
// CLASS: cTerrainModel
//

////////////////////////////////////////

cTerrainModel::cTerrainModel()
 : m_tileSize(0),
   m_nTilesX(0),
   m_nTilesZ(0)
{
}

////////////////////////////////////////

cTerrainModel::~cTerrainModel()
{
}

////////////////////////////////////////

tResult cTerrainModel::Read(IReader * pReader)
{
   if (pReader == NULL)
   {
      return E_POINTER;
   }

   uint32 terrainFileId1 = 0, terrainFileId2 = 0;
   if (pReader->Read(&terrainFileId1) != S_OK
      || pReader->Read(&terrainFileId1) != S_OK)
   {
      return E_FAIL;
   }

   if (terrainFileId1 != kTerrainFileId1
      || terrainFileId2 != kTerrainFileId2)
   {
      ErrorMsg("Not a terrain map file\n");
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
      pReader->Read(&m_tileSetName, 0) != S_OK)
   {
      return E_FAIL;
   }

   return S_OK;
}

////////////////////////////////////////

tResult cTerrainModel::Write(IWriter * pWriter)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (pWriter->Write(kTerrainFileId1) != S_OK ||
      pWriter->Write(kTerrainFileId1) != S_OK ||
      pWriter->Write(kTerrainFileVersion) != S_OK ||
      pWriter->Write(m_tileSize) != S_OK ||
      pWriter->Write(m_nTilesX) != S_OK ||
      pWriter->Write(m_nTilesZ) != S_OK ||
      pWriter->Write(m_tileSetName.c_str()) != S_OK)
   {
      return E_FAIL;
   }

   return S_OK;
}

////////////////////////////////////////

tResult cTerrainModel::AddTerrainModelListener(ITerrainModelListener * pListener)
{
   return add_interface(m_listeners, pListener) ? S_OK : E_FAIL;
}

////////////////////////////////////////

tResult cTerrainModel::RemoveTerrainModelListener(ITerrainModelListener * pListener)
{
   return remove_interface(m_listeners, pListener) ? S_OK : E_FAIL;
}

////////////////////////////////////////

tResult cTerrainModel::GetDimensions(uint * pxd, uint * pzd) const
{
   if (pxd == NULL || pzd == NULL)
   {
      return E_POINTER;
   }

   *pxd = m_nTilesX;
   *pzd = m_nTilesZ;
   return S_OK;
}

////////////////////////////////////////

tResult cTerrainModel::GetExtents(uint * px, uint * pz) const
{
   if (px == NULL || pz == NULL)
   {
      return E_POINTER;
   }

   *px = m_tileSize * m_nTilesX;
   *pz = m_tileSize * m_nTilesZ;
   return S_OK;
}

////////////////////////////////////////

tResult cTerrainModel::GetTileSet(IEditorTileSet * * ppTileSet)
{
   return m_pTileSet.GetPointer(ppTileSet);
}

////////////////////////////////////////

const tTerrainQuads & cTerrainModel::GetTerrainQuads() const
{
   return m_terrainQuads;
}

////////////////////////////////////////

tTerrainQuads::const_iterator cTerrainModel::BeginTerrainQuads() const
{
   return m_terrainQuads.begin();
}

tTerrainQuads::const_iterator cTerrainModel::EndTerrainQuads() const
{
   return m_terrainQuads.end();
}

////////////////////////////////////////

tResult cTerrainModel::SetTileTerrain(uint tx, uint tz, uint terrain, uint * pFormer)
{
   if (tx < m_nTilesX && tz < m_nTilesZ)
   {
      uint index = (tz * m_nTilesZ) + tx;
      if (index < m_terrainQuads.size())
      {
         if (pFormer != NULL)
         {
            *pFormer = m_terrainQuads[index].tile;
         }
         m_terrainQuads[index].tile = terrain;
         NotifyListeners();
         return S_OK;
      }
   }
   return E_FAIL;
}

////////////////////////////////////////

tResult cTerrainModel::GetTileIndices(float x, float z, uint * pix, uint * piz) const
{
   if (pix == NULL || piz == NULL)
   {
      return E_POINTER;
   }

   uint halfTile = m_tileSize >> 1;
   *pix = Round((x - halfTile) / m_tileSize);
   *piz = Round((z - halfTile) / m_tileSize);
   return S_OK;
}

////////////////////////////////////////

tResult cTerrainModel::GetTileVertices(uint tx, uint tz, tVec3 vertices[4]) const
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

tResult cTerrainModel::Init(const cMapSettings & mapSettings)
{
   Assert(!m_pTileSet);
   Assert(m_tileSetName.empty());
   Assert(m_nTilesX == 0 && m_nTilesZ == 0);

   UseGlobal(EditorTileManager);
   if (pEditorTileManager->GetTileSet(mapSettings.GetTileSet(), &m_pTileSet) != S_OK)
   {
      WarnMsg1("Unable to find tile set \"%s\"; using default instead\n",
         mapSettings.GetTileSet() == NULL ? "(NULL)" : mapSettings.GetTileSet());
      pEditorTileManager->GetDefaultTileSet(&m_pTileSet);
   }

   if (!m_pTileSet || m_pTileSet->GetName(&m_tileSetName) != S_OK)
   {
      return E_FAIL;
   }

   cAutoIPtr<IHeightMap> pHeightMap;
   if (mapSettings.GetHeightMap(&pHeightMap) != S_OK)
   {
      return E_FAIL;
   }

   return Init(mapSettings.GetXDimension(), mapSettings.GetZDimension(), pHeightMap);
}

////////////////////////////////////////

tResult cTerrainModel::Init(uint nTilesX, uint nTilesZ, IHeightMap * pHeightMap)
{
   if (FAILED(InitQuads(nTilesX, nTilesZ, pHeightMap, &m_terrainQuads)))
   {
      return E_FAIL;
   }

   m_tileSize = kDefaultStepSize;
   m_nTilesX = nTilesX;
   m_nTilesZ = nTilesZ;

   NotifyListeners();

   return S_OK;
}

////////////////////////////////////////

tResult cTerrainModel::InitQuads(uint nTilesX, uint nTilesZ, IHeightMap * pHeightMap, tTerrainQuads * pQuads)
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
   for (uint iz = 0; iz < nTilesZ; iz++, z += stepSize)
   {
      float x = 0;
      for (uint ix = 0; ix < nTilesX; ix++, x += stepSize, iQuad++)
      {
         sTerrainQuad & tq = pQuads->at(iQuad);

         tq.tile = 0;

         tq.verts[0].uv1 = tVec2(0,0);
         tq.verts[1].uv1 = tVec2(1,0);
         tq.verts[2].uv1 = tVec2(1,1);
         tq.verts[3].uv1 = tVec2(0,1);

#define Height(xx,zz) (pHeightMap->GetNormalizedHeight((xx)/extentX,(zz)/extentZ)*kMaxTerrainHeight)
         tq.verts[0].pos = tVec3(x, Height(x,z), z);
         tq.verts[1].pos = tVec3(x+stepSize, Height(x+stepSize,z), z);
         tq.verts[2].pos = tVec3(x+stepSize, Height(x+stepSize,z+stepSize), z+stepSize);
         tq.verts[3].pos = tVec3(x, Height(x,z+stepSize), z+stepSize);
#undef Height
      }
   }

   return S_OK;
}

////////////////////////////////////////

void cTerrainModel::NotifyListeners()
{
   tListeners::iterator iter = m_listeners.begin();
   tListeners::iterator end = m_listeners.end();
   for (; iter != end; iter++)
   {
      (*iter)->OnTerrainChange();
   }
}

////////////////////////////////////////

tResult TerrainModelCreate(const cMapSettings & mapSettings, ITerrainModel * * ppTerrainModel)
{
   if (ppTerrainModel == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<cTerrainModel> pTM(new cTerrainModel);
   if (!pTM)
   {
      return E_OUTOFMEMORY;
   }

   if (pTM->Init(mapSettings) != S_OK)
   {
      return E_FAIL;
   }

   *ppTerrainModel = CTAddRef(static_cast<ITerrainModel*>(pTM));

   return S_OK;
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
         m_pHeightData = NULL; // Don't delete this--it's a cached resource
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
   UseGlobal(ResourceManager);
   if (pResourceManager->Load(tResKey(pszHeightData, kRC_Image), (void**)&pHeightData) != S_OK)
   {
      return E_FAIL;
   }

   *ppHeightMap = static_cast<IHeightMap *>(new cHeightMap(pHeightData));
   if (*ppHeightMap == NULL)
   {
      return E_OUTOFMEMORY;
   }

   return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
