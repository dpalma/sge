/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "geomipmap.h"

#include "engine/engineapi.h"

#include "tech/connptimpl.h"
#include "tech/imageapi.h"
#include "tech/ray.h"
#include "tech/readwriteapi.h"
#include "tech/readwriteutils.h"
#include "tech/resourceapi.h"

#include <algorithm>
#include <map>
#include <tinyxml.h>

#include "tech/dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(GMMTerrain);
#define LocalMsg(msg)            DebugMsgEx(GMMTerrain,(msg))
#define LocalMsg1(msg,a)         DebugMsgEx1(GMMTerrain,(msg),(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(GMMTerrain,(msg),(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(GMMTerrain,(msg),(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)   DebugMsgEx4(GMMTerrain,(msg),(a),(b),(c),(d))

/////////////////////////////////////////////////////////////////////////////

F_DECLARE_GUID(SAVELOADID_GMMTerrain);

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
// CLASS: cGMMTerrain
//

////////////////////////////////////////

tResult GMMTerrainCreate()
{
   cAutoIPtr<cGMMTerrain> p(new cGMMTerrain);
   if (!p)
   {
      return E_OUTOFMEMORY;
   }

   if (RegisterGlobalObject(IID_ITerrainModel, static_cast<ITerrainModel*>(p)) == S_OK
      && RegisterGlobalObject(IID_ITerrainRenderer, static_cast<ITerrainRenderer*>(p)) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}

////////////////////////////////////////

BEGIN_CONSTRAINTS(cGMMTerrain)
   AFTER_GUID(IID_ISaveLoadManager)
END_CONSTRAINTS()

////////////////////////////////////////

cGMMTerrain::cGMMTerrain()
{
}

////////////////////////////////////////

cGMMTerrain::~cGMMTerrain()
{
}

////////////////////////////////////////
// Since cGMMTerrain implements both ITerrainModel and ITerrainRenderer,
// Init and Term will be called twice each.

tResult cGMMTerrain::Init()
{
   UseGlobal(SaveLoadManager);
   pSaveLoadManager->RegisterSaveLoadParticipant(SAVELOADID_GMMTerrain,
      1, static_cast<ISaveLoadParticipant*>(this));
   return S_OK;
}

////////////////////////////////////////

tResult cGMMTerrain::Term()
{
   UseGlobal(SaveLoadManager);
   pSaveLoadManager->RevokeSaveLoadParticipant(SAVELOADID_GMMTerrain, 1);
   return S_OK;
}

////////////////////////////////////////

tResult cGMMTerrain::Initialize(const cTerrainSettings & terrainSettings)
{
   if (_tcslen(terrainSettings.GetTileSet()) == 0)
   {
      return E_FAIL;
   }

   if (terrainSettings.GetHeightData() == kTHD_HeightMap)
   {
      IImage * pHeightMap = NULL;
      UseGlobal(ResourceManager);
      if (pResourceManager->Load(terrainSettings.GetHeightMap(), kRT_Image, NULL, (void**)&pHeightMap) != S_OK)
      {
         return E_FAIL;
      }

      uint w = pHeightMap->GetWidth(), h = pHeightMap->GetHeight();

      if (!IsPowerOfTwo(w - 1) || !IsPowerOfTwo(h - 1))
      {
         ErrorMsg2("Invalid height map dimensions: %d x %d\n", w, h);
         return E_FAIL;
      }

      m_vertices.resize(w * h);

      float z = 0;
      for (uint iz = 0; iz < h; ++iz, z += terrainSettings.GetTileSize())
      {
         float x = 0;
         for (uint ix = 0; ix < w; ++ix, x += terrainSettings.GetTileSize())
         {
            uint index = h * iz + ix;

            byte heightSample[4];
            Verify(pHeightMap->GetPixel(ix, iz, heightSample) == S_OK);

            float normalizedHeight = static_cast<float>(heightSample[0]) / 255.0f;

            m_vertices[index] = tVec3(x, normalizedHeight * terrainSettings.GetHeightMapScale(), z);
         }
      }
   }
   else if (terrainSettings.GetHeightData() == kTHD_Fixed)
   {
      uint w = terrainSettings.GetTileCountX() + 1, h = terrainSettings.GetTileCountZ() + 1;

      m_vertices.resize(w * h);

      float z = 0;
      for (uint iz = 0; iz < h; ++iz, z += terrainSettings.GetTileSize())
      {
         float x = 0;
         for (uint ix = 0; ix < w; ++ix, x += terrainSettings.GetTileSize())
         {
            uint index = h * iz + ix;
            m_vertices[index] = tVec3(x, 0, z);
         }
      }
   }

   m_terrainSettings = terrainSettings;

   return S_OK;
}

////////////////////////////////////////

tResult cGMMTerrain::Clear()
{
   m_vertices.clear();
   return S_OK;
}

////////////////////////////////////////

tResult cGMMTerrain::GetTerrainSettings(cTerrainSettings * pTerrainSettings) const
{
   if (pTerrainSettings == NULL)
   {
      return E_POINTER;
   }
   *pTerrainSettings = m_terrainSettings;
   return S_OK;
}

////////////////////////////////////////

tResult cGMMTerrain::AddTerrainModelListener(ITerrainModelListener * pListener)
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cGMMTerrain::RemoveTerrainModelListener(ITerrainModelListener * pListener)
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cGMMTerrain::EnumTerrainQuads(IEnumTerrainQuads * * ppEnum)
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cGMMTerrain::EnumTerrainQuads(uint xStart, uint xEnd, uint zStart, uint zEnd,
                                      IEnumTerrainQuads * * ppEnum)
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cGMMTerrain::GetVertexFromHitTest(const cRay & ray, HTERRAINVERTEX * phVertex) const
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

tResult cGMMTerrain::GetVertexPosition(HTERRAINVERTEX hVertex, tVec3 * pPosition) const
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

tResult cGMMTerrain::ChangeVertexElevation(HTERRAINVERTEX hVertex, float elevDelta)
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

   return S_OK;
}

////////////////////////////////////////

tResult cGMMTerrain::SetVertexElevation(HTERRAINVERTEX hVertex, float elevation)
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

   return S_OK;
}

////////////////////////////////////////

tResult cGMMTerrain::GetQuadFromHitTest(const cRay & ray, HTERRAINQUAD * phQuad) const
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

tResult cGMMTerrain::SetQuadTile(HTERRAINQUAD hQuad, uint tile)
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cGMMTerrain::GetQuadTile(HTERRAINQUAD hQuad, uint * pTile) const
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cGMMTerrain::GetQuadCorners(HTERRAINQUAD hQuad, tVec3 corners[4]) const
{
   uint16 x, z;
   DecomposeHandle(hQuad, &x, &z);
   return GetQuadCorners(x, z, corners);
}

////////////////////////////////////////

tResult cGMMTerrain::GetQuadNeighbors(HTERRAINQUAD hQuad, HTERRAINQUAD neighbors[8]) const
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

tResult cGMMTerrain::GetPointOnTerrain(float nx, float nz, tVec3 * pLocation) const
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

void cGMMTerrain::SetTilesPerChunk(uint tilesPerChunk)
{
}

////////////////////////////////////////

uint cGMMTerrain::GetTilesPerChunk() const
{
   return 0;
}

////////////////////////////////////////

tResult cGMMTerrain::EnableBlending(bool bEnable)
{
   return E_NOTIMPL;
}

////////////////////////////////////////

void cGMMTerrain::Render()
{
}

////////////////////////////////////////

tResult cGMMTerrain::HighlightTerrainQuad(HTERRAINQUAD hQuad)
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cGMMTerrain::HighlightTerrainVertex(HTERRAINVERTEX hVertex)
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cGMMTerrain::ClearHighlight()
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cGMMTerrain::GetTileIndices(float x, float z, uint * pix, uint * piz) const
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

tResult cGMMTerrain::GetQuadCorners(uint quadx, uint quadz, tVec3 corners[4]) const
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

tResult cGMMTerrain::Save(IWriter * pWriter)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (pWriter->Write(m_terrainSettings) != S_OK
      || pWriter->Write(m_vertices) != S_OK)
   {
      return E_FAIL;
   }

   return S_OK;
}

////////////////////////////////////////

tResult cGMMTerrain::Load(IReader * pReader, int version)
{
   if (pReader == NULL)
   {
      return E_POINTER;
   }

   if (version != 1)
   {
      // Would eventually handle upgrading here
      return E_FAIL;
   }

   if (pReader->Read(&m_terrainSettings) != S_OK
      || pReader->Read(&m_vertices) != S_OK)
   {
      return E_FAIL;
   }

   return S_OK;
}

////////////////////////////////////////

void cGMMTerrain::Reset()
{
   Clear();
}


/////////////////////////////////////////////////////////////////////////////
