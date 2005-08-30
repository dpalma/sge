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

#include <algorithm>
#include <map>
#include <GL/glew.h>

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
// CLASS: cTerrainSettings
//

////////////////////////////////////////

cTerrainSettings::cTerrainSettings()
 : m_tileSize(TerrainSettingsDefaults::kTerrainTileSize),
   m_nTilesX(TerrainSettingsDefaults::kTerrainTileCountX),
   m_nTilesZ(TerrainSettingsDefaults::kTerrainTileCountZ),
   m_initialTile(0),
   m_heightData(TerrainSettingsDefaults::kTerrainHeightData)
{
}

////////////////////////////////////////

cTerrainSettings::cTerrainSettings(const cTerrainSettings & other)
 : m_tileSize(other.m_tileSize),
   m_nTilesX(other.m_nTilesX),
   m_nTilesZ(other.m_nTilesZ),
   m_tileSet(other.m_tileSet),
   m_initialTile(other.m_initialTile),
   m_heightData(other.m_heightData),
   m_heightMap(other.m_heightMap)
{
}

////////////////////////////////////////

cTerrainSettings::~cTerrainSettings()
{
}

////////////////////////////////////////

const cTerrainSettings & cTerrainSettings::operator =(const cTerrainSettings & other)
{
   m_tileSize = other.m_tileSize;
   m_nTilesX = other.m_nTilesX;
   m_nTilesZ = other.m_nTilesZ;
   m_tileSet = other.m_tileSet;
   m_initialTile = other.m_initialTile;
   m_heightData = other.m_heightData;
   m_heightMap = other.m_heightMap;
   return *this;
}

////////////////////////////////////////

void cTerrainSettings::SetTileSize(uint tileSize)
{
   m_tileSize = tileSize;
}

////////////////////////////////////////

uint cTerrainSettings::GetTileSize() const
{
   return m_tileSize;
}

////////////////////////////////////////

void cTerrainSettings::SetTileCountX(uint tileCountX)
{
   m_nTilesX = tileCountX;
}

////////////////////////////////////////

uint cTerrainSettings::GetTileCountX() const
{
   return m_nTilesX;
}

////////////////////////////////////////

void cTerrainSettings::SetTileCountZ(uint tileCountZ)
{
   m_nTilesZ = tileCountZ;
}

////////////////////////////////////////

uint cTerrainSettings::GetTileCountZ() const
{
   return m_nTilesZ;
}

////////////////////////////////////////

void cTerrainSettings::SetTileSet(const tChar * pszTileSet)
{
   if (pszTileSet != NULL)
   {
      m_tileSet = pszTileSet;
   }
   else
   {
      m_tileSet.erase();
   }
}

////////////////////////////////////////

const tChar * cTerrainSettings::GetTileSet() const
{
   return m_tileSet.c_str();
}

////////////////////////////////////////

void cTerrainSettings::SetInitialTile(int initialTile)
{
   m_initialTile = initialTile;
}

////////////////////////////////////////

int cTerrainSettings::GetInitialTile() const
{
   return m_initialTile;
}

////////////////////////////////////////

void cTerrainSettings::SetHeightData(eTerrainHeightData heightData)
{
   m_heightData = heightData;
}

////////////////////////////////////////

eTerrainHeightData cTerrainSettings::GetHeightData() const
{
   return m_heightData;
}

////////////////////////////////////////

void cTerrainSettings::SetHeightMap(const tChar * pszHeightMap)
{
   if (pszHeightMap != NULL)
   {
      m_heightMap = pszHeightMap;
   }
   else
   {
      m_heightMap.erase();
   }
}

////////////////////////////////////////

const tChar * cTerrainSettings::GetHeightMap() const
{
   return m_heightMap.c_str();
}

////////////////////////////////////////

tResult cReadWriteOps<cTerrainSettings>::Read(IReader * pReader, cTerrainSettings * pTerrainSettings)
{
   if (pReader == NULL || pTerrainSettings == NULL)
   {
      return E_POINTER;
   }

   if (pReader->Read(&pTerrainSettings->m_tileSize) == S_OK
      && pReader->Read(&pTerrainSettings->m_nTilesX) == S_OK
      && pReader->Read(&pTerrainSettings->m_nTilesZ) == S_OK
      && pReader->Read(&pTerrainSettings->m_tileSet) == S_OK
      && pReader->Read(&pTerrainSettings->m_initialTile) == S_OK
      && pReader->Read(&pTerrainSettings->m_heightData) == S_OK
      && pReader->Read(&pTerrainSettings->m_heightMap) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cReadWriteOps<cTerrainSettings>::Write(IWriter * pWriter, const cTerrainSettings & terrainSettings)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (pWriter->Write(terrainSettings.m_tileSize) == S_OK
      && pWriter->Write(terrainSettings.m_nTilesX) == S_OK
      && pWriter->Write(terrainSettings.m_nTilesZ) == S_OK
      && pWriter->Write(terrainSettings.m_tileSet) == S_OK
      && pWriter->Write(terrainSettings.m_initialTile) == S_OK
      && pWriter->Write(terrainSettings.m_heightData) == S_OK
      && pWriter->Write(terrainSettings.m_heightMap) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainModel
//

////////////////////////////////////////

tResult TerrainModelCreate()
{
   cAutoIPtr<ITerrainModel> p(new cTerrainModel);
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_ITerrainModel, static_cast<ITerrainModel*>(p));
}

////////////////////////////////////////

cTerrainModel::cTerrainModel()
{
}

////////////////////////////////////////

cTerrainModel::~cTerrainModel()
{
}

////////////////////////////////////////

tResult cTerrainModel::Init()
{
   return S_OK;
}

////////////////////////////////////////

tResult cTerrainModel::Term()
{
   return S_OK;
}

////////////////////////////////////////

tResult cTerrainModel::Initialize(const cTerrainSettings & terrainSettings)
{
   UseGlobal(EditorTileManager);
   if (pEditorTileManager->GetTileSet(terrainSettings.GetTileSet(), &m_pTileSet) != S_OK)
   {
      WarnMsg1("Unable to find tile set \"%s\"; using default instead\n",
         terrainSettings.GetTileSet() == NULL ? "(NULL)" : terrainSettings.GetTileSet());
      pEditorTileManager->GetDefaultTileSet(&m_pTileSet);
   }

   cAutoIPtr<IHeightMap> pHeightMap;
   if (terrainSettings.GetHeightData() == kTHD_HeightMap)
   {
      if (HeightMapLoad(terrainSettings.GetHeightMap(), &pHeightMap) != S_OK)
      {
         return E_FAIL;
      }
   }
   else if (terrainSettings.GetHeightData() == kTHD_Noise)
   {
      return E_NOTIMPL;
   }
   else
   {
      if (HeightMapCreateFixed(0, &pHeightMap) != S_OK)
      {
         return E_FAIL;
      }
   }

   if (InitQuads(terrainSettings.GetTileCountX(), terrainSettings.GetTileCountZ(),
      terrainSettings.GetInitialTile(), pHeightMap, &m_terrainQuads) == S_OK)
   {
      NotifyListeners(&ITerrainModelListener::OnTerrainInitialize);
      return S_OK;
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cTerrainModel::Clear()
{
   SafeRelease(m_pTileSet);
   m_terrainQuads.clear();
   NotifyListeners(&ITerrainModelListener::OnTerrainClear);
   return S_OK;
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

   if (pReader->Read(&m_terrainSettings) != S_OK)
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
      pWriter->Write(m_terrainSettings) != S_OK)
   {
      return E_FAIL;
   }

   return S_OK;
}

////////////////////////////////////////

tResult cTerrainModel::GetTerrainSettings(cTerrainSettings * pTerrainSettings) const
{
   if (pTerrainSettings == NULL)
   {
      return E_POINTER;
   }
   *pTerrainSettings = m_terrainSettings;
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

tResult cTerrainModel::GetTileSet(IEditorTileSet * * ppTileSet)
{
   return m_pTileSet.GetPointer(ppTileSet);
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

tResult cTerrainModel::SetQuadTile(uint quadx, uint quadz, uint tile, uint * pFormer)
{
   if (quadx < m_terrainSettings.GetTileCountX() && quadz < m_terrainSettings.GetTileCountZ())
   {
      uint index = (quadz * m_terrainSettings.GetTileCountZ()) + quadx;
      if (index < m_terrainQuads.size())
      {
         if (pFormer != NULL)
         {
            *pFormer = m_terrainQuads[index].tile;
         }
         m_terrainQuads[index].tile = tile;
         NotifyListeners(&ITerrainModelListener::OnTerrainChange);
         return S_OK;
      }
   }
   return E_FAIL;
}

////////////////////////////////////////

tResult cTerrainModel::GetQuadTile(uint quadx, uint quadz, uint * pTile) const
{
   if (pTile == NULL)
   {
      return E_POINTER;
   }

   if (quadx >= m_terrainSettings.GetTileCountX()
      || quadz >= m_terrainSettings.GetTileCountZ())
   {
      return E_INVALIDARG;
   }

   uint index = (quadz * m_terrainSettings.GetTileCountZ()) + quadx;
   Assert(index < m_terrainQuads.size());

   *pTile = m_terrainQuads[index].tile;
   return S_OK;
}

////////////////////////////////////////

tResult cTerrainModel::GetTileIndices(float x, float z, uint * pix, uint * piz) const
{
   if (pix == NULL || piz == NULL)
   {
      return E_POINTER;
   }

   uint halfTile = m_terrainSettings.GetTileSize() >> 1;
   *pix = Round((x - halfTile) / m_terrainSettings.GetTileSize());
   *piz = Round((z - halfTile) / m_terrainSettings.GetTileSize());
   return S_OK;
}

////////////////////////////////////////

tResult cTerrainModel::GetQuadVertices(uint quadx, uint quadz, sTerrainVertex verts[4]) const
{
   if (verts == NULL)
   {
      return E_POINTER;
   }

   if (quadx >= m_terrainSettings.GetTileCountX()
      || quadz >= m_terrainSettings.GetTileCountZ())
   {
      return E_INVALIDARG;
   }

   uint index = (quadz * m_terrainSettings.GetTileCountZ()) + quadx;
   Assert(index < m_terrainQuads.size());

   memcpy(verts, m_terrainQuads[index].verts, 4 * sizeof(sTerrainVertex));
   return S_OK;
}

////////////////////////////////////////

tResult cTerrainModel::InitQuads(uint nTilesX, uint nTilesZ, uint tile, IHeightMap * pHeightMap, tTerrainQuads * pQuads)
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

         tq.tile = tile;

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

void cTerrainModel::NotifyListeners(void (ITerrainModelListener::*pfnListenerMethod)())
{
   tListeners::iterator iter = m_listeners.begin();
   tListeners::iterator end = m_listeners.end();
   for (; iter != end; iter++)
   {
      ((*iter)->*pfnListenerMethod)();
   }
}


/////////////////////////////////////////////////////////////////////////////

tResult HeightMapCreateFixed(float heightValue, IHeightMap * * ppHeightMap)
{
   if (ppHeightMap == NULL)
   {
      return E_POINTER;
   }

   class cFixedHeightMap : public cComObject<IMPLEMENTS(IHeightMap)>
   {
   public:
      cFixedHeightMap(float heightValue) : m_heightValue(heightValue)
      {
      }

      virtual float GetNormalizedHeight(float /*nx*/, float /*nz*/) const
      {
         return m_heightValue;
      }

   private:
      float m_heightValue;
   };

   *ppHeightMap = static_cast<IHeightMap *>(new cFixedHeightMap(heightValue));
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
