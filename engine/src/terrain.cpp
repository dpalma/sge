/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "terrain.h"

#include "engineapi.h"
#include "ray.h"

#include "resourceapi.h"
#include "imageapi.h"
#include "readwriteapi.h"
#include "globalobj.h"
#include "connptimpl.h"

#include <algorithm>
#include <map>
#include <GL/glew.h>
#include <tinyxml.h>

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(TerrainModel);
#define LocalMsg(msg)            DebugMsgEx(TerrainModel,(msg))
#define LocalMsg1(msg,a)         DebugMsgEx1(TerrainModel,(msg),(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(TerrainModel,(msg),(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(TerrainModel,(msg),(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)   DebugMsgEx4(TerrainModel,(msg),(a),(b),(c),(d))

/////////////////////////////////////////////////////////////////////////////

const int kDefaultStepSize = 32;

static const uint kMaxTerrainHeight = 30;


/////////////////////////////////////////////////////////////////////////////

template <typename HTYPE>
inline void ComposeHandle(uint16 h, uint16 l, HTYPE * pHandle)
{
   *pHandle = (HTYPE)((((uint)h) << 16) | ((uint)l));
}

template <typename HTYPE>
inline void DecomposeHandle(HTYPE handle, uint16 * ph, uint16 * pl)
{
   uint u = (uint)handle;
   *ph = static_cast<uint16>((u >> 16) & 0xFFFF);
   *pl = static_cast<uint16>(u & 0xFFFF);
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainSettings
//

////////////////////////////////////////

cTerrainSettings::cTerrainSettings()
 : m_tileSize(TerrainSettingsDefaults::kTerrainTileSize)
 , m_nTilesX(TerrainSettingsDefaults::kTerrainTileCountX)
 , m_nTilesZ(TerrainSettingsDefaults::kTerrainTileCountZ)
 , m_initialTile(0)
 , m_heightData(TerrainSettingsDefaults::kTerrainHeightData)
 , m_heightMapScale(1)
{
}

////////////////////////////////////////

cTerrainSettings::cTerrainSettings(const cTerrainSettings & other)
 : m_tileSize(other.m_tileSize)
 , m_nTilesX(other.m_nTilesX)
 , m_nTilesZ(other.m_nTilesZ)
 , m_tileSet(other.m_tileSet)
 , m_initialTile(other.m_initialTile)
 , m_heightData(other.m_heightData)
 , m_heightMap(other.m_heightMap)
 , m_heightMapScale(other.m_heightMapScale)
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
   m_heightMapScale = other.m_heightMapScale;
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

void cTerrainSettings::SetHeightMapScale(float scale)
{
   m_heightMapScale = scale;
}

////////////////////////////////////////

float cTerrainSettings::GetHeightMapScale() const
{
   return m_heightMapScale;
}

////////////////////////////////////////////////////////////////////////////////

template <>
class cReadWriteOps<cTerrainSettings>
{
public:
   static tResult Read(IReader * pReader, cTerrainSettings * pTerrainSettings);
   static tResult Write(IWriter * pWriter, const cTerrainSettings & terrainSettings);
};

////////////////////////////////////////

tResult cReadWriteOps<cTerrainSettings>::Read(IReader * pReader, cTerrainSettings * pTerrainSettings)
{
   if (pReader == NULL || pTerrainSettings == NULL)
   {
      return E_POINTER;
   }

   int version;
   if (pReader->Read(&version) != S_OK || version != 1)
   {
      return E_FAIL;
   }

   int heightData = 0;
   if (pReader->Read(&pTerrainSettings->m_tileSize) == S_OK
      && pReader->Read(&pTerrainSettings->m_nTilesX) == S_OK
      && pReader->Read(&pTerrainSettings->m_nTilesZ) == S_OK
      && pReader->Read(&pTerrainSettings->m_tileSet) == S_OK
      && pReader->Read(&pTerrainSettings->m_initialTile) == S_OK
      && pReader->Read(&heightData) == S_OK
      && pReader->Read(&pTerrainSettings->m_heightMap) == S_OK
      && pReader->Read(&pTerrainSettings->m_heightMapScale) == S_OK)
   {
      pTerrainSettings->m_heightData = (eTerrainHeightData)heightData;
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

   if (pWriter->Write(1) == S_OK // version
      && pWriter->Write(terrainSettings.m_tileSize) == S_OK
      && pWriter->Write(terrainSettings.m_nTilesX) == S_OK
      && pWriter->Write(terrainSettings.m_nTilesZ) == S_OK
      && pWriter->Write(terrainSettings.m_tileSet) == S_OK
      && pWriter->Write(terrainSettings.m_initialTile) == S_OK
      && pWriter->Write(static_cast<int>(terrainSettings.m_heightData)) == S_OK
      && pWriter->Write(terrainSettings.m_heightMap) == S_OK
      && pWriter->Write(terrainSettings.m_heightMapScale) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}


////////////////////////////////////////////////////////////////////////////////

template <>
class cReadWriteOps<tUints>
{
public:
   static tResult Read(IReader * pReader, tUints * pUints);
   static tResult Write(IWriter * pWriter, const tUints & uints);
};

////////////////////////////////////////

tResult cReadWriteOps<tUints>::Read(IReader * pReader, tUints * pUints)
{
   if (pReader == NULL || pUints == NULL)
   {
      return E_POINTER;
   }

   tUints::size_type nUints = 0;
   if (pReader->Read(&nUints) == S_OK)
   {
      pUints->resize(nUints);
      if (pReader->Read(&(*pUints)[0], nUints * sizeof(tUints::value_type)) == S_OK)
      {
         return S_OK;
      }
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cReadWriteOps<tUints>::Write(IWriter * pWriter, const tUints & uints)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (pWriter->Write(uints.size()) == S_OK
      && pWriter->Write((void*)&uints[0], uints.size() * sizeof(tUints::value_type)) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}


////////////////////////////////////////////////////////////////////////////////

template <>
class cReadWriteOps<tVec3s>
{
public:
   static tResult Read(IReader * pReader, tVec3s * pVec3s);
   static tResult Write(IWriter * pWriter, const tVec3s & vec3s);
};

////////////////////////////////////////

tResult cReadWriteOps<tVec3s>::Read(IReader * pReader, tVec3s * pVec3s)
{
   if (pReader == NULL || pVec3s == NULL)
   {
      return E_POINTER;
   }

   tVec3s::size_type nVec3s = 0;
   if (pReader->Read(&nVec3s) == S_OK)
   {
      pVec3s->resize(nVec3s);
      if (pReader->Read(&(*pVec3s)[0], nVec3s * sizeof(tVec3s::value_type)) == S_OK)
      {
         return S_OK;
      }
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cReadWriteOps<tVec3s>::Write(IWriter * pWriter, const tVec3s & vec3s)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (pWriter->Write(vec3s.size()) == S_OK
      && pWriter->Write((void*)&vec3s[0], vec3s.size() * sizeof(tVec3s::value_type)) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEnumTerrainQuads
//

////////////////////////////////////////

cEnumTerrainQuads::cEnumTerrainQuads(uint xStart, uint xEnd, uint zStart, uint zEnd)
 : m_xStart(xStart)
 , m_xEnd(xEnd)
 , m_zStart(zStart)
 , m_zEnd(zEnd)
 , m_x(xStart)
 , m_z(zStart)
{
}

////////////////////////////////////////

cEnumTerrainQuads::~cEnumTerrainQuads()
{
}

////////////////////////////////////////

tResult cEnumTerrainQuads::Next(ulong count, HTERRAINQUAD * pQuads, ulong * pnQuads)
{
   if (pQuads == NULL || pnQuads == NULL)
   {
      return E_POINTER;
   }

   return DoNext(count, pQuads, pnQuads);
}

////////////////////////////////////////

tResult cEnumTerrainQuads::Skip(ulong count)
{
   return DoNext(count, NULL, NULL);
}

////////////////////////////////////////
// Used to shared code between Next and Skip

tResult cEnumTerrainQuads::DoNext(ulong count, HTERRAINQUAD * pQuads, ulong * pnQuads)
{
   if (count == 0)
   {
      return S_FALSE;
   }

   if (m_x == m_xEnd && m_z == m_zEnd)
   {
      return E_FAIL;
   }

   ulong nEnumerated = 0;
   for (ulong i = 0; i < count; i++)
   {
      if (m_z < m_zEnd && m_x < m_xEnd)
      {
         if (pQuads != NULL)
         {
            ComposeHandle(m_x, m_z, &pQuads[i]);
         }
         nEnumerated += 1;
      }
      m_x++;
      if (m_x == m_xEnd)
      {
         m_z++;
         if (m_z == m_zEnd)
         {
            break;
         }
         m_x = m_xStart;
      }
   }

   if (pnQuads != NULL)
   {
      *pnQuads = nEnumerated;
   }

   return (count == nEnumerated) ? S_OK : S_FALSE;
}

////////////////////////////////////////

tResult cEnumTerrainQuads::Reset()
{
   m_x = m_xStart;
   m_z = m_zStart;
   return S_OK;
}

////////////////////////////////////////

tResult cEnumTerrainQuads::Clone(IEnumTerrainQuads * * ppEnum)
{
   if (ppEnum == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IEnumTerrainQuads> pEnum(static_cast<IEnumTerrainQuads*>(
      new cEnumTerrainQuads(m_xStart, m_xEnd, m_zStart, m_zEnd)));

   if (!pEnum)
   {
      return E_OUTOFMEMORY;
   }

   *ppEnum = CTAddRef(pEnum);
   return S_OK;
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

BEGIN_CONSTRAINTS(cTerrainModel)
   AFTER_GUID(IID_ISaveLoadManager)
END_CONSTRAINTS()

////////////////////////////////////////

F_DECLARE_GUID(SAVELOADID_TerrainModel);

static const int g_terrainModelVer = 1;

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
   UseGlobal(SaveLoadManager);
   pSaveLoadManager->RegisterSaveLoadParticipant(SAVELOADID_TerrainModel,
      g_terrainModelVer, static_cast<ISaveLoadParticipant*>(this));
   return S_OK;
}

////////////////////////////////////////

tResult cTerrainModel::Term()
{
   UseGlobal(SaveLoadManager);
   pSaveLoadManager->RevokeSaveLoadParticipant(SAVELOADID_TerrainModel, g_terrainModelVer);
   return S_OK;
}

////////////////////////////////////////

tResult cTerrainModel::Initialize(const cTerrainSettings & terrainSettings)
{
   if (_tcslen(terrainSettings.GetTileSet()) == 0)
   {
      return E_FAIL;
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

   m_quadTiles.resize(terrainSettings.GetTileCountX() * terrainSettings.GetTileCountZ(), terrainSettings.GetInitialTile());

   uint nVertsX = terrainSettings.GetTileCountX() + 1, nVertsZ = terrainSettings.GetTileCountZ() + 1;
   m_vertices.resize(nVertsX * nVertsZ);

   uint extentX = terrainSettings.GetTileCountX() * terrainSettings.GetTileSize();
   uint extentZ = terrainSettings.GetTileCountZ() * terrainSettings.GetTileSize();

   float z = 0;
   for (uint iz = 0; iz < nVertsZ; iz++, z += terrainSettings.GetTileSize())
   {
      float x = 0;
      for (uint ix = 0; ix < nVertsX; ix++, x += terrainSettings.GetTileSize())
      {
         uint index = nVertsZ * iz + ix;

         float height = pHeightMap->GetNormalizedHeight(
            static_cast<float>(x) / extentX,
            static_cast<float>(z) / extentZ) * terrainSettings.GetHeightMapScale();

         m_vertices[index] = tVec3(x, height, z);
      }
   }

   m_terrainSettings = terrainSettings;
   NotifyListeners(&ITerrainModelListener::OnTerrainInitialize);
   return S_OK;
}

////////////////////////////////////////

tResult cTerrainModel::Clear()
{
   m_vertices.clear();
   m_quadTiles.clear();
   NotifyListeners(&ITerrainModelListener::OnTerrainClear);
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

tResult cTerrainModel::EnumTerrainQuads(IEnumTerrainQuads * * ppEnum)
{
   return EnumTerrainQuads(
      0, m_terrainSettings.GetTileCountX(),
      0, m_terrainSettings.GetTileCountZ(),
      ppEnum);
}

////////////////////////////////////////

tResult cTerrainModel::EnumTerrainQuads(uint xStart, uint xEnd, uint zStart, uint zEnd,
                                        IEnumTerrainQuads * * ppEnum)
{
   if (ppEnum == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IEnumTerrainQuads> pEnum(static_cast<IEnumTerrainQuads*>(
      new cEnumTerrainQuads(xStart, xEnd, zStart, zEnd)));

   if (!pEnum)
   {
      return E_OUTOFMEMORY;
   }

   *ppEnum = CTAddRef(pEnum);
   return S_OK;
}

////////////////////////////////////////

tResult cTerrainModel::GetVertexFromHitTest(const cRay & ray, HTERRAINVERTEX * phVertex) const
{
   if (phVertex == NULL)
   {
      return E_POINTER;
   }

   tVec3 pointOnPlane;
   if (ray.IntersectsPlane(tVec3(0,1,0), 0, &pointOnPlane))
   {
      LocalMsg3("Hit the terrain at approximately (%.1f, %.1f, %.1f)\n",
         pointOnPlane.x, pointOnPlane.y, pointOnPlane.z);

      uint ix = FloatToInt(pointOnPlane.x / m_terrainSettings.GetTileSize());
      uint iz = FloatToInt(pointOnPlane.z / m_terrainSettings.GetTileSize());

      LocalMsg2("Hit vertex (%d, %d)\n", ix, iz);
      ComposeHandle(ix, iz, phVertex);
      return S_OK;
   }

   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cTerrainModel::GetVertexPosition(HTERRAINVERTEX hVertex, tVec3 * pPosition) const
{
   if (hVertex == INVALID_HTERRAINVERTEX)
   {
      return E_INVALIDARG;
   }

   if (pPosition == NULL)
   {
      return E_POINTER;
   }

   uint16 x, z;
   DecomposeHandle(hVertex, &x, &z);

   if (x > m_terrainSettings.GetTileCountX() || z > m_terrainSettings.GetTileCountZ())
   {
      return E_FAIL;
   }

   uint index = z * (m_terrainSettings.GetTileCountZ() + 1) + x;

   *pPosition = m_vertices[index];

   return S_OK;
}

////////////////////////////////////////

tResult cTerrainModel::ChangeVertexElevation(HTERRAINVERTEX hVertex, float elevDelta)
{
   if (hVertex == INVALID_HTERRAINVERTEX)
   {
      return E_INVALIDARG;
   }

   uint16 x, z;
   DecomposeHandle(hVertex, &x, &z);

   if (x > m_terrainSettings.GetTileCountX() || z > m_terrainSettings.GetTileCountZ())
   {
      return E_FAIL;
   }

   uint index = z * (m_terrainSettings.GetTileCountZ() + 1) + x;

   m_vertices[index].y += elevDelta;

   tListeners::iterator iter = m_listeners.begin();
   tListeners::iterator end = m_listeners.end();
   for (; iter != end; iter++)
   {
      (*iter)->OnTerrainElevationChange(hVertex);
   }

   return S_OK;
}

////////////////////////////////////////

tResult cTerrainModel::SetVertexElevation(HTERRAINVERTEX hVertex, float elevation)
{
   if (hVertex == INVALID_HTERRAINVERTEX)
   {
      return E_INVALIDARG;
   }

   uint16 x, z;
   DecomposeHandle(hVertex, &x, &z);

   if (x > m_terrainSettings.GetTileCountX() || z > m_terrainSettings.GetTileCountZ())
   {
      return E_FAIL;
   }

   uint index = z * (m_terrainSettings.GetTileCountZ() + 1) + x;

   if (AlmostEqual(m_vertices[index].y, elevation))
   {
      return S_FALSE;
   }

   m_vertices[index].y = elevation;

   tListeners::iterator iter = m_listeners.begin();
   tListeners::iterator end = m_listeners.end();
   for (; iter != end; iter++)
   {
      (*iter)->OnTerrainElevationChange(hVertex);
   }

   return S_OK;
}

////////////////////////////////////////

tResult cTerrainModel::GetQuadFromHitTest(const cRay & ray, HTERRAINQUAD * phQuad) const
{
   if (phQuad == NULL)
   {
      return E_POINTER;
   }

   tVec3 pointOnPlane;
   if (ray.IntersectsPlane(tVec3(0,1,0), 0, &pointOnPlane))
   {
      LocalMsg3("Hit the terrain at approximately (%.1f, %.1f, %.1f)\n",
         pointOnPlane.x, pointOnPlane.y, pointOnPlane.z);

      uint ix, iz;
      GetTileIndices(pointOnPlane.x, pointOnPlane.z, &ix, &iz);

      LocalMsg2("Hit tile (%d, %d)\n", ix, iz);

      ComposeHandle(ix, iz, phQuad);

      return S_OK;
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cTerrainModel::SetQuadTile(HTERRAINQUAD hQuad, uint tile)
{
   uint16 x, z;
   DecomposeHandle(hQuad, &x, &z);

   if (x < m_terrainSettings.GetTileCountX() && z < m_terrainSettings.GetTileCountZ())
   {
      uint index = (z * m_terrainSettings.GetTileCountZ()) + x;
      if (index < m_quadTiles.size())
      {
         {
            tListeners::iterator iter = m_listeners.begin();
            for (; iter != m_listeners.end(); iter++)
            {
               if ((*iter)->OnTerrainTileChanging(hQuad, m_quadTiles[index], tile) == S_FALSE)
               {
                  InfoMsg4("Terrain tile change at (%d, %d) from %d to %d vetoed\n",
                     x, z, m_quadTiles[index], tile);
                  return S_FALSE;
               }
            }
         }

         m_quadTiles[index] = tile;

         {
            tListeners::iterator iter = m_listeners.begin();
            for (; iter != m_listeners.end(); iter++)
            {
               (*iter)->OnTerrainTileChanged(hQuad);
            }
         }

         return S_OK;
      }
   }
   return E_FAIL;
}

////////////////////////////////////////

tResult cTerrainModel::GetQuadTile(HTERRAINQUAD hQuad, uint * pTile) const
{
   uint16 x, z;
   DecomposeHandle(hQuad, &x, &z);

   if (pTile == NULL)
   {
      return E_POINTER;
   }

   if (x >= m_terrainSettings.GetTileCountX()
      || z >= m_terrainSettings.GetTileCountZ())
   {
      return E_INVALIDARG;
   }

   uint index = (z * m_terrainSettings.GetTileCountZ()) + x;
   Assert(index < m_quadTiles.size());

   *pTile = m_quadTiles[index];
   return S_OK;
}

////////////////////////////////////////

tResult cTerrainModel::GetQuadCorners(HTERRAINQUAD hQuad, tVec3 corners[4]) const
{
   uint16 x, z;
   DecomposeHandle(hQuad, &x, &z);
   return GetQuadCorners(x, z, corners);
}

////////////////////////////////////////

tResult cTerrainModel::GetQuadNeighbors(HTERRAINQUAD hQuad, HTERRAINQUAD neighbors[8]) const
{
   if (hQuad == INVALID_HTERRAINQUAD)
   {
      return E_INVALIDARG;
   }

   if (neighbors == NULL)
   {
      return E_POINTER;
   }

   uint16 x, z;
   DecomposeHandle(hQuad, &x, &z);

   if (x >= m_terrainSettings.GetTileCountX() || z >= m_terrainSettings.GetTileCountZ())
   {
      return E_INVALIDARG;
   }

   static const uint16 kNoIndex = (uint16)(~0u);

   // The (uint16) cast is required to eliminate a compiler warning in VC++ versions earlier than 7.1
   uint16 xPrev = x > 0 ? x - 1 : kNoIndex;
   uint16 xNext = x < (m_terrainSettings.GetTileCountX() - 1) ? x + 1 : (uint16)kNoIndex;

   uint16 zPrev = z > 0 ? z - 1 : kNoIndex;
   uint16 zNext = z < (m_terrainSettings.GetTileCountZ() - 1) ? z + 1 : (uint16)kNoIndex;

   const uint16 neighborCoords[8][2] =
   {
      {xPrev, zPrev},
      {x    , zPrev},
      {xNext, zPrev},
      {xPrev, z    },
      {xNext, z    },
      {xPrev, zNext},
      {x    , zNext},
      {xNext, zNext},
   };

   for (int i = 0; i < _countof(neighborCoords); i++)
   {
      uint16 nx = neighborCoords[i][0];
      uint16 nz = neighborCoords[i][1];

      if (nx != kNoIndex && nz != kNoIndex)
      {
         ComposeHandle(nx, nz, &neighbors[i]);
      }
      else
      {
         neighbors[i] = INVALID_HTERRAINQUAD;
      }
   }

   return S_OK;
}

////////////////////////////////////////

tResult cTerrainModel::GetPointOnTerrain(float nx, float nz, tVec3 * pLocation) const
{
   if (pLocation == NULL)
   {
      return E_POINTER;
   }

   float x = nx * m_terrainSettings.GetTileCountX() * m_terrainSettings.GetTileSize();
   float z = nz * m_terrainSettings.GetTileCountZ() * m_terrainSettings.GetTileSize();

   uint ix, iz;
   if (GetTileIndices(x, z, &ix, &iz) == S_OK)
   {
      tVec3 corners[4];
      if (GetQuadCorners(ix, iz, corners) == S_OK)
      {
         tVec3 hit;
         cRay ray(tVec3(x, 99999, z), tVec3(0, -1, 0));
         if (ray.IntersectsTriangle(corners[2], corners[1], corners[0], &hit)
            || ray.IntersectsTriangle(corners[0], corners[2], corners[3], &hit))
         {
            *pLocation = tVec3(x, hit.y, z);
            return S_OK;
         }
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

   uint halfTile = m_terrainSettings.GetTileSize() >> 1;
   *pix = FloatToInt((x - halfTile) / m_terrainSettings.GetTileSize());
   *piz = FloatToInt((z - halfTile) / m_terrainSettings.GetTileSize());
   return S_OK;
}

////////////////////////////////////////

tResult cTerrainModel::GetQuadCorners(uint quadx, uint quadz, tVec3 corners[4]) const
{
   if (corners == NULL)
   {
      return E_POINTER;
   }

   if (quadx >= m_terrainSettings.GetTileCountX()
      || quadz >= m_terrainSettings.GetTileCountZ())
   {
      return E_INVALIDARG;
   }

#define Index(qx, qz) (((qz) * (m_terrainSettings.GetTileCountZ() + 1)) + qx)
   corners[0] = m_vertices[Index(quadx,   quadz)];
   corners[1] = m_vertices[Index(quadx+1, quadz)];
   corners[2] = m_vertices[Index(quadx+1, quadz+1)];
   corners[3] = m_vertices[Index(quadx,   quadz+1)];
#undef Index

   return S_OK;
}

////////////////////////////////////////

tResult cTerrainModel::Save(IWriter * pWriter)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (pWriter->Write(m_terrainSettings) != S_OK
      || pWriter->Write(m_vertices) != S_OK
      || pWriter->Write(m_quadTiles) != S_OK)
   {
      return E_FAIL;
   }

   return S_OK;
}

////////////////////////////////////////

tResult cTerrainModel::Load(IReader * pReader, int version)
{
   if (pReader == NULL)
   {
      return E_POINTER;
   }

   if (version != g_terrainModelVer)
   {
      // Would eventually handle upgrading here
      return E_FAIL;
   }

   if (pReader->Read(&m_terrainSettings) != S_OK
      || pReader->Read(&m_vertices) != S_OK
      || pReader->Read(&m_quadTiles) != S_OK)
   {
      return E_FAIL;
   }

   NotifyListeners(&ITerrainModelListener::OnTerrainInitialize);

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
//
// CLASS: cTerrainTileSet
//

///////////////////////////////////////

cTerrainTileSet::cTerrainTileSet(const tChar * pszName)
 : m_name((pszName != NULL) ? pszName : "")
{
}

///////////////////////////////////////

cTerrainTileSet::~cTerrainTileSet()
{
}

///////////////////////////////////////

tResult cTerrainTileSet::GetName(cStr * pName) const
{
   if (pName != NULL)
   {
      *pName = m_name;
      return S_OK;
   }
   else
   {
      return E_POINTER;
   }
}

///////////////////////////////////////

tResult cTerrainTileSet::GetTileCount(uint * pTileCount) const
{
   if (pTileCount != NULL)
   {
      *pTileCount = m_tiles.size();
      return S_OK;
   }
   else
   {
      return E_POINTER;
   }
}

///////////////////////////////////////

tResult cTerrainTileSet::GetTileTexture(uint iTile, cStr * pTexture) const
{
   if (iTile >= m_tiles.size())
   {
      return E_INVALIDARG;
   }
   if (pTexture == NULL)
   {
      return E_POINTER;
   }
   *pTexture = m_tiles[iTile].texture;
   return S_OK;
}

///////////////////////////////////////

tResult cTerrainTileSet::GetTileName(uint iTile, cStr * pName) const
{
   if (iTile >= m_tiles.size())
   {
      return E_INVALIDARG;
   }
   if (pName == NULL)
   {
      return E_POINTER;
   }
   if (!m_tiles[iTile].name.empty())
   {
      *pName = m_tiles[iTile].name;
   }
   else
   {
      *pName = m_tiles[iTile].texture;
      int iDot = pName->rfind('.');
      if (iDot != cStr::npos)
      {
         pName->erase(iDot);
      }
      (*pName)[0] = toupper(pName->at(0));
   }
   return S_OK;
}

///////////////////////////////////////

tResult cTerrainTileSet::GetTileFlags(uint iTile, uint * pFlags) const
{
   if (iTile >= m_tiles.size())
   {
      return E_INVALIDARG;
   }
   if (pFlags == NULL)
   {
      return E_POINTER;
   }
   *pFlags = m_tiles[iTile].flags;
   return S_OK;
}

///////////////////////////////////////

tResult cTerrainTileSet::AddTile(const tChar * pszName, const tChar * pszTexture, uint flags)
{
   if (pszName == NULL || pszTexture == NULL)
   {
      return E_POINTER;
   }

   sTerrainTileInfo tileInfo;
   tileInfo.name = pszName;
   tileInfo.texture = pszTexture;
   tileInfo.flags = flags;

   m_tiles.push_back(tileInfo);

   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////

#define ELEMENT_TILESET    "tileset"
#define ELEMENT_TILE       "tile"
#define ATTRIB_NAME        "name"
#define ATTRIB_TEXTURE     "texture"
#define ATTRIB_FLAGS       "flags"

void * TerrainTileSetFromXml(void * pData, int dataLength, void * param)
{
   TiXmlDocument * pTiXmlDoc = reinterpret_cast<TiXmlDocument *>(pData);
   if (pTiXmlDoc != NULL)
   {
      for (TiXmlElement * pTiXmlElement = pTiXmlDoc->FirstChildElement();
         pTiXmlElement != NULL; pTiXmlElement = pTiXmlElement->NextSiblingElement())
      {
         if (pTiXmlElement->Type() == TiXmlNode::ELEMENT
            && strcmp(pTiXmlElement->Value(), ELEMENT_TILESET) == 0
            && pTiXmlElement->Attribute(ATTRIB_NAME) != NULL)
         {
            cAutoIPtr<cTerrainTileSet> pTerrainTileSet(new cTerrainTileSet(pTiXmlElement->Attribute(ATTRIB_NAME)));
            if (!pTerrainTileSet)
            {
               return NULL;
            }

            for (TiXmlElement * pChild = pTiXmlElement->FirstChildElement();
               pChild != NULL; pChild = pChild->NextSiblingElement())
            {
               if (pChild->Type() == TiXmlNode::ELEMENT
                  && strcmp(pChild->Value(), ELEMENT_TILE) == 0)
               {
                  if (pChild->Attribute(ATTRIB_NAME)
                     && pChild->Attribute(ATTRIB_TEXTURE))
                  {
                     uint flags = 0;
                     pTerrainTileSet->AddTile(pChild->Attribute(ATTRIB_NAME), pChild->Attribute(ATTRIB_TEXTURE), flags);
                  }
               }
            }

            uint tileCount = 0;
            if ((pTerrainTileSet->GetTileCount(&tileCount) == S_OK) && (tileCount > 0))
            {
               return static_cast<ITerrainTileSet*>(CTAddRef(pTerrainTileSet));
            }
         }
      }
   }

   return NULL;
}

void TerrainTileSetUnload(void * pData)
{
   ITerrainTileSet * pTerrainTileSet = reinterpret_cast<ITerrainTileSet *>(pData);
   SafeRelease(pTerrainTileSet);
}

void TerrainRegisterResourceFormats()
{
   static bool bRegistered = false;
   static uint8 nAttempts = 10;
   if (!bRegistered && nAttempts)
   {
      nAttempts -= 1;
      UseGlobal(ResourceManager);
      if (pResourceManager->RegisterFormat(kRT_TerrainTileSet, kRT_TiXml, NULL, NULL,
         TerrainTileSetFromXml, TerrainTileSetUnload) == S_OK)
      {
         bRegistered = true;
      }
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
      cHeightMap(IImage * pHeightMap)
       : m_pHeightMap(pHeightMap)
      {
         Assert(pHeightMap != NULL);
      }

      ~cHeightMap()
      {
         m_pHeightMap = NULL; // Don't delete this--it's a cached resource
      }

      virtual float GetNormalizedHeight(float nx, float nz) const
      {
         Assert(m_pHeightMap != NULL);

         // support only grayscale images for now
         if (m_pHeightMap->GetPixelFormat() != kPF_Grayscale)
         {
            return 0;
         }

         if ((nx < 0) || (nx > 1) || (nz < 0) || (nz > 1))
         {
            return 0;
         }

         uint x = FloatToInt(nx * m_pHeightMap->GetWidth());
         uint z = FloatToInt(nz * m_pHeightMap->GetHeight());

         const uint8 * pData = reinterpret_cast<const uint8 *>(m_pHeightMap->GetData());

         uint8 sample = pData[(z * m_pHeightMap->GetWidth()) + x];

         return static_cast<float>(sample) / 255.0f;
      }

   private:
      IImage * m_pHeightMap;
   };

   IImage * pHeightData = NULL;
   UseGlobal(ResourceManager);
   if (pResourceManager->Load(pszHeightData, kRT_Image, NULL, (void**)&pHeightData) != S_OK)
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
