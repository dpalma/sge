/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "terrainrender.h"
#include "editorapi.h"
#include "engineapi.h"

#include "renderapi.h"

#include "imagedata.h"
#include "resourceapi.h"
#include "globalobj.h"
#include "filespec.h"

#include <map>
#include <algorithm>
#include <functional>
#include <GL/glew.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// REFERENCES
// http://cbloom.com/3d/techdocs/splatting.txt
// http://oss.sgi.com/projects/ogl-sample/registry/ARB/texture_env_combine.txt

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

   for (tChunks::iterator iter = m_chunks.begin(); iter != m_chunks.end(); iter++)
   {
      delete *iter;
   }
   m_chunks.clear();

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

   cAutoIPtr<IEditorTileSet> pTileSet;
   pTerrainModel->GetTileSet(&pTileSet);

   for (uint iz = 0; iz < nChunksZ; iz++)
   {
      for (uint ix = 0; ix < nChunksX; ix++)
      {
         cTerrainChunk * pChunk = NULL;
         if (cTerrainChunkBlended::Create(pTerrainModel->GetTerrainQuads(),
            terrainSettings.GetTileCountX(), terrainSettings.GetTileCountZ(),
            ix, iz, pTileSet, &pChunk) == S_OK)
         {
            m_chunks.push_back(pChunk);
         }
      }
   }

   m_bTerrainChanged = false;
}

////////////////////////////////////////

// VC6 requires explicit instantiation of mem_fun_t for void return type
#if _MSC_VER <= 1200
template <>
class std::mem_fun_t<void, cTerrainChunkBlended>
{
public:
   mem_fun_t(void (cTerrainChunkBlended::*pfn)())
      : m_pfn(pfn) {}
	void operator()(cTerrainChunkBlended * p) const
   {
      ((p->*m_pfn)());
   }
private:
	void (cTerrainChunkBlended::*m_pfn)();
};
#endif

////////////////////////////////////////

void cTerrainRenderer::Render()
{
   if (m_bEnableBlending)
   {
      if (!m_chunks.empty())
      {
         std::for_each(m_chunks.begin(), m_chunks.end(), std::mem_fun(&cTerrainChunk::Render));
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
      UseGlobal(TerrainModel);

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
      m_pOuter->RegenerateChunks();
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
//
// CLASS: cTerrainChunkSimple
//

////////////////////////////////////////

tResult cTerrainChunkSimple::Create(uint iChunkX, uint iChunkZ, cTerrainChunk * * ppChunk)
{
   if (ppChunk == NULL)
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

   cTerrainChunkSimple * pChunk = new cTerrainChunkSimple;
   if (pChunk == NULL)
   {
      return E_OUTOFMEMORY;
   }

   const uint tilesPerChunk = pTerrainRenderer->GetTilesPerChunk();

   pChunk->m_vertices.reserve(tilesPerChunk * tilesPerChunk * 4);

   uint iTileZ = iChunkZ * tilesPerChunk;
   uint iEndZ = iTileZ + tilesPerChunk;
   for (; iTileZ < iEndZ; iTileZ++)
   {
      uint iTileX = iChunkX * tilesPerChunk;
      uint iEndX = iTileX + tilesPerChunk;
      for (; iTileX < iEndX; iTileX++)
      {
         uint iTileQuad = (iTileZ * terrainSettings.GetTileCountZ()) + iTileX;

         tVec3 tileQuadVerts[4];
         if (pTerrainModel->GetTileVertices(iTileX, iTileZ, tileQuadVerts) == S_OK)
         {
            // TODO
         }
      }
   }

   *ppChunk = pChunk;
   return S_OK;
}

////////////////////////////////////////

cTerrainChunkSimple::cTerrainChunkSimple()
{
}

////////////////////////////////////////

cTerrainChunkSimple::~cTerrainChunkSimple()
{
}

////////////////////////////////////////

void cTerrainChunkSimple::Render()
{
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSplatBuilder
//

////////////////////////////////////////

cSplatBuilder::cSplatBuilder(uint tile, const char * pszTexture)
 : m_tile(tile),
   m_tileTexture(pszTexture ? pszTexture : ""),
   m_alphaMapId(0)
{
}

////////////////////////////////////////

cSplatBuilder::~cSplatBuilder()
{
}

////////////////////////////////////////

tResult cSplatBuilder::GetGlTexture(uint * pTexId)
{
   UseGlobal(ResourceManager);
   return pResourceManager->Load(m_tileTexture.c_str(), kRT_GlTexture, NULL, (void**)pTexId);
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

void cSplatBuilder::AddTriangle(uint i0, uint i1, uint i2)
{
   m_indices.push_back(i0);
   m_indices.push_back(i1);
   m_indices.push_back(i2);
}

////////////////////////////////////////

size_t cSplatBuilder::GetIndexCount() const
{
   return m_indices.size();
}

////////////////////////////////////////

const uint * cSplatBuilder::GetIndexPtr() const
{
   return &m_indices[0];
}

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

static const sTerrainQuad * GetQuadPtr(uint x, uint z, const tTerrainQuads & quads, uint nQuadsX, uint nQuadsZ)
{
   if (x == kNoIndex || z == kNoIndex)
   {
      return NULL;
   }
   else
   {
      return &quads[(z * nQuadsZ) + x];
   }
}

void cSplatBuilder::BuildAlphaMap(const tTerrainQuads & quads,
                                  uint nQuadsX, uint nQuadsZ,
                                  uint iChunkX, uint iChunkZ)
{
   UseGlobal(TerrainRenderer);

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

   uint zStart = iChunkZ * pTerrainRenderer->GetTilesPerChunk();
   uint zEnd = zStart + pTerrainRenderer->GetTilesPerChunk();
   uint xStart = iChunkX * pTerrainRenderer->GetTilesPerChunk();
   uint xEnd = xStart + pTerrainRenderer->GetTilesPerChunk();

   for (uint z = zStart; z < zEnd; z++)
   {
      uint zPrev = kNoIndex;
      if (z > zStart)
      {
         zPrev = z - 1;
      }
      else if ((z == zStart) && (zStart > 0))
      {
         zPrev = zStart - 1;
      }

      uint zNext = kNoIndex;
      if (z < (zEnd - 1))
      {
         zNext = z + 1;
      }
      else if ((z == (zEnd - 1)) && (zEnd < nQuadsZ))
      {
         zNext = zEnd;
      }

      for (uint x = xStart; x < xEnd; x++)
      {
         uint xPrev = kNoIndex;
         if (x > xStart)
         {
            xPrev = x - 1;
         }
         else if ((x == xStart) && (xStart > 0))
         {
            xPrev = xStart - 1;
         }

         uint xNext = kNoIndex;
         if (x < (xEnd - 1))
         {
            xNext = x + 1;
         }
         else if ((x == (xEnd - 1)) && (xEnd < nQuadsX))
         {
            xNext = xEnd;
         }

         const sTerrainQuad * neighbors[8] =
         {
            GetQuadPtr(xPrev, zPrev, quads, nQuadsX, nQuadsZ),
            GetQuadPtr(x,     zPrev, quads, nQuadsX, nQuadsZ),
            GetQuadPtr(xNext, zPrev, quads, nQuadsX, nQuadsZ),
            GetQuadPtr(xPrev, z,     quads, nQuadsX, nQuadsZ),
            GetQuadPtr(xNext, z,     quads, nQuadsX, nQuadsZ),
            GetQuadPtr(xPrev, zNext, quads, nQuadsX, nQuadsZ),
            GetQuadPtr(x,     zNext, quads, nQuadsX, nQuadsZ),
            GetQuadPtr(xNext, zNext, quads, nQuadsX, nQuadsZ),
         };

         float texelWeights[4];
         int texelDivisors[4];

         memset(texelWeights, 0, sizeof(texelWeights));
         memset(texelDivisors, 0, sizeof(texelDivisors));

         for (int i = 0; i < _countof(neighbors); i++)
         {
            if (neighbors[i] == NULL)
            {
               continue;
            }

            if (neighbors[i]->tile == m_tile)
            {
               for (int j = 0; j < _countof(texelWeights); j++)
               {
                  texelWeights[j] += g_texelWeights[j][i];
                  texelDivisors[j] += 1;
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

         uint iz = (z - zStart) * 2;
         uint ix = (x - xStart) * 2;

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
#if 0 && defined(_DEBUG)
         cStr file;
         file.Format("%sAlpha%d%d.bmp", m_tileTexture.c_str(), iChunkX, iChunkZ);
         cAutoIPtr<IWriter> pWriter(FileCreateWriter(cFileSpec(file.c_str())));
         BmpWrite(pImage, pWriter);
#endif

         GlTextureCreate(pImage, &m_alphaMapId);
      }

      delete pImage;
      pImage = NULL;
   }

   DeleteObject(hBitmap), hBitmap = NULL;
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

tResult cTerrainChunkBlended::Create(const tTerrainQuads & quads,
                              uint nQuadsX, uint nQuadsZ,
                              uint iChunkX, uint iChunkZ, 
                              IEditorTileSet * pTileSet,
                              cTerrainChunk * * ppChunk)
{
   if (pTileSet == NULL || ppChunk == NULL)
   {
      return E_POINTER;
   }

   cTerrainChunkBlended * pChunk = new cTerrainChunkBlended;
   if (!pChunk)
   {
      return E_OUTOFMEMORY;
   }

   UseGlobal(TerrainRenderer);

   pChunk->m_vertices.resize(pTerrainRenderer->GetTilesPerChunk() * pTerrainRenderer->GetTilesPerChunk() * 4);
   uint iVert = 0;

   typedef std::map<uint, cSplatBuilder *> tSplatBuilderMap;
   tSplatBuilderMap splatBuilders;

   uint ix = iChunkX * pTerrainRenderer->GetTilesPerChunk();
   uint iz = iChunkZ * pTerrainRenderer->GetTilesPerChunk();
   uint cx = pTerrainRenderer->GetTilesPerChunk(), cz = pTerrainRenderer->GetTilesPerChunk();

   float d = 1.0f / pTerrainRenderer->GetTilesPerChunk();

   for (uint z = iz; z < (iz + cz); z++)
   {
      for (uint x = ix; x < (ix + cx); x++)
      {
         uint iQuad = (z * nQuadsZ) + x;
         const sTerrainQuad & quad = quads[iQuad];

         cSplatBuilder * pSplatBuilder = NULL;

         if (splatBuilders.find(quad.tile) == splatBuilders.end())
         {
            cStr tileTexture;
            Verify(pTileSet->GetTileTexture(quad.tile, &tileTexture) == S_OK);
            pSplatBuilder = new cSplatBuilder(quad.tile, tileTexture.c_str());
            if (pSplatBuilder != NULL)
            {
               splatBuilders[quad.tile] = pSplatBuilder;
            }
         }
         else
         {
            pSplatBuilder = splatBuilders[quad.tile];
         }

         if (pSplatBuilder != NULL)
         {
            pSplatBuilder->AddTriangle(iVert+2,iVert+1,iVert+0);
            pSplatBuilder->AddTriangle(iVert+0,iVert+3,iVert+2);
         }

         float u = d * (x - ix);
         float v = d * (z - iz);

         pChunk->m_vertices[iVert+0].uv2 = tVec2(u, v);
         pChunk->m_vertices[iVert+1].uv2 = tVec2(u+d, v);
         pChunk->m_vertices[iVert+2].uv2 = tVec2(u+d, v+d);
         pChunk->m_vertices[iVert+3].uv2 = tVec2(u, v+d);

         pChunk->m_vertices[iVert++] = quad.verts[0];
         pChunk->m_vertices[iVert++] = quad.verts[1];
         pChunk->m_vertices[iVert++] = quad.verts[2];
         pChunk->m_vertices[iVert++] = quad.verts[3];
      }
   }

   Assert(pChunk->m_vertices.size() == iVert);

   tSplatBuilderMap::iterator iter = splatBuilders.begin();
   tSplatBuilderMap::iterator end = splatBuilders.end();
   for (; iter != end; iter++)
   {
      iter->second->BuildAlphaMap(quads, nQuadsX, nQuadsZ, iChunkX, iChunkZ);
      pChunk->m_splats.push_back(iter->second);
   }
   splatBuilders.clear();

   *ppChunk = pChunk;

   return S_OK;
}

////////////////////////////////////////

void cTerrainChunkBlended::Render()
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
      glDisable(GL_BLEND);
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);

      GLuint alphaMapId;
      if ((*iter)->GetAlphaMap(&alphaMapId) == S_OK)
      {
         glTexCoordPointer(2, GL_FLOAT, sizeof(sTerrainVertex), pVertexData + offsetof(sTerrainVertex, uv2));
         glEnableClientState(GL_TEXTURE_COORD_ARRAY);
         glBindTexture(GL_TEXTURE_2D, alphaMapId);
         glEnable(GL_TEXTURE_2D);
//         glBlendFunc(GL_SRC_COLOR, GL_ZERO);
         glDrawElements(GL_TRIANGLES, (*iter)->GetIndexCount(), GL_UNSIGNED_INT, (*iter)->GetIndexPtr());
      }

      glEnable(GL_BLEND);
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

      GLuint texId;
      if ((*iter)->GetGlTexture(&texId) == S_OK)
      {
         glTexCoordPointer(2, GL_FLOAT, sizeof(sTerrainVertex), pVertexData + offsetof(sTerrainVertex, uv1));
         glEnableClientState(GL_TEXTURE_COORD_ARRAY);
         glBindTexture(GL_TEXTURE_2D, texId);
         glEnable(GL_TEXTURE_2D);
//         glBlendFunc(GL_SRC_COLOR, GL_DST_ALPHA);
         glDrawElements(GL_TRIANGLES, (*iter)->GetIndexCount(), GL_UNSIGNED_INT, (*iter)->GetIndexPtr());
      }
   }

   glPopClientAttrib();
   glPopAttrib();
}

/////////////////////////////////////////////////////////////////////////////
