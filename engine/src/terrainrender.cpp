/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "terrainrender.h"
#include "engineapi.h"

#include "imagedata.h"
#include "resourceapi.h"
#include "globalobj.h"
#include "filespec.h"
#include "configapi.h"
#include "readwriteapi.h"

#include <algorithm>
#include <functional>
#include <map>

#include <GL/glew.h>

#include "dbgalloc.h" // must be last header

// REFERENCES
// http://cbloom.com/3d/techdocs/splatting.txt
// http://oss.sgi.com/projects/ogl-sample/registry/ARB/texture_env_combine.txt
// http://www.gamedev.net/community/forums/topic.asp?topic_id=330331

const uint kNoIndex = ~0;

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainRenderer
//

////////////////////////////////////////

tResult TerrainRendererCreate(bool bForEditor /*=false*/)
{
   cAutoIPtr<ITerrainRenderer> p(new cTerrainRenderer(bForEditor));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_ITerrainRenderer, static_cast<ITerrainRenderer*>(p));
}

////////////////////////////////////////

cTerrainRenderer::cTerrainRenderer(bool bForEditor)
 : m_terrainModelListener(this)
 , m_nTilesPerChunk(TerrainRendererDefaults::kTerrainTilesPerChunk)
 , m_pWholeTerrainChunk(NULL)
 , m_bInEditor(bForEditor)
 , m_bEnableBlending(true)
 , m_bTerrainChanged(false)
{
}

////////////////////////////////////////

cTerrainRenderer::~cTerrainRenderer()
{
}

////////////////////////////////////////

BEGIN_CONSTRAINTS(cTerrainRenderer)
   AFTER_GUID(IID_ITerrainModel)
END_CONSTRAINTS()

////////////////////////////////////////

tResult cTerrainRenderer::Init()
{
   UseGlobal(TerrainModel);
   pTerrainModel->AddTerrainModelListener(&m_terrainModelListener);

   return S_OK;
}

////////////////////////////////////////

tResult cTerrainRenderer::Term()
{
   UseGlobal(TerrainModel);
   pTerrainModel->RemoveTerrainModelListener(&m_terrainModelListener);

   ClearChunks();

   delete m_pWholeTerrainChunk, m_pWholeTerrainChunk = NULL;

   return S_OK;
}

////////////////////////////////////////

void cTerrainRenderer::SetTilesPerChunk(uint tilesPerChunk)
{
   // TODO: some sort of validation on the input
   m_nTilesPerChunk = tilesPerChunk;
   RegenerateChunks();
}

////////////////////////////////////////

uint cTerrainRenderer::GetTilesPerChunk() const
{
   return m_nTilesPerChunk;
}

////////////////////////////////////////

tResult cTerrainRenderer::EnableBlending(bool bEnable)
{
   m_bEnableBlending = bEnable;
   if (bEnable && m_bTerrainChanged)
   {
      RegenerateChunks();
   }
   return S_OK;
}

////////////////////////////////////////

void cTerrainRenderer::RegenerateChunks()
{
   if (!m_bEnableBlending)
   {
      return;
   }

   UseGlobal(TerrainModel);

   for (tChunks::iterator iter = m_chunks.begin(); iter != m_chunks.end(); iter++)
   {
      delete *iter;
   }
   m_chunks.clear();

   cTerrainSettings terrainSettings;
   Verify(pTerrainModel->GetTerrainSettings(&terrainSettings) == S_OK);

   uint nChunksX = terrainSettings.GetTileCountX() / GetTilesPerChunk();
   uint nChunksZ = terrainSettings.GetTileCountZ() / GetTilesPerChunk();

   ITerrainTileSet * pTerrainTileSet = NULL;
   UseGlobal(ResourceManager);
   if (pResourceManager->Load(terrainSettings.GetTileSet(), kRT_TerrainTileSet, NULL, (void**)&pTerrainTileSet) != S_OK)
   {
      return;
   }

   for (uint iz = 0; iz < nChunksZ; iz++)
   {
      cRange<uint> zr(iz * GetTilesPerChunk(), (iz+1) * GetTilesPerChunk());

      for (uint ix = 0; ix < nChunksX; ix++)
      {
         cRange<uint> xr(ix * GetTilesPerChunk(), (ix+1) * GetTilesPerChunk());

         cTerrainChunk * pChunk = NULL;
         if (cTerrainChunk::Create(xr, zr, pTerrainTileSet, false, &pChunk) == S_OK)
         {
            m_chunks.push_back(pChunk);
         }
      }
   }

   m_bTerrainChanged = false;
}

////////////////////////////////////////

void cTerrainRenderer::ClearChunks()
{
   for (tChunks::iterator iter = m_chunks.begin(); iter != m_chunks.end(); iter++)
   {
      delete *iter;
   }
   m_chunks.clear();
}

////////////////////////////////////////

// VC6 requires explicit instantiation of mem_fun_t for void return type
#if _MSC_VER <= 1200
template <>
class std::mem_fun_t<void, cTerrainChunk>
{
public:
   mem_fun_t(void (cTerrainChunk::*pfn)())
      : m_pfn(pfn) {}
   void operator()(cTerrainChunk * pChunk) const
   {
      ((pChunk->*m_pfn)());
   }
private:
   void (cTerrainChunk::*m_pfn)();
};

template <>
class std::mem_fun1_t<void, cTerrainChunk, ITerrainTileSet*>
{
public:
   typedef cTerrainChunk * first_argument_type;
   typedef ITerrainTileSet * second_argument_type;
   typedef void result_type;
   mem_fun1_t(void (cTerrainChunk::*pfn)(ITerrainTileSet*))
      : m_pfn(pfn) {}
   void operator()(cTerrainChunk * pChunk, ITerrainTileSet * pTileSet) const
   {
      ((pChunk->*m_pfn)(pTileSet));
   }
private:
   void (cTerrainChunk::*m_pfn)(ITerrainTileSet*);
};

template <>
class std::binder2nd< std::mem_fun1_t<void, cTerrainChunk, ITerrainTileSet*> >
{
public:
   binder2nd(const std::mem_fun1_t<void, cTerrainChunk, ITerrainTileSet*> & op,
      ITerrainTileSet * value) : m_op(op), m_value(value)
   {
   }
   void operator()(cTerrainChunk * _X) const
   {
      (m_op(_X, m_value));
   }
private:
   std::mem_fun1_t<void, cTerrainChunk, ITerrainTileSet*> m_op;
   ITerrainTileSet * m_value;
};
#endif

////////////////////////////////////////

void cTerrainRenderer::Render()
{
   if (m_bEnableBlending)
   {
      if (!m_chunks.empty())
      {
         std::for_each(m_chunks.begin(), m_chunks.end(),
            std::mem_fun<void, cTerrainChunk>(&cTerrainChunk::Render));
      }
   }
   else
   {
      if (m_pWholeTerrainChunk != NULL)
      {
         m_pWholeTerrainChunk->Render();
      }
   }
}

////////////////////////////////////////

cTerrainRenderer::cTerrainModelListener::cTerrainModelListener(cTerrainRenderer * pOuter)
 : m_pOuter(pOuter)
{
}

////////////////////////////////////////

void cTerrainRenderer::cTerrainModelListener::OnTerrainInitialize()
{
   // HACK: should do this in a more sensible place
   glewInit();

   if (m_pOuter != NULL)
   {
      m_pOuter->m_bTerrainChanged = true;
      m_pOuter->RegenerateChunks();

      UseGlobal(TerrainModel);

      cTerrainSettings terrainSettings;
      Verify(pTerrainModel->GetTerrainSettings(&terrainSettings) == S_OK);

      ITerrainTileSet * pTerrainTileSet = NULL;
      UseGlobal(ResourceManager);
      if (pResourceManager->Load(terrainSettings.GetTileSet(), kRT_TerrainTileSet, NULL, (void**)&pTerrainTileSet) != S_OK)
      {
         return;
      }

      delete m_pOuter->m_pWholeTerrainChunk;
      m_pOuter->m_pWholeTerrainChunk = NULL;

      cTerrainChunk::Create(
         cRange<uint>(0, terrainSettings.GetTileCountX()),
         cRange<uint>(0, terrainSettings.GetTileCountZ()),
         pTerrainTileSet, true,
         &m_pOuter->m_pWholeTerrainChunk);
   }
}

////////////////////////////////////////

void cTerrainRenderer::cTerrainModelListener::OnTerrainClear()
{
   if (m_pOuter != NULL)
   {
      m_pOuter->m_bTerrainChanged = true;
      m_pOuter->ClearChunks();

      delete m_pOuter->m_pWholeTerrainChunk;
      m_pOuter->m_pWholeTerrainChunk = NULL;
   }
}

////////////////////////////////////////

void cTerrainRenderer::cTerrainModelListener::OnTerrainTileChange(HTERRAINQUAD hQuad)
{
   if (m_pOuter != NULL)
   {
      m_pOuter->m_bTerrainChanged = true;
      m_pOuter->RegenerateChunks();

      UseGlobal(TerrainModel);

      cTerrainSettings terrainSettings;
      Verify(pTerrainModel->GetTerrainSettings(&terrainSettings) == S_OK);

      if (m_pOuter->m_pWholeTerrainChunk != NULL)
      {
         ITerrainTileSet * pTerrainTileSet = NULL;
         UseGlobal(ResourceManager);
         if (pResourceManager->Load(terrainSettings.GetTileSet(), kRT_TerrainTileSet, NULL, (void**)&pTerrainTileSet) != S_OK)
         {
            return;
         }

         m_pOuter->m_pWholeTerrainChunk->BuildSplats(
            cRange<uint>(0, terrainSettings.GetTileCountX()),
            cRange<uint>(0, terrainSettings.GetTileCountZ()),
            pTerrainTileSet, true);
      }
   }
}

////////////////////////////////////////

void cTerrainRenderer::cTerrainModelListener::OnTerrainElevationChange(HTERRAINVERTEX hVertex)
{
   if (m_pOuter != NULL)
   {
      m_pOuter->m_bTerrainChanged = true;
      m_pOuter->RegenerateChunks();

      if (m_pOuter->m_pWholeTerrainChunk != NULL)
      {
         UseGlobal(TerrainModel);

         cTerrainSettings terrainSettings;
         Verify(pTerrainModel->GetTerrainSettings(&terrainSettings) == S_OK);

         m_pOuter->m_pWholeTerrainChunk->BuildVertexBuffer(
            cRange<uint>(0, terrainSettings.GetTileCountX()),
            cRange<uint>(0, terrainSettings.GetTileCountZ()),
            NULL);
      }
   }
}

/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////

static float SplatTexelWeight(const tVec2 & pt1, const tVec2 & pt2)
{
   static const float kOneOver175Sqr = 1.0f / (1.75f * 1.75f);
   return 1 - (Vec2DistanceSqr(pt1, pt2) * kOneOver175Sqr);
}

template <typename T>
inline T Clamp(T value, T rangeFirst, T rangeLast)
{
   if (value < rangeFirst)
   {
      return rangeFirst;
   }
   else if (value > rangeLast)
   {
      return rangeLast;
   }
   else
   {
      return value;
   }
}

////////////////////////////////////////

inline float STW(const tVec2 & pt1, const tVec2 & pt2)
{
   return Clamp(SplatTexelWeight(pt1,pt2), 0.f, 1.f);
}

////////////////////////////////////////
// The four means that each tile in the map is represented by a 2x2 square
// of pixels in the splat alpha map.

static const float g_splatTexelWeights[4][9] =
{
   {
      // from upper left texel (-.25, -.25)
      STW(tVec2(-.25,-.25), tVec2(-1,-1)), // top left
      STW(tVec2(-.25,-.25), tVec2( 0,-1)), // top mid
      STW(tVec2(-.25,-.25), tVec2( 1,-1)), // top right
      STW(tVec2(-.25,-.25), tVec2(-1, 0)), // left
      STW(tVec2(-.25,-.25), tVec2( 0, 0)), // 
      STW(tVec2(-.25,-.25), tVec2( 1, 0)), // right
      STW(tVec2(-.25,-.25), tVec2(-1, 1)), // bottom left
      STW(tVec2(-.25,-.25), tVec2( 0, 1)), // bottom mid
      STW(tVec2(-.25,-.25), tVec2( 1, 1)), // bottom right (could be zero)
   },
   {
      // from upper right texel (+.25, -.25)
      STW(tVec2(.25,-.25), tVec2(-1,-1)), // top left
      STW(tVec2(.25,-.25), tVec2( 0,-1)), // top mid
      STW(tVec2(.25,-.25), tVec2( 1,-1)), // top right
      STW(tVec2(.25,-.25), tVec2(-1, 0)), // left
      STW(tVec2(.25,-.25), tVec2( 0, 0)), // 
      STW(tVec2(.25,-.25), tVec2( 1, 0)), // right
      STW(tVec2(.25,-.25), tVec2(-1, 1)), // bottom left (could be zero)
      STW(tVec2(.25,-.25), tVec2( 0, 1)), // bottom mid
      STW(tVec2(.25,-.25), tVec2( 1, 1)), // bottom right
   },
   {
      // from lower left texel (-.25, +.25)
      STW(tVec2(-.25,.25), tVec2(-1,-1)), // top left
      STW(tVec2(-.25,.25), tVec2( 0,-1)), // top mid
      STW(tVec2(-.25,.25), tVec2( 1,-1)), // top right (could be zero)
      STW(tVec2(-.25,.25), tVec2(-1, 0)), // left
      STW(tVec2(-.25,.25), tVec2( 0, 0)), // 
      STW(tVec2(-.25,.25), tVec2( 1, 0)), // right
      STW(tVec2(-.25,.25), tVec2(-1, 1)), // bottom left
      STW(tVec2(-.25,.25), tVec2( 0, 1)), // bottom mid
      STW(tVec2(-.25,.25), tVec2( 1, 1)), // bottom right
   },
   {
      // from lower right texel (+.25, +.25)
      STW(tVec2(.25,.25), tVec2(-1,-1)), // top left (could be zero)
      STW(tVec2(.25,.25), tVec2( 0,-1)), // top mid
      STW(tVec2(.25,.25), tVec2( 1,-1)), // top right
      STW(tVec2(.25,.25), tVec2(-1, 0)), // left
      STW(tVec2(.25,.25), tVec2( 0, 0)), // 
      STW(tVec2(.25,.25), tVec2( 1, 0)), // right
      STW(tVec2(.25,.25), tVec2(-1, 1)), // bottom left
      STW(tVec2(.25,.25), tVec2( 0, 1)), // bottom mid
      STW(tVec2(.25,.25), tVec2( 1, 1)), // bottom right
   },
};

////////////////////////////////////////

void BuildSplatAlphaMap(uint splatTile,
                        const cRange<uint> xRange,
                        const cRange<uint> zRange,
                        uint * pAlphaMapId)
{
   UseGlobal(TerrainModel);
   UseGlobal(TerrainRenderer);

   cTerrainSettings terrainSettings;
   Verify(pTerrainModel->GetTerrainSettings(&terrainSettings) == S_OK);

   Assert(xRange.GetLength() == zRange.GetLength());

   cAutoIPtr<IEnumTerrainQuads> pEnumQuads;
   if (pTerrainModel->EnumTerrainQuads(
      xRange.GetStart(), xRange.GetEnd(),
      zRange.GetStart(), zRange.GetEnd(),
      &pEnumQuads) != S_OK)
   {
      return;
   }

   int imageWidth = pTerrainRenderer->GetTilesPerChunk() * 2;
   int imageHeight = imageWidth;
   int imageBitCount = 32;
   int imageBytes = imageWidth * imageHeight * imageBitCount / 8;

   byte * pBitmapBits = new byte[imageBytes];
   memset(pBitmapBits, 0, imageBytes);

   for (uint z = zRange.GetStart(); z < zRange.GetEnd(); z++)
   {
      uint zPrev = zRange.GetPrev(z, 0, kNoIndex);
      uint zNext = zRange.GetNext(z, terrainSettings.GetTileCountZ(), kNoIndex);

      for (uint x = xRange.GetStart(); x < xRange.GetEnd(); x++)
      {
         uint xPrev = xRange.GetPrev(x, 0, kNoIndex);
         uint xNext = xRange.GetNext(x, terrainSettings.GetTileCountX(), kNoIndex);

         HTERRAINQUAD hQuad = INVALID_HTERRAINQUAD;
         ulong nQuads = 0;
         if (pEnumQuads->Next(1, &hQuad, &nQuads) != S_OK || nQuads != 1)
         {
            continue;
         }

         float texelWeights[4];
         int texelDivisors[4];

         memset(texelWeights, 0, sizeof(texelWeights));
         memset(texelDivisors, 0, sizeof(texelDivisors));

         HTERRAINQUAD neighbors[9];
         if (pTerrainModel->GetQuadNeighbors(hQuad, neighbors) != S_OK)
         {
            continue;
         }

         memmove(&neighbors[5], &neighbors[4], 4 * sizeof(HTERRAINQUAD));
         neighbors[4] = hQuad;

         for (int i = 0; i < _countof(neighbors); i++)
         {
            uint ntile;
            if (neighbors[i] != INVALID_HTERRAINQUAD
               && pTerrainModel->GetQuadTile(neighbors[i], &ntile) == S_OK
               && ntile == splatTile)
            {
               for (int j = 0; j < _countof(texelWeights); j++)
               {
                  texelWeights[j] += g_splatTexelWeights[j][i];
                  texelDivisors[j] += 1;
               }
            }
         }

#if 0
         if (texelDivisors[0] != 0) texelWeights[0] /= texelDivisors[0];
         if (texelDivisors[1] != 0) texelWeights[1] /= texelDivisors[1];
         if (texelDivisors[2] != 0) texelWeights[2] /= texelDivisors[2];
         if (texelDivisors[3] != 0) texelWeights[3] /= texelDivisors[3];
#endif

         texelWeights[0] = Clamp(texelWeights[0], 0.f, 1.f);
         texelWeights[1] = Clamp(texelWeights[1], 0.f, 1.f);
         texelWeights[2] = Clamp(texelWeights[2], 0.f, 1.f);
         texelWeights[3] = Clamp(texelWeights[3], 0.f, 1.f);

         uint iz = (z - zRange.GetStart()) * 2;
         uint ix = (x - xRange.GetStart()) * 2;

         uint m = imageBitCount / 8;

         byte * p0 = pBitmapBits + (iz * imageWidth * m) + (ix * m);
         byte * p1 = pBitmapBits + (iz * imageWidth * m) + ((ix+1) * m);
         byte * p2 = pBitmapBits + ((iz+1) * imageWidth * m) + (ix * m);
         byte * p3 = pBitmapBits + ((iz+1) * imageWidth * m) + ((ix+1) * m);

         p0[0] = p0[1] = p0[2] = p0[3] = (byte)(255 * texelWeights[0]);
         p1[0] = p1[1] = p1[2] = p1[3] = (byte)(255 * texelWeights[1]);
         p2[0] = p2[1] = p2[2] = p2[3] = (byte)(255 * texelWeights[2]);
         p3[0] = p3[1] = p3[2] = p3[3] = (byte)(255 * texelWeights[3]);
      }
   }

   cImageData * pImage = new cImageData;
   if (pImage != NULL)
   {
      if (pImage->Create(imageWidth, imageHeight, kPF_RGBA8888, pBitmapBits))
      {
         if (ConfigIsTrue("debug_write_splat_alpha_maps"))
         {
            cStr file;
            file.Format("SplatAlpha_%d_(%d,%d)-(%d,%d).bmp", splatTile,
               xRange.GetStart(), zRange.GetStart(), xRange.GetEnd(),zRange.GetEnd());
            cAutoIPtr<IWriter> pWriter(FileCreateWriter(cFileSpec(file.c_str())));
            BmpWrite(pImage, pWriter);
         }

         GlTextureCreate(pImage, pAlphaMapId);
      }

      delete pImage;
      pImage = NULL;
   }

   delete [] pBitmapBits;
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSplatBuilder
//

////////////////////////////////////////

cSplatBuilder::cSplatBuilder(uint tile)
 : m_tile(tile)
 , m_alphaMapId(kNoIndex)
{
}

////////////////////////////////////////

cSplatBuilder::~cSplatBuilder()
{
}

////////////////////////////////////////

void cSplatBuilder::SetAlphaMap(uint alphaMapId)
{
   WarnMsgIf1(m_alphaMapId != kNoIndex, "Losing GL texture id %d\n", m_alphaMapId);
   m_alphaMapId = alphaMapId;
}

////////////////////////////////////////

tResult cSplatBuilder::GetAlphaMap(uint * pAlphaMapId) const
{
   if (pAlphaMapId == NULL)
   {
      return E_POINTER;
   }
   *pAlphaMapId = m_alphaMapId;
   return (m_alphaMapId != kNoIndex) ? S_OK : S_FALSE;
}

////////////////////////////////////////

void cSplatBuilder::AddQuad(HTERRAINQUAD hQuad)
{
   if (hQuad != INVALID_HTERRAINQUAD)
   {
      m_quads.insert(hQuad);
      m_indices.clear();
   }
}

////////////////////////////////////////

tResult cSplatBuilder::GetIndexBuffer(const tQuadVertexMap & qvm,
                                      const uint * * ppIndices,
                                      uint * pnIndices) const
{
   if (ppIndices == NULL || pnIndices == NULL)
   {
      return E_POINTER;
   }

   if (m_indices.empty())
   {
      std::set<HTERRAINQUAD>::const_iterator iter;
      for (iter = m_quads.begin(); iter != m_quads.end(); iter++)
      {
         tQuadVertexMap::const_iterator f = qvm.find(*iter);
         if (f == qvm.end())
         {
            continue;
         }
         uint iVert = f->second;
         m_indices.push_back(iVert+2);
         m_indices.push_back(iVert+1);
         m_indices.push_back(iVert);
         m_indices.push_back(iVert);
         m_indices.push_back(iVert+3);
         m_indices.push_back(iVert+2);
      }
   }

   *ppIndices = &m_indices[0];
   *pnIndices = m_indices.size();
   return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSplat
//

////////////////////////////////////////

cSplat::cSplat(const cStr & texture, uint alphaMap, const std::vector<uint> indices)
 : m_texture(texture)
 , m_alphaMap(alphaMap)
 , m_indices(indices.begin(), indices.end())
{
}

////////////////////////////////////////

cSplat::~cSplat()
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
   {
      tSplats::iterator iter = m_splats2.begin();
      for (; iter != m_splats2.end(); iter++)
      {
         delete *iter;
      }
      m_splats2.clear();
   }
}

////////////////////////////////////////

tResult cTerrainChunk::Create(const cRange<uint> xRange,
                              const cRange<uint> zRange,
                              ITerrainTileSet * pTileSet,
                              bool bNoBlending,
                              cTerrainChunk * * ppChunk)
{
   if (ppChunk == NULL)
   {
      return E_POINTER;
   }

   cTerrainChunk * pChunk = new cTerrainChunk;
   if (!pChunk)
   {
      return E_OUTOFMEMORY;
   }

   pChunk->BuildVertexBuffer(xRange, zRange, &pChunk->m_quadVertexMap);
   pChunk->BuildSplats(xRange, zRange, pTileSet, bNoBlending);

   *ppChunk = pChunk;

   return S_OK;
}

////////////////////////////////////////

tResult cTerrainChunk::BuildVertexBuffer(const cRange<uint> xRange,
                                         const cRange<uint> zRange,
                                         tQuadVertexMap * pQuadVertexMap)
{
   UseGlobal(TerrainModel);
   cTerrainSettings terrainSettings;
   Verify(pTerrainModel->GetTerrainSettings(&terrainSettings) == S_OK);

   m_vertices.resize(xRange.GetLength() * zRange.GetLength() * 4);

   cAutoIPtr<IEnumTerrainQuads> pEnumQuads;
   if (pTerrainModel->EnumTerrainQuads(
      xRange.GetStart(), xRange.GetEnd(),
      zRange.GetStart(), zRange.GetEnd(),
      &pEnumQuads) == S_OK)
   {
      UseGlobal(TerrainRenderer);
      float oneOverChunkExtentX = 1.0f / static_cast<float>(
         pTerrainRenderer->GetTilesPerChunk() * terrainSettings.GetTileSize());
      float oneOverChunkExtentZ = 1.0f / static_cast<float>(
         pTerrainRenderer->GetTilesPerChunk() * terrainSettings.GetTileSize());

      bool bFirst = true;
      tVec3 rangeStart(0,0,0);

      uint iVert = 0;

      HTERRAINQUAD hQuad = INVALID_HTERRAINQUAD;
      ulong nQuads = 0;

      while (pEnumQuads->Next(1, &hQuad, &nQuads) == S_OK && nQuads == 1)
      {
         tVec3 corners[4];
         if (pTerrainModel->GetQuadCorners(hQuad, corners) == S_OK)
         {
            if (bFirst)
            {
               rangeStart = corners[0];
               bFirst = false;
            }

            if (pQuadVertexMap != NULL)
            {
               pQuadVertexMap->insert(std::make_pair(hQuad, iVert));
            }

            m_vertices[iVert+0].uv1 = tVec2(0,0);
            m_vertices[iVert+1].uv1 = tVec2(1,0);
            m_vertices[iVert+2].uv1 = tVec2(1,1);
            m_vertices[iVert+3].uv1 = tVec2(0,1);

            for (int j = 0; j < 4; j++)
            {
               m_vertices[iVert+j].pos = corners[j];

               m_vertices[iVert+j].uv2 = tVec2(
                  (corners[j].x - rangeStart.x) * oneOverChunkExtentX,
                  (corners[j].z - rangeStart.z) * oneOverChunkExtentZ);
            }
         }

         iVert += 4;
      }

      return S_OK;
   }

   return E_FAIL;
}

////////////////////////////////////////

void cTerrainChunk::BuildSplats(const cRange<uint> xRange, const cRange<uint> zRange,
                                ITerrainTileSet * pTileSet, bool bNoBlending)
{
   UseGlobal(TerrainModel);

   cTerrainSettings terrainSettings;
   Verify(pTerrainModel->GetTerrainSettings(&terrainSettings) == S_OK);

   typedef std::map<uint, cSplatBuilder *> tSplatBuilderMap;
   tSplatBuilderMap splatBuilders;

   cAutoIPtr<IEnumTerrainQuads> pEnumQuads;
   if (pTerrainModel->EnumTerrainQuads(
      xRange.GetStart(), xRange.GetEnd(),
      zRange.GetStart(), zRange.GetEnd(),
      &pEnumQuads) == S_OK)
   {
      HTERRAINQUAD hQuad = INVALID_HTERRAINQUAD;
      ulong nQuads = 0;

      while (pEnumQuads->Next(1, &hQuad, &nQuads) == S_OK && nQuads == 1)
      {
         uint tile;
         if (pTerrainModel->GetQuadTile(hQuad, &tile) != S_OK)
         {
            continue;
         }

         cSplatBuilder * pSplatBuilder = NULL;

         if (splatBuilders.find(tile) == splatBuilders.end())
         {
            pSplatBuilder = new cSplatBuilder(tile);
            if (pSplatBuilder != NULL)
            {
               splatBuilders[tile] = pSplatBuilder;
            }
         }
         else
         {
            pSplatBuilder = splatBuilders[tile];
         }

         if (pSplatBuilder != NULL)
         {
            pSplatBuilder->AddQuad(hQuad);

            if (!bNoBlending)
            {
               HTERRAINQUAD neighbors[8];
               if (pTerrainModel->GetQuadNeighbors(hQuad, neighbors) == S_OK)
               {
                  for (int i = 0; i < _countof(neighbors); i++)
                  {
                     // Add bordering quads too (these provide the fade)
                     uint ntile;
                     if (neighbors[i] != INVALID_HTERRAINQUAD
                        && pTerrainModel->GetQuadTile(neighbors[i], &ntile) == S_OK
                        && ntile != tile)
                     {
                        pSplatBuilder->AddQuad(neighbors[i]);
                     }
                  }
               }
#ifdef _DEBUG
               else
               {
                  ErrorMsg("GetQuadNeighbors failed\n");
               }
#endif
            }
         }
      }
   }

   // Clear any existing splats
   {
      tSplats::iterator iter = m_splats2.begin();
      for (; iter != m_splats2.end(); iter++)
      {
         delete *iter;
      }
      m_splats2.clear();
   }

   // Store the new splats
   {
      Assert(m_splats2.empty());

      tSplatBuilderMap::iterator iter = splatBuilders.begin();
      for (; iter != splatBuilders.end(); iter++)
      {
         cSplatBuilder * pSplatBuilder = iter->second;

         uint alphaMapId = kNoIndex;
         if (!bNoBlending)
         {
            BuildSplatAlphaMap(pSplatBuilder->GetTile(), xRange, zRange, &alphaMapId);
            pSplatBuilder->SetAlphaMap(alphaMapId);
         }

         const uint * pIndices;
         uint nIndices;
         pSplatBuilder->GetIndexBuffer(m_quadVertexMap, &pIndices, &nIndices);

         cStr tileTexture;
         if (pTileSet->GetTileTexture(pSplatBuilder->GetTile(), &tileTexture) == S_OK)
         {
            cSplat * pSplat = new cSplat(tileTexture, alphaMapId, pSplatBuilder->GetIndices());
            if (pSplat != NULL)
            {
               m_splats2.push_back(pSplat);
            }
         }

         delete pSplatBuilder;
      }

      splatBuilders.clear();
   }
}

////////////////////////////////////////

void cTerrainChunk::Render()
{
   glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
   glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

   glDisableClientState(GL_EDGE_FLAG_ARRAY);
   glDisableClientState(GL_INDEX_ARRAY);
   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_NORMAL_ARRAY);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glDisableClientState(GL_COLOR_ARRAY);

   const byte * pVertexData = (const byte *)(sTerrainVertex *)&m_vertices[0];

   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, sizeof(sTerrainVertex), pVertexData + offsetof(sTerrainVertex, pos));

   glColor4f(1,1,1,1);

   UseGlobal(ResourceManager);

   tSplats::iterator iter = m_splats2.begin();
   for (; iter != m_splats2.end(); iter++)
   {
      GLuint alphaMap = (*iter)->GetAlphaMap();
      if (alphaMap != kNoIndex)
      {
         glActiveTextureARB(GL_TEXTURE0);
         glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
         glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE0);
         glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
         glBindTexture(GL_TEXTURE_2D, alphaMap);
         glEnable(GL_TEXTURE_2D);

         glClientActiveTextureARB(GL_TEXTURE0);
         glTexCoordPointer(2, GL_FLOAT, sizeof(sTerrainVertex), pVertexData + offsetof(sTerrainVertex, uv2));
         glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      }

      GLuint texId;
      if (pResourceManager->Load((*iter)->GetTexture().c_str(), kRT_GlTexture, NULL, (void**)&texId) == S_OK)
      {
         glActiveTextureARB(GL_TEXTURE1);
         glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
         glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
         glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PRIMARY_COLOR);
         glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
         glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE1);
         glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
         glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE0);
         glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
         glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, GL_TEXTURE0);
         glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB, GL_SRC_ALPHA);
         glBindTexture(GL_TEXTURE_2D, texId);
         glEnable(GL_TEXTURE_2D);

         glClientActiveTextureARB(GL_TEXTURE1);
         glTexCoordPointer(2, GL_FLOAT, sizeof(sTerrainVertex), pVertexData + offsetof(sTerrainVertex, uv1));
         glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      }

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      const uint * pIndices = NULL;
      uint nIndices = 0;

      glDrawElements(GL_TRIANGLES, (*iter)->GetIndexCount(), GL_UNSIGNED_INT, (*iter)->GetIndexPtr());
   }

   glPopClientAttrib();
   glPopAttrib();
}

/////////////////////////////////////////////////////////////////////////////
