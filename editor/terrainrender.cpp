/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "terrainrender.h"
#include "editorapi.h"
#include "engineapi.h"

#include "imagedata.h"
#include "resourceapi.h"
#include "globalobj.h"
#include "filespec.h"
#include "configapi.h"

#include <algorithm>
#include <functional>
#include <map>

#include <GL/glew.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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

tResult TerrainRendererCreate()
{
   cAutoIPtr<ITerrainRenderer> p(new cTerrainRenderer);
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_ITerrainRenderer, static_cast<ITerrainRenderer*>(p));
}

////////////////////////////////////////

cTerrainRenderer::cTerrainRenderer()
 : m_terrainModelListener(this),
   m_nTilesPerChunk(TerrainRendererDefaults::kTerrainTilesPerChunk),
   m_bEnableBlending(true),
   m_bTerrainChanged(false)
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

   for (uint iz = 0; iz < nChunksZ; iz++)
   {
      cRange<uint> zr(iz * GetTilesPerChunk(), (iz+1) * GetTilesPerChunk());

      for (uint ix = 0; ix < nChunksX; ix++)
      {
         cRange<uint> xr(ix * GetTilesPerChunk(), (ix+1) * GetTilesPerChunk());

         cTerrainChunk * pChunk = NULL;
         if (cTerrainChunkBlended::Create(xr, zr, &pChunk) == S_OK)
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
class std::mem_fun1_t<void, cTerrainChunk, IEditorTileSet*>
{
public:
   typedef cTerrainChunk * first_argument_type;
   typedef IEditorTileSet * second_argument_type;
   typedef void result_type;
   mem_fun1_t(void (cTerrainChunk::*pfn)(IEditorTileSet*))
      : m_pfn(pfn) {}
   void operator()(cTerrainChunk * pChunk, IEditorTileSet * pTileSet) const
   {
      ((pChunk->*m_pfn)(pTileSet));
   }
private:
   void (cTerrainChunk::*m_pfn)(IEditorTileSet*);
};

template <>
class std::binder2nd< std::mem_fun1_t<void, cTerrainChunk, IEditorTileSet*> >
{
public:
   binder2nd(const std::mem_fun1_t<void, cTerrainChunk, IEditorTileSet*> & op,
      IEditorTileSet * value) : m_op(op), m_value(value)
   {
   }
   void operator()(cTerrainChunk * _X) const
   {
      (m_op(_X, m_value));
   }
private:
   std::mem_fun1_t<void, cTerrainChunk, IEditorTileSet*> m_op;
   IEditorTileSet * m_value;
};
#endif

////////////////////////////////////////

void cTerrainRenderer::Render()
{
   UseGlobal(TerrainModel);
   cAutoIPtr<IEditorTileSet> pTileSet;
   if (pTerrainModel->GetTileSet(&pTileSet) != S_OK)
   {
      return;
   }

   if (m_bEnableBlending)
   {
      if (!m_chunks.empty())
      {
         std::for_each(m_chunks.begin(), m_chunks.end(),
            std::bind2nd(std::mem_fun1<void, cTerrainChunk, IEditorTileSet*>(&cTerrainChunk::Render),
               static_cast<IEditorTileSet*>(pTileSet)));
      }
   }
   else
   {
      glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);

      glDisableClientState(GL_EDGE_FLAG_ARRAY);
      glDisableClientState(GL_INDEX_ARRAY);
      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_NORMAL_ARRAY);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glDisableClientState(GL_COLOR_ARRAY);

//      glEnable(GL_COLOR_MATERIAL);

      UseGlobal(ResourceManager);

      tTerrainQuads::const_iterator iter = pTerrainModel->BeginTerrainQuads();
      tTerrainQuads::const_iterator end = pTerrainModel->EndTerrainQuads();
      for (; iter != end; iter++)
      {
         const sTerrainVertex * pVertices = iter->verts;

         cStr texture;
         cAutoIPtr<IEditorTileSet> pEditorTileSet;
         if (pTerrainModel->GetTileSet(&pEditorTileSet) == S_OK
            && pEditorTileSet->GetTileTexture(iter->tile, &texture) == S_OK)
         {
            GLuint tex;
            if (pResourceManager->Load(texture.c_str(), kRT_GlTexture, NULL, (void**)&tex) == S_OK)
            {
               glEnable(GL_TEXTURE_2D);
               glBindTexture(GL_TEXTURE_2D, tex);
            }
         }

         glBegin(GL_QUADS);

         glNormal3f(1,1,1);
         glColor4f(1,1,1,1);

         glTexCoord2fv(pVertices[0].uv1.v);
         glVertex3fv(pVertices[0].pos.v);

         glTexCoord2fv(pVertices[3].uv1.v);
         glVertex3fv(pVertices[3].pos.v);

         glTexCoord2fv(pVertices[2].uv1.v);
         glVertex3fv(pVertices[2].pos.v);

         glTexCoord2fv(pVertices[1].uv1.v);
         glVertex3fv(pVertices[1].pos.v);

         glEnd();
      }

      glPopAttrib();
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
   if (m_pOuter != NULL)
   {
      m_pOuter->m_bTerrainChanged = true;
      m_pOuter->RegenerateChunks();
   }
}

////////////////////////////////////////

void cTerrainRenderer::cTerrainModelListener::OnTerrainClear()
{
   if (m_pOuter != NULL)
   {
      m_pOuter->m_bTerrainChanged = true;
      m_pOuter->ClearChunks();
   }
}

////////////////////////////////////////

void cTerrainRenderer::cTerrainModelListener::OnTerrainChange()
{
   if (m_pOuter != NULL)
   {
      m_pOuter->m_bTerrainChanged = true;
      m_pOuter->RegenerateChunks();
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

inline float STW(const tVec2 & pt1, const tVec2 & pt2)
{
   return Clamp(SplatTexelWeight(pt1,pt2), 0.f, 1.f);
}

////////////////////////////////////////

static const float g_texelWeights[4][8] =
{
   {
      // from upper left texel (-.25, -.25)
      STW(tVec2(-.25,-.25), tVec2(-1,-1)), // top left
      STW(tVec2(-.25,-.25), tVec2( 0,-1)), // top mid
      STW(tVec2(-.25,-.25), tVec2( 1,-1)), // top right
      STW(tVec2(-.25,-.25), tVec2(-1, 0)), // left
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
      STW(tVec2(.25,.25), tVec2( 1, 0)), // right
      STW(tVec2(.25,.25), tVec2(-1, 1)), // bottom left
      STW(tVec2(.25,.25), tVec2( 0, 1)), // bottom mid
      STW(tVec2(.25,.25), tVec2( 1, 1)), // bottom right
   },
};

////////////////////////////////////////

void BuildSplatAlphaMap(uint splatTile, const cRange<uint> xRange, const cRange<uint> zRange, uint * pAlphaMapId)
{
   UseGlobal(TerrainModel);
   UseGlobal(TerrainRenderer);

   cTerrainSettings terrainSettings;
   Verify(pTerrainModel->GetTerrainSettings(&terrainSettings) == S_OK);

   BITMAPINFO bmi = {0};
   bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
   bmi.bmiHeader.biWidth = pTerrainRenderer->GetTilesPerChunk() * 2;
   bmi.bmiHeader.biHeight = -(static_cast<int>(pTerrainRenderer->GetTilesPerChunk()) * 2);
   bmi.bmiHeader.biPlanes = 1;
   bmi.bmiHeader.biCompression = BI_RGB;
   bmi.bmiHeader.biBitCount = 32;
   byte * pBitmapBits = NULL;
   HBITMAP hBitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&pBitmapBits, NULL, 0);
   if (hBitmap == NULL)
   {
      return;
   }

   DIBSECTION dibSection = {0};
   Verify(GetObject(hBitmap, sizeof(dibSection), &dibSection));

   uint redMask = dibSection.dsBitfields[0];
   uint greenMask = dibSection.dsBitfields[1];
   uint blueMask = dibSection.dsBitfields[2];

   for (uint z = zRange.GetStart(); z < zRange.GetEnd(); z++)
   {
      uint zPrev = zRange.GetPrev(z, 0, kNoIndex);
      uint zNext = zRange.GetNext(z, terrainSettings.GetTileCountZ(), kNoIndex);

      for (uint x = xRange.GetStart(); x < xRange.GetEnd(); x++)
      {
         uint xPrev = xRange.GetPrev(x, 0, kNoIndex);
         uint xNext = xRange.GetNext(x, terrainSettings.GetTileCountX(), kNoIndex);

         const uint neighborCoords[8][2] =
         {
            {xPrev, zPrev},
            {x,     zPrev},
            {xNext, zPrev},
            {xPrev, z,   },
            {xNext, z,   },
            {xPrev, zNext},
            {x,     zNext},
            {xNext, zNext},
         };

         float texelWeights[4];
         int texelDivisors[4];

         memset(texelWeights, 0, sizeof(texelWeights));
         memset(texelDivisors, 0, sizeof(texelDivisors));

         for (int i = 0; i < _countof(neighborCoords); i++)
         {
            uint tile, nx = neighborCoords[i][0], nz = neighborCoords[i][1];
            if (pTerrainModel->GetQuadTile(nx, nz, &tile) == S_OK)
            {
               if (tile == splatTile)
               {
                  for (int j = 0; j < _countof(texelWeights); j++)
                  {
                     texelWeights[j] += g_texelWeights[j][i];
                     texelDivisors[j] += 1;
                  }
               }
            }
         }

//         if (texelDivisors[0] != 0) texelWeights[0] /= texelDivisors[0];
//         if (texelDivisors[1] != 0) texelWeights[1] /= texelDivisors[1];
//         if (texelDivisors[2] != 0) texelWeights[2] /= texelDivisors[2];
//         if (texelDivisors[3] != 0) texelWeights[3] /= texelDivisors[3];

         texelWeights[0] = Clamp(texelWeights[0], 0.f, 1.f);
         texelWeights[1] = Clamp(texelWeights[1], 0.f, 1.f);
         texelWeights[2] = Clamp(texelWeights[2], 0.f, 1.f);
         texelWeights[3] = Clamp(texelWeights[3], 0.f, 1.f);

         uint iz = (z - zRange.GetStart()) * 2;
         uint ix = (x - xRange.GetStart()) * 2;

         uint m = bmi.bmiHeader.biBitCount / 8;

         byte * p0 = pBitmapBits + (iz * bmi.bmiHeader.biWidth * m) + (ix * m);
         byte * p1 = pBitmapBits + (iz * bmi.bmiHeader.biWidth * m) + ((ix+1) * m);
         byte * p2 = pBitmapBits + ((iz+1) * bmi.bmiHeader.biWidth * m) + (ix * m);
         byte * p3 = pBitmapBits + ((iz+1) * bmi.bmiHeader.biWidth * m) + ((ix+1) * m);

         p0[0] = p0[1] = p0[2] = p0[3] = (byte)(255 * texelWeights[0]);
         p1[0] = p1[1] = p1[2] = p1[3] = (byte)(255 * texelWeights[1]);
         p2[0] = p2[1] = p2[2] = p2[3] = (byte)(255 * texelWeights[2]);
         p3[0] = p3[1] = p3[2] = p3[3] = (byte)(255 * texelWeights[3]);
      }
   }

   cImageData * pImage = new cImageData;
   if (pImage != NULL)
   {
      if (pImage->Create(bmi.bmiHeader.biWidth, abs(bmi.bmiHeader.biHeight), kPF_RGBA8888, pBitmapBits))
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

   DeleteObject(hBitmap), hBitmap = NULL;
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSplatBuilder
//

////////////////////////////////////////

cSplatBuilder::cSplatBuilder(const cRange<uint> xRange, const cRange<uint> zRange,
                             uint tile, uint alphaMapId)
 : m_xRange(xRange),
   m_zRange(zRange),
   m_tile(tile),
   m_alphaMapId(alphaMapId)
{
}

////////////////////////////////////////

cSplatBuilder::~cSplatBuilder()
{
}

////////////////////////////////////////

tResult cSplatBuilder::GetGlTexture(IEditorTileSet * pTileSet, uint * pTexId)
{
   cStr tileTex;
   if (pTileSet->GetTileTexture(m_tile, &tileTex) == S_OK)
   {
      UseGlobal(ResourceManager);
      return pResourceManager->Load(tileTex.c_str(), kRT_GlTexture, NULL, (void**)pTexId);
   }
   return E_FAIL;
}

////////////////////////////////////////

tResult cSplatBuilder::GetAlphaMap(uint * pAlphaMapId)
{
   if (pAlphaMapId == NULL)
   {
      return E_POINTER;
   }
   *pAlphaMapId = m_alphaMapId;
   return S_OK;
}

////////////////////////////////////////

void cSplatBuilder::AddQuad(uint x, uint z)
{
#ifdef _DEBUG
   tSplatQuadSet::iterator f = m_quads.find(std::make_pair(x,z));
   if (f != m_quads.end())
   {
      Assert(f->first == x && f->second == z);
   }
#endif
   std::pair<tSplatQuadSet::iterator, bool> result = m_quads.insert(std::make_pair(x,z));
   m_indices.clear();
}

////////////////////////////////////////

size_t cSplatBuilder::GetIndexCount() const
{
   GetIndexPtr();
   return m_indices.size();
}

////////////////////////////////////////

const uint * cSplatBuilder::GetIndexPtr() const
{
   if (m_indices.empty())
   {
      tSplatQuadSet::const_iterator iter;
      for (iter = m_quads.begin(); iter != m_quads.end(); iter++)
      {
         uint iVert = ((iter->second - m_zRange.GetStart()) * m_zRange.GetLength() + iter->first - m_xRange.GetStart()) * 4;
         m_indices.push_back(iVert+2);
         m_indices.push_back(iVert+1);
         m_indices.push_back(iVert);
         m_indices.push_back(iVert);
         m_indices.push_back(iVert+3);
         m_indices.push_back(iVert+2);
      }
   }

   return &m_indices[0];
}

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainChunkBlended
//

////////////////////////////////////////

cTerrainChunkBlended::cTerrainChunkBlended()
{
}

////////////////////////////////////////

cTerrainChunkBlended::~cTerrainChunkBlended()
{
   tSplatBuilders::iterator iter = m_splats.begin();
   tSplatBuilders::iterator end = m_splats.end();
   for (; iter != end; iter++)
   {
      delete *iter;
   }
   m_splats.clear();
}

////////////////////////////////////////

tResult cTerrainChunkBlended::Create(const cRange<uint> xRange,
                                     const cRange<uint> zRange,
                                     cTerrainChunk * * ppChunk)
{
   if (ppChunk == NULL)
   {
      return E_POINTER;
   }

   cTerrainChunkBlended * pChunk = new cTerrainChunkBlended;
   if (!pChunk)
   {
      return E_OUTOFMEMORY;
   }

   UseGlobal(TerrainModel);
   UseGlobal(TerrainRenderer);

   cTerrainSettings terrainSettings;
   Verify(pTerrainModel->GetTerrainSettings(&terrainSettings) == S_OK);

   pChunk->m_vertices.resize(xRange.GetLength() * zRange.GetLength() * 4);

   typedef std::map<uint, cSplatBuilder *> tSplatBuilderMap;
   tSplatBuilderMap splatBuilders;

   float d = 1.0f / pTerrainRenderer->GetTilesPerChunk();

   for (uint z = zRange.GetStart(); z < zRange.GetEnd(); z++)
   {
      uint zPrev = zRange.GetPrev(z, 0, kNoIndex);
      uint zNext = zRange.GetNext(z, terrainSettings.GetTileCountZ(), kNoIndex);

      for (uint x = xRange.GetStart(); x < xRange.GetEnd(); x++)
      {
         uint xPrev = xRange.GetPrev(x, 0, kNoIndex);
         uint xNext = xRange.GetNext(x, terrainSettings.GetTileCountX(), kNoIndex);

         uint tile;
         Verify(pTerrainModel->GetQuadTile(x, z, &tile) == S_OK);

         cSplatBuilder * pSplatBuilder = NULL;

         if (splatBuilders.find(tile) == splatBuilders.end())
         {
            uint alphaMapId;
            BuildSplatAlphaMap(tile, xRange, zRange, &alphaMapId);

            pSplatBuilder = new cSplatBuilder(xRange, zRange, tile, alphaMapId);
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
            pSplatBuilder->AddQuad(x,z);

            const uint neighbors[][2] =
            {
               {xPrev, zPrev},
               {x,     zPrev},
               {xNext, zPrev},
               {xPrev, z,   },
               {xNext, z,   },
               {xPrev, zNext},
               {x,     zNext},
               {xNext, zNext},
            };

            for (int i = 0; i < _countof(neighbors); i++)
            {
               // Add bordering quads too (these provide some of the fade)
               uint ntile, nx = neighbors[i][0], nz = neighbors[i][1];
               if (pTerrainModel->GetQuadTile(nx, nz, &ntile) == S_OK
                  && ntile != tile)
               {
                  pSplatBuilder->AddQuad(nx,nz);
               }
            }
         }

         uint iVert = ((z - zRange.GetStart()) * zRange.GetLength() + x - xRange.GetStart()) * 4;

         sTerrainVertex verts[4];
         if (pTerrainModel->GetQuadVertices(x, z, verts) == S_OK)
         {
            pChunk->m_vertices[iVert+0] = verts[0];
            pChunk->m_vertices[iVert+1] = verts[1];
            pChunk->m_vertices[iVert+2] = verts[2];
            pChunk->m_vertices[iVert+3] = verts[3];

            float u = d * (x - xRange.GetStart());
            float v = d * (z - zRange.GetStart());

            pChunk->m_vertices[iVert+0].uv2 = tVec2(u, v);
            pChunk->m_vertices[iVert+1].uv2 = tVec2(u+d, v);
            pChunk->m_vertices[iVert+2].uv2 = tVec2(u+d, v+d);
            pChunk->m_vertices[iVert+3].uv2 = tVec2(u, v+d);
         }
      }
   }

   tSplatBuilderMap::iterator iter = splatBuilders.begin();
   tSplatBuilderMap::iterator end = splatBuilders.end();
   for (; iter != end; iter++)
   {
      pChunk->m_splats.push_back(iter->second);
   }
   splatBuilders.clear();

   *ppChunk = pChunk;

   return S_OK;
}

////////////////////////////////////////

void cTerrainChunkBlended::Render(IEditorTileSet * pTileSet)
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

//   glColor4f(1,1,1,1);

   tSplatBuilders::iterator iter = m_splats.begin();
   tSplatBuilders::iterator end = m_splats.end();
   for (; iter != end; iter++)
   {
      RenderSplatDstAlpha(*iter, pTileSet, pVertexData);
   }

   glPopClientAttrib();
   glPopAttrib();
}

////////////////////////////////////////

void cTerrainChunkBlended::RenderSplatDstAlpha(cSplatBuilder * pSplat, IEditorTileSet * pTileSet, const byte * pVertexData)
{
//   glDisable(GL_BLEND);
   glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);

   GLuint alphaMapId;
   if (pSplat->GetAlphaMap(&alphaMapId) == S_OK)
   {
      glTexCoordPointer(2, GL_FLOAT, sizeof(sTerrainVertex), pVertexData + offsetof(sTerrainVertex, uv2));
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glBindTexture(GL_TEXTURE_2D, alphaMapId);
      glEnable(GL_TEXTURE_2D);
//      glBlendFunc(GL_SRC_COLOR, GL_ZERO);
      glDrawElements(GL_TRIANGLES, pSplat->GetIndexCount(), GL_UNSIGNED_INT, pSplat->GetIndexPtr());
   }

   glEnable(GL_BLEND);
//   glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
   glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

   GLuint texId;
   if (pSplat->GetGlTexture(pTileSet, &texId) == S_OK)
   {
      glTexCoordPointer(2, GL_FLOAT, sizeof(sTerrainVertex), pVertexData + offsetof(sTerrainVertex, uv1));
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glBindTexture(GL_TEXTURE_2D, texId);
      glEnable(GL_TEXTURE_2D);
      glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
//      glBlendFunc(GL_SRC_COLOR, GL_DST_ALPHA);
      glDrawElements(GL_TRIANGLES, pSplat->GetIndexCount(), GL_UNSIGNED_INT, pSplat->GetIndexPtr());
   }
}


/////////////////////////////////////////////////////////////////////////////
