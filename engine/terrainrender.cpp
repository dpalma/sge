/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "terrainrender.h"

#include "tech/color.h"
#include "tech/imageapi.h"
#include "tech/resourceapi.h"
#include "tech/globalobj.h"
#include "tech/filespec.h"
#include "tech/configapi.h"
#include "tech/point2.inl"
#include "tech/readwriteapi.h"
#include "tech/vec4.h"

#include <algorithm>
#include <functional>
#include <map>

#include <GL/glew.h>

#include "tech/dbgalloc.h" // must be last header

// REFERENCES
// http://cbloom.com/3d/techdocs/splatting.txt
// http://oss.sgi.com/projects/ogl-sample/registry/ARB/texture_env_combine.txt
// http://www.gamedev.net/community/forums/topic.asp?topic_id=330331

#pragma warning(disable:4355) // 'this' : used in base member initializer list

const uint kNoIndex = ~0u;

/////////////////////////////////////////////////////////////////////////////

const sVertexElement g_terrainVert[] =
{
   { kVEU_TexCoord,  kVET_Float2,   1, 0 },
   { kVEU_TexCoord,  kVET_Float2,   0, 2 * sizeof(float) },
   { kVEU_Position,  kVET_Float3,   0, 4 * sizeof(float) },
};

////////////////////////////////////////

static tResult TerrainRendererCreate(bool bForEditor)
{
   cAutoIPtr<ITerrainRenderer> p(new cTerrainRenderer(bForEditor));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_ITerrainRenderer, static_cast<ITerrainRenderer*>(p));
}

////////////////////////////////////////

tResult TerrainRendererCreate()
{
   return TerrainRendererCreate(false);
}

////////////////////////////////////////

tResult TerrainRendererCreateForEditor()
{
   return TerrainRendererCreate(true);
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainRenderer
//

////////////////////////////////////////

cTerrainRenderer::cTerrainRenderer(bool bForEditor)
 : m_terrainModelListener(this)
 , m_nTilesPerChunk(TerrainRendererDefaults::kTerrainTilesPerChunk)
 , m_pWholeTerrainChunk(NULL)
 , m_bInEditor(bForEditor)
 , m_bEnableBlending(true)
 , m_bTerrainChanged(false)
 , m_baseTile(kNoIndex)
 , m_highlightQuad(INVALID_HTERRAINQUAD)
 , m_highlightVertex(INVALID_HTERRAINVERTEX)
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

F_DECLARE_GUID(SAVELOADID_TerrainModel);

// {D133F917-602C-4d99-A29A-2DE963AF21D8}
static const GUID SAVELOADID_TerrainRenderer = 
{ 0xd133f917, 0x602c, 0x4d99, { 0xa2, 0x9a, 0x2d, 0xe9, 0x63, 0xaf, 0x21, 0xd8 } };

static const int g_terrainRendererVer = 1;

static const cBeforeAfterConstraint g_saveConstraints[] =
{
   cBeforeAfterConstraint(&SAVELOADID_TerrainModel, kBefore)
};

////////////////////////////////////////

tResult cTerrainRenderer::Init()
{
   UseGlobal(TerrainModel);
   pTerrainModel->AddTerrainModelListener(&m_terrainModelListener);

   UseGlobal(SaveLoadManager);
   pSaveLoadManager->RegisterSaveLoadParticipant(SAVELOADID_TerrainRenderer,
      g_saveConstraints, _countof(g_saveConstraints), g_terrainRendererVer,
      static_cast<ISaveLoadParticipant*>(this));

   return S_OK;
}

////////////////////////////////////////

tResult cTerrainRenderer::Term()
{
   UseGlobal(SaveLoadManager);
   pSaveLoadManager->RevokeSaveLoadParticipant(SAVELOADID_TerrainRenderer, g_terrainRendererVer);

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
      ErrorMsg1("Unable to load terrain tile set %s\n", terrainSettings.GetTileSet());
      return;
   }

   m_baseTile = kNoIndex;
   {
      uint maxTileCount = 0;
      tTileCountMap::const_iterator iter = m_tileCountMap.begin();
      for (; iter != m_tileCountMap.end(); iter++)
      {
         if (iter->second > maxTileCount)
         {
            m_baseTile = iter->first;
            maxTileCount = iter->second;
         }
      }
   }

   for (uint iz = 0; iz < nChunksZ; iz++)
   {
      cRange<uint> zr(iz * GetTilesPerChunk(), (iz+1) * GetTilesPerChunk());

      for (uint ix = 0; ix < nChunksX; ix++)
      {
         cRange<uint> xr(ix * GetTilesPerChunk(), (ix+1) * GetTilesPerChunk());

         cTerrainChunk * pChunk = NULL;
         if (cTerrainChunk::Create(xr, zr, pTerrainTileSet, m_baseTile, false, &pChunk) == S_OK)
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
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
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

//extern "C" __declspec(dllimport) short __stdcall GetAsyncKeyState(int);

void cTerrainRenderer::Render()
{
   static bool glewInitCalled = false;
   if (!glewInitCalled)
   {
      if (glewInit() != GLEW_OK)
      {
         ErrorMsg("GLEW failed to initialize\n");
      }
      glewInitCalled = true;
   }

   if (m_bEnableBlending)
   {
      if (!m_chunks.empty())
      {
//#if 0
//         tChunks::iterator iter = m_chunks.begin();
//         for (int index = 0; iter != m_chunks.end(); iter++, index++)
//         {
//            if (GetAsyncKeyState('1' + index) < 0)
//            {
//               continue;
//            }
//            (*iter)->Render();
//         }
//#else
         std::for_each(m_chunks.begin(), m_chunks.end(),
            std::mem_fun<void, cTerrainChunk>(&cTerrainChunk::Render));
//#endif
      }
   }
   else
   {
      if (m_pWholeTerrainChunk != NULL)
      {
         m_pWholeTerrainChunk->Render();
      }
   }

   if (RunningInEditor())
   {
      static const GLfloat kHighlightTileColor[] = { 0, 1, 0, 0.25f }; // semi-transparent green
      static const GLfloat kHighlightVertexColor[] = { 0, 0, 1, 0.25f }; // semi-transparent blue

      if (m_highlightQuad != INVALID_HTERRAINQUAD)
      {
         UseGlobal(TerrainModel);
         tVec3 corners[4];
         if (pTerrainModel->GetQuadCorners(m_highlightQuad, corners) == S_OK)
         {
            glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
            glEnable(GL_POLYGON_OFFSET_FILL);
            // HACK: the polygon offset value here has to be big enough to clear the
            // offsets used by the terrain splats
            glPolygonOffset(-6, -6);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBegin(GL_QUADS);
               glColor4fv(kHighlightTileColor);
               glNormal3f(0, 1, 0);
               glVertex3fv(corners[0].v);
               glVertex3fv(corners[3].v);
               glVertex3fv(corners[2].v);
               glVertex3fv(corners[1].v);
            glEnd();
            glPopAttrib();
         }
      }

      if (m_highlightVertex != INVALID_HTERRAINVERTEX)
      {
         UseGlobal(TerrainModel);
         tVec3 vertex;
         if (pTerrainModel->GetVertexPosition(m_highlightVertex, &vertex) == S_OK)
         {
            static const tVec3 offsets[4] =
            {
               tVec3( -5, 0,  5 ),
               tVec3(  5, 0,  5 ),
               tVec3(  5, 0, -5 ),
               tVec3( -5, 0, -5 ),
            };

            tVec3 corners[4] =
            {
               vertex + offsets[0],
               vertex + offsets[1],
               vertex + offsets[2],
               vertex + offsets[3],
            };

            glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
            glEnable(GL_POLYGON_OFFSET_FILL);
            // HACK: the polygon offset value here has to be big enough to clear the
            // offsets used by the terrain splats
            glPolygonOffset(-6, -6);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBegin(GL_QUADS);
               glColor4fv(kHighlightVertexColor);
               glNormal3f(0, 1, 0);
               glVertex3fv(corners[0].v);
               glVertex3fv(corners[3].v);
               glVertex3fv(corners[2].v);
               glVertex3fv(corners[1].v);
            glEnd();
            glPopAttrib();
         }
      }
   }
}

////////////////////////////////////////

tResult cTerrainRenderer::HighlightTerrainQuad(HTERRAINQUAD hQuad)
{
   m_highlightQuad = hQuad;
   m_highlightVertex = INVALID_HTERRAINVERTEX;
   return S_OK;
}

////////////////////////////////////////

tResult cTerrainRenderer::HighlightTerrainVertex(HTERRAINVERTEX hVertex)
{
   m_highlightQuad = INVALID_HTERRAINQUAD;
   m_highlightVertex = hVertex;
   return S_OK;
}

////////////////////////////////////////

tResult cTerrainRenderer::ClearHighlight()
{
   m_highlightQuad = INVALID_HTERRAINQUAD;
   m_highlightVertex = INVALID_HTERRAINVERTEX;
   return S_OK;
}

////////////////////////////////////////

tResult cTerrainRenderer::Save(IWriter * pWriter)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   return S_FALSE;
}

////////////////////////////////////////

tResult cTerrainRenderer::Load(IReader * pReader, int version)
{
   if (pReader == NULL)
   {
      return E_POINTER;
   }

   // Will eventually handle upgrading here
   if (g_terrainRendererVer != version)
   {
      return S_FALSE;
   }

   // TODO

   return S_FALSE;
}

////////////////////////////////////////

void cTerrainRenderer::Reset()
{
}

////////////////////////////////////////

cTerrainRenderer::cTerrainModelListener::cTerrainModelListener(cTerrainRenderer * pOuter)
 : m_pOuter(pOuter)
{
}

////////////////////////////////////////

void cTerrainRenderer::cTerrainModelListener::OnTerrainInitialize()
{
   if (m_pOuter != NULL)
   {
      UseGlobal(TerrainModel);

      cTerrainSettings terrainSettings;
      Verify(pTerrainModel->GetTerrainSettings(&terrainSettings) == S_OK);

      cAutoIPtr<IEnumTerrainQuads> pEnumQuads;
      if (pTerrainModel->EnumTerrainQuads(
         0, terrainSettings.GetTileCountX(),
         0, terrainSettings.GetTileCountZ(),
         &pEnumQuads) == S_OK)
      {
         HTERRAINQUAD hQuads[16];
         ulong nQuads = 0;

         while (pEnumQuads->Next(_countof(hQuads), hQuads, &nQuads) == S_OK)
         {
            for (ulong i = 0; i < nQuads; i++)
            {
               uint tile;
               if (pTerrainModel->GetQuadTile(hQuads[i], &tile) == S_OK)
               {
                  if (m_pOuter->m_tileCountMap.find(tile) == m_pOuter->m_tileCountMap.end())
                  {
                     m_pOuter->m_tileCountMap[tile] = 1;
                  }
                  else
                  {
                     m_pOuter->m_tileCountMap[tile] += 1;
                  }
               }
               else
               {
                  WarnMsg("ITerrainModel::GetQuadTile failed\n");
               }
            }
         }
      }

      m_pOuter->m_bTerrainChanged = true;
      m_pOuter->RegenerateChunks();

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
         pTerrainTileSet, m_pOuter->m_baseTile, true,
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

tResult cTerrainRenderer::cTerrainModelListener::OnTerrainTileChanging(HTERRAINQUAD hQuad,
                                                                       uint oldTile, uint newTile)
{
   if (m_pOuter->m_tileCountMap.find(newTile) == m_pOuter->m_tileCountMap.end())
   {
      m_pOuter->m_tileCountMap[newTile] = 1;
   }
   else
   {
      m_pOuter->m_tileCountMap[newTile] += 1;
   }

   Assert(m_pOuter->m_tileCountMap.find(oldTile) != m_pOuter->m_tileCountMap.end());
   m_pOuter->m_tileCountMap[oldTile] -= 1;

   return S_OK;
}

////////////////////////////////////////

void cTerrainRenderer::cTerrainModelListener::OnTerrainTileChanged(HTERRAINQUAD hQuad)
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
            pTerrainTileSet, m_pOuter->m_baseTile, true);
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

static float SplatTexelWeight(const cPoint2<float> & pt1, const cPoint2<float> & pt2)
{
   static const float kOneOver175Sqr = 1.0f / (1.75f * 1.75f);
   return 1 - (DistanceSqr(pt1, pt2) * kOneOver175Sqr);
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
// The four means that each tile in the map is represented by a 2x2 square
// of pixels in the splat alpha map.

#define STW(pt1, pt2) Clamp(SplatTexelWeight((pt1),(pt2)), 0.f, 9999.f)

static const float g_splatTexelWeights[4][9] =
{
   {
      // from upper left texel (-.25, -.25)
      STW(cPoint2<float>(-.25,-.25), cPoint2<float>(-1,-1)), // top left
      STW(cPoint2<float>(-.25,-.25), cPoint2<float>( 0,-1)), // top mid
      STW(cPoint2<float>(-.25,-.25), cPoint2<float>( 1,-1)), // top right
      STW(cPoint2<float>(-.25,-.25), cPoint2<float>(-1, 0)), // left
      STW(cPoint2<float>(-.25,-.25), cPoint2<float>( 0, 0)), // 
      STW(cPoint2<float>(-.25,-.25), cPoint2<float>( 1, 0)), // right
      STW(cPoint2<float>(-.25,-.25), cPoint2<float>(-1, 1)), // bottom left
      STW(cPoint2<float>(-.25,-.25), cPoint2<float>( 0, 1)), // bottom mid
      STW(cPoint2<float>(-.25,-.25), cPoint2<float>( 1, 1)), // bottom right (could be zero)
   },
   {
      // from upper right texel (+.25, -.25)
      STW(cPoint2<float>(.25,-.25), cPoint2<float>(-1,-1)), // top left
      STW(cPoint2<float>(.25,-.25), cPoint2<float>( 0,-1)), // top mid
      STW(cPoint2<float>(.25,-.25), cPoint2<float>( 1,-1)), // top right
      STW(cPoint2<float>(.25,-.25), cPoint2<float>(-1, 0)), // left
      STW(cPoint2<float>(.25,-.25), cPoint2<float>( 0, 0)), // 
      STW(cPoint2<float>(.25,-.25), cPoint2<float>( 1, 0)), // right
      STW(cPoint2<float>(.25,-.25), cPoint2<float>(-1, 1)), // bottom left (could be zero)
      STW(cPoint2<float>(.25,-.25), cPoint2<float>( 0, 1)), // bottom mid
      STW(cPoint2<float>(.25,-.25), cPoint2<float>( 1, 1)), // bottom right
   },
   {
      // from lower left texel (-.25, +.25)
      STW(cPoint2<float>(-.25,.25), cPoint2<float>(-1,-1)), // top left
      STW(cPoint2<float>(-.25,.25), cPoint2<float>( 0,-1)), // top mid
      STW(cPoint2<float>(-.25,.25), cPoint2<float>( 1,-1)), // top right (could be zero)
      STW(cPoint2<float>(-.25,.25), cPoint2<float>(-1, 0)), // left
      STW(cPoint2<float>(-.25,.25), cPoint2<float>( 0, 0)), // 
      STW(cPoint2<float>(-.25,.25), cPoint2<float>( 1, 0)), // right
      STW(cPoint2<float>(-.25,.25), cPoint2<float>(-1, 1)), // bottom left
      STW(cPoint2<float>(-.25,.25), cPoint2<float>( 0, 1)), // bottom mid
      STW(cPoint2<float>(-.25,.25), cPoint2<float>( 1, 1)), // bottom right
   },
   {
      // from lower right texel (+.25, +.25)
      STW(cPoint2<float>(.25,.25), cPoint2<float>(-1,-1)), // top left (could be zero)
      STW(cPoint2<float>(.25,.25), cPoint2<float>( 0,-1)), // top mid
      STW(cPoint2<float>(.25,.25), cPoint2<float>( 1,-1)), // top right
      STW(cPoint2<float>(.25,.25), cPoint2<float>(-1, 0)), // left
      STW(cPoint2<float>(.25,.25), cPoint2<float>( 0, 0)), // 
      STW(cPoint2<float>(.25,.25), cPoint2<float>( 1, 0)), // right
      STW(cPoint2<float>(.25,.25), cPoint2<float>(-1, 1)), // bottom left
      STW(cPoint2<float>(.25,.25), cPoint2<float>( 0, 1)), // bottom mid
      STW(cPoint2<float>(.25,.25), cPoint2<float>( 1, 1)), // bottom right
   },
};

#undef STW

////////////////////////////////////////

tResult BuildSplatAlphaMap(uint splatTile,
                           const cRange<uint> xRange,
                           const cRange<uint> zRange,
                           IImage * * ppAlphaMapImage)
{
   if (ppAlphaMapImage == NULL)
   {
      return E_POINTER;
   }

   UseGlobal(TerrainModel);
   UseGlobal(TerrainRenderer);

   cTerrainSettings terrainSettings;
   if (pTerrainModel->GetTerrainSettings(&terrainSettings) != S_OK)
   {
      return E_FAIL;
   }

   Assert(xRange.GetLength() == zRange.GetLength());

   cAutoIPtr<IEnumTerrainQuads> pEnumQuads;
   if (pTerrainModel->EnumTerrainQuads(
      xRange.GetStart(), xRange.GetEnd(),
      zRange.GetStart(), zRange.GetEnd(),
      &pEnumQuads) != S_OK)
   {
      return E_FAIL;
   }

   int imageSize = pTerrainRenderer->GetTilesPerChunk() * 2;

   cAutoIPtr<IImage> pImage;
   if (ImageCreate(imageSize, imageSize, kPF_RGBA8888, NULL, &pImage) != S_OK)
   {
      return E_FAIL;
   }

   for (uint z = zRange.GetStart(); z < zRange.GetEnd(); z++)
   {
      for (uint x = xRange.GetStart(); x < xRange.GetEnd(); x++)
      {
         HTERRAINQUAD hQuad = INVALID_HTERRAINQUAD;
         ulong nQuads = 0;
         if (pEnumQuads->Next(1, &hQuad, &nQuads) != S_OK || nQuads != 1)
         {
            continue;
         }

         HTERRAINQUAD neighbors[9];
         if (pTerrainModel->GetQuadNeighbors(hQuad, neighbors) != S_OK)
         {
            continue;
         }

         memmove(&neighbors[5], &neighbors[4], 4 * sizeof(HTERRAINQUAD));
         neighbors[4] = hQuad;

         std::map<uint, tVec4> texelWeightMap;
         texelWeightMap[splatTile] = tVec4(0,0,0,0);

         tVec4 texelWeightTotals(0,0,0,0);

         for (int i = 0; i < _countof(neighbors); i++)
         {
            if (neighbors[i] == INVALID_HTERRAINQUAD)
            {
               continue;
            }

            uint neighborTile;
            if (pTerrainModel->GetQuadTile(neighbors[i], &neighborTile) != S_OK)
            {
               continue;
            }

            if (texelWeightMap.find(neighborTile) == texelWeightMap.end())
            {
               texelWeightMap[neighborTile] = tVec4(0,0,0,0);
            }

            tVec4 neighborWeights(
               g_splatTexelWeights[0][i],
               g_splatTexelWeights[1][i],
               g_splatTexelWeights[2][i],
               g_splatTexelWeights[3][i]);
            texelWeightMap[neighborTile] += neighborWeights;
            texelWeightTotals += neighborWeights;
         }

         tVec4 & texelWeights = texelWeightMap[splatTile];
         texelWeights.x /= texelWeightTotals.x;
         texelWeights.y /= texelWeightTotals.y;
         texelWeights.z /= texelWeightTotals.z;
         texelWeights.w /= texelWeightTotals.w;

         uint iz = (z - zRange.GetStart()) * 2;
         uint ix = (x - xRange.GetStart()) * 2;

         byte twx = static_cast<byte>(texelWeights.x * 255);
         byte twy = static_cast<byte>(texelWeights.y * 255);
         byte twz = static_cast<byte>(texelWeights.z * 255);
         byte tww = static_cast<byte>(texelWeights.w * 255);

         byte temp[4];

         temp[0] = temp[1] = temp[2] = temp[3] = twx;
         pImage->SetPixel(ix, iz, temp);
         temp[0] = temp[1] = temp[2] = temp[3] = twy;
         pImage->SetPixel(ix+1, iz, temp);
         temp[0] = temp[1] = temp[2] = temp[3] = twz;
         pImage->SetPixel(ix, iz+1, temp);
         temp[0] = temp[1] = temp[2] = temp[3] = tww;
         pImage->SetPixel(ix+1, iz+1, temp);
      }
   }

   *ppAlphaMapImage = CTAddRef(pImage);
   return S_OK;
}

tResult BuildSplatAlphaMap(uint splatTile,
                           const cRange<uint> xRange,
                           const cRange<uint> zRange,
                           uint * pAlphaMapId)
{
   cAutoIPtr<IImage> pImage;
   if (BuildSplatAlphaMap(splatTile, xRange, zRange, &pImage) == S_OK)
   {
      if (ConfigIsTrue("debug_write_splat_alpha_maps"))
      {
         cStr file;
         Sprintf(&file, "SplatAlpha_%d_(%d,%d)-(%d,%d).bmp", splatTile,
            xRange.GetStart(), zRange.GetStart(), xRange.GetEnd(),zRange.GetEnd());
         cAutoIPtr<IWriter> pWriter;
         if (FileWriterCreate(cFileSpec(file.c_str()), kFileModeBinary, &pWriter) == S_OK)
         {
            BmpWrite(pImage, pWriter);
         }
      }

      UseGlobal(Renderer);
      return pRenderer->CreateTexture(pImage, true, (void**)pAlphaMapId);
   }

   return E_FAIL;
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSplatBuilder
//

////////////////////////////////////////

cSplatBuilder::cSplatBuilder(uint tile)
 : m_tile(tile)
{
}

////////////////////////////////////////

cSplatBuilder::~cSplatBuilder()
{
}

////////////////////////////////////////

void cSplatBuilder::AddQuad(HTERRAINQUAD hQuad)
{
   if (hQuad != INVALID_HTERRAINQUAD)
   {
      m_quads.insert(hQuad);
   }
}

////////////////////////////////////////

tResult cSplatBuilder::BuildIndexBuffer(const tQuadVertexMap & qvm, std::vector<uint> * pIndices) const
{
   if (pIndices == NULL)
   {
      return E_POINTER;
   }

   pIndices->clear();

   std::set<HTERRAINQUAD>::const_iterator iter;
   for (iter = m_quads.begin(); iter != m_quads.end(); iter++)
   {
      tQuadVertexMap::const_iterator f = qvm.find(*iter);
      if (f == qvm.end())
      {
         continue;
      }
      uint iVert = f->second;
      pIndices->push_back(iVert+2);
      pIndices->push_back(iVert+1);
      pIndices->push_back(iVert);
      pIndices->push_back(iVert);
      pIndices->push_back(iVert+3);
      pIndices->push_back(iVert+2);
   }

   return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSplat
//

////////////////////////////////////////

cSplat::cSplat(const cStr & texture, uint alphaMap, const std::vector<uint> indices, ePrimitiveType primitive)
 : m_texture(texture)
 , m_alphaMap(alphaMap)
 , m_indices(indices.begin(), indices.end())
 , m_primitive(primitive)
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
      tSplats::iterator iter = m_splats.begin();
      for (; iter != m_splats.end(); iter++)
      {
         delete *iter;
      }
      m_splats.clear();
   }
}

////////////////////////////////////////

tResult cTerrainChunk::Create(const cRange<uint> xRange,
                              const cRange<uint> zRange,
                              ITerrainTileSet * pTileSet,
                              uint baseTile,
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
   pChunk->BuildSplats(xRange, zRange, pTileSet, baseTile, bNoBlending);

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

            m_vertices[iVert+0].uv1 = cVec2<float>(0,0);
            m_vertices[iVert+1].uv1 = cVec2<float>(1,0);
            m_vertices[iVert+2].uv1 = cVec2<float>(1,1);
            m_vertices[iVert+3].uv1 = cVec2<float>(0,1);

            for (int j = 0; j < 4; j++)
            {
               m_vertices[iVert+j].pos = corners[j];

               m_vertices[iVert+j].uv2 = cVec2<float>(
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
                                ITerrainTileSet * pTileSet, uint baseTile, bool bNoBlending)
{
   UseGlobal(TerrainModel);

   cTerrainSettings terrainSettings;
   Verify(pTerrainModel->GetTerrainSettings(&terrainSettings) == S_OK);

   typedef std::map<uint, cSplatBuilder *> tSplatBuilderMap;
   tSplatBuilderMap splatBuilders;

   std::map<uint, uint> tileCounts;

   cSplatBuilder * pBaseSplatBuilder = new cSplatBuilder(baseTile);

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

         if (tileCounts.find(tile) == tileCounts.end())
         {
            tileCounts[tile] = 0;
         }

         tileCounts[tile] += 1;

         cSplatBuilder * pSplatBuilder = NULL;
         if (tile != baseTile)
         {
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
         }

         pBaseSplatBuilder->AddQuad(hQuad);

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
      tSplats::iterator iter = m_splats.begin();
      for (; iter != m_splats.end(); iter++)
      {
         delete *iter;
      }
      m_splats.clear();
   }

   // Create the base splat
   {
      std::vector<uint> indices;
      pBaseSplatBuilder->BuildIndexBuffer(m_quadVertexMap, &indices);

      cStr tileTexture;
      if (pTileSet->GetTileTexture(pBaseSplatBuilder->GetTile(), &tileTexture) == S_OK)
      {
         cSplat * pSplat = new cSplat(tileTexture, kNoIndex, indices, kPT_Triangles);
         if (pSplat != NULL)
         {
            m_splats.push_back(pSplat);
         }
      }

      delete pBaseSplatBuilder;
   }

   // Create the splats
   {
      tSplatBuilderMap::iterator iter = splatBuilders.begin();
      for (; iter != splatBuilders.end(); iter++)
      {
         AssertMsg(iter->first != baseTile, "Base splat builder should have been removed");

         cSplatBuilder * pSplatBuilder = iter->second;

         uint alphaMapId = kNoIndex;
         if (!bNoBlending)
         {
            BuildSplatAlphaMap(pSplatBuilder->GetTile(), xRange, zRange, &alphaMapId);
         }

         std::vector<uint> indices;
         pSplatBuilder->BuildIndexBuffer(m_quadVertexMap, &indices);

         cStr tileTexture;
         if (pTileSet->GetTileTexture(pSplatBuilder->GetTile(), &tileTexture) == S_OK)
         {
            cSplat * pSplat = new cSplat(tileTexture, alphaMapId, indices, kPT_Triangles);
            if (pSplat != NULL)
            {
               m_splats.push_back(pSplat);
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
   glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT);
   glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

   glEnable(GL_POLYGON_OFFSET_FILL);

   UseGlobal(Renderer);

   pRenderer->SetVertexFormat(g_terrainVert, _countof(g_terrainVert));
   pRenderer->SubmitVertices(&m_vertices[0], m_vertices.size());
   pRenderer->SetIndexFormat(kIF_32Bit);

   tSplats::iterator iter = m_splats.begin();
   for (int iSplat = 0; iter != m_splats.end(); iter++, iSplat++)
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
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE_SGIS);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE_SGIS);

         glEnable(GL_BLEND);
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      }

      if (pRenderer->SetTexture(1, (*iter)->GetTexture().c_str()) == S_OK)
      {
         glActiveTextureARB(GL_TEXTURE1);
         if (alphaMap != kNoIndex)
         {
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
         }
         else
         {
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
         }
      }

      glPolygonOffset(static_cast<GLfloat>(-iSplat), static_cast<GLfloat>(-iSplat));

      pRenderer->RenderIndexed((*iter)->GetPrimitive(), const_cast<uint*>((*iter)->GetIndexPtr()), (*iter)->GetIndexCount());
   }

   glPopClientAttrib();
   glPopAttrib();
}

/////////////////////////////////////////////////////////////////////////////
