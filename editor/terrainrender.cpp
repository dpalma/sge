/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "terrainrender.h"
#include "editorapi.h"

#include "materialapi.h"
#include "renderapi.h"
#include "textureapi.h"

#include "imagedata.h"

#include <algorithm>
#include <GL/gl.h>
#include <GL/glext.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// REFERENCES
// http://cbloom.com/3d/techdocs/splatting.txt
// http://oss.sgi.com/projects/ogl-sample/registry/ARB/texture_env_combine.txt

const int kTilesPerChunk = 32;

const uint kNoIndex = ~0;

/////////////////////////////////////////////////////////////////////////////
// HACK: this stuff will go away when the code is converted to using
// IRenderDevice, IMaterial, etc.
typedef void (APIENTRY * tglActiveTextureARB) (GLenum texture);
typedef void (APIENTRY * tglClientActiveTextureARB) (GLenum texture);
tglActiveTextureARB pfnglActiveTextureARB = NULL;
tglClientActiveTextureARB pfnglClientActiveTextureARB = NULL;
void InitGLExtensions()
{
   if (pfnglActiveTextureARB == NULL)
   {
      pfnglActiveTextureARB = (tglActiveTextureARB)wglGetProcAddress("glActiveTextureARB");
   }
   if (pfnglClientActiveTextureARB == NULL)
   {
      pfnglClientActiveTextureARB = (tglClientActiveTextureARB)wglGetProcAddress("glClientActiveTextureARB");
   }
}

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
   return S_OK;
}

////////////////////////////////////////

BEGIN_CONSTRAINTS()
   AFTER_GUID(IID_IScene)
END_CONSTRAINTS()

////////////////////////////////////////

cTerrainRenderer::cTerrainRenderer()
 : cGlobalObject<IMPLEMENTS(ITerrainRenderer)>("TerrainRenderer", CONSTRAINTS()),
   m_sceneEntity(this),
   m_tml(this),
   m_nChunksX(0),
   m_nChunksZ(0),
   m_bEnableBlending(true),
   m_bTerrainChanged(false)
{
}

////////////////////////////////////////

cTerrainRenderer::~cTerrainRenderer()
{
}

////////////////////////////////////////

tResult cTerrainRenderer::Init()
{
   // HACK
   UseGlobal(Scene);
   pScene->AddEntity(kSL_Terrain, &m_sceneEntity);

   return S_OK;
}

////////////////////////////////////////

tResult cTerrainRenderer::Term()
{
   // HACK
   UseGlobal(Scene);
   pScene->RemoveEntity(kSL_Terrain, &m_sceneEntity);

   std::for_each(m_chunks.begin(), m_chunks.end(), CTInterfaceMethod(&cTerrainChunk::Release));
   m_chunks.clear();

   SetModel(NULL);

   return S_OK;
}

////////////////////////////////////////

tResult cTerrainRenderer::SetModel(ITerrainModel * pTerrainModel)
{
   if (!!m_pModel)
   {
      m_pModel->RemoveTerrainModelListener(&m_tml);
      SafeRelease(m_pModel);
   }
   if (pTerrainModel != NULL)
   {
      m_pModel = CTAddRef(pTerrainModel);
      m_pModel->AddTerrainModelListener(&m_tml);
      RegenerateChunks();
   }
   return S_OK;
}

////////////////////////////////////////

tResult cTerrainRenderer::GetModel(ITerrainModel * * ppTerrainModel)
{
   return m_pModel.GetPointer(ppTerrainModel);
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

   if (!m_pModel)
   {
      return;
   }

   std::for_each(m_chunks.begin(), m_chunks.end(), CTInterfaceMethod(&cTerrainChunk::Release));
   m_chunks.clear();

   uint nTilesX, nTilesZ;
   m_pModel->GetDimensions(&nTilesX, &nTilesZ);

   m_nChunksX = nTilesX / kTilesPerChunk;
   m_nChunksZ = nTilesZ / kTilesPerChunk;

   cAutoIPtr<IEditorTileSet> pTileSet;
   m_pModel->GetTileSet(&pTileSet);

   for (uint iz = 0; iz < m_nChunksZ; iz++)
   {
      for (uint ix = 0; ix < m_nChunksX; ix++)
      {
         cAutoIPtr<cTerrainChunk> pChunk;
         if (cTerrainChunk::Create(m_pModel->GetTerrainQuads(), nTilesX, nTilesZ,
                                   ix, iz, pTileSet, &pChunk) == S_OK)
         {
            m_chunks.push_back(CTAddRef(pChunk));
         }
      }
   }

   m_bTerrainChanged = false;
}

////////////////////////////////////////

tResult cTerrainRenderer::Render(IRenderDevice * pRenderDevice)
{
   if (m_bEnableBlending)
   {
      if (!m_chunks.empty())
      {
         tChunks::iterator iter = m_chunks.begin();
         tChunks::iterator end = m_chunks.end();
         for (; iter != end; iter++)
         {
            (*iter)->Render(pRenderDevice);
         }
         return S_OK;
      }

      return E_FAIL;
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

      glEnable(GL_COLOR_MATERIAL);

      tTerrainQuads::const_iterator iter = m_pModel->GetTerrainQuads().begin();
      tTerrainQuads::const_iterator end = m_pModel->GetTerrainQuads().end();
      for (; iter != end; iter++)
      {
         const sTerrainVertex * pVertices = iter->verts;

         cAutoIPtr<IEditorTileSet> pEditorTileSet;
         if (m_pModel->GetTileSet(&pEditorTileSet) == S_OK)
         {
            cAutoIPtr<ITexture> pTexture;
            if (pEditorTileSet->GetTileTexture(iter->tile, &pTexture) == S_OK)
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
}

////////////////////////////////////////

cTerrainRenderer::cSceneEntity::cSceneEntity(cTerrainRenderer * pOuter)
 : m_pOuter(pOuter),
   m_translation(0,0,0),
   m_rotation(0,0,0,1),
   m_transform(tMatrix4::GetIdentity())
{
}

////////////////////////////////////////

cTerrainRenderer::cSceneEntity::~cSceneEntity()
{
}

////////////////////////////////////////

void cTerrainRenderer::cSceneEntity::Render(IRenderDevice * pRenderDevice)
{
   Assert(m_pOuter != NULL);
   if (m_pOuter->Render(pRenderDevice) == S_OK)
   {
   }
}



/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSplat
//

////////////////////////////////////////

cSplat::cSplat()
{
}

////////////////////////////////////////

cSplat::~cSplat()
{
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSplatBuilder
//

////////////////////////////////////////

cSplatBuilder::cSplatBuilder(IEditorTileSet * pTileSet, uint tile)
 : m_pTileSet(CTAddRef(pTileSet)),
   m_tile(tile)
{
   if (pTileSet != NULL)
   {
      pTileSet->GetTileTexture(tile, &m_pTexture);
   }
}

////////////////////////////////////////

cSplatBuilder::~cSplatBuilder()
{
}

////////////////////////////////////////

tResult cSplatBuilder::GetTexture(ITexture * * ppTexture)
{
   return m_pTexture.GetPointer(ppTexture);
}

////////////////////////////////////////

tResult cSplatBuilder::GetAlphaMap(ITexture * * ppTexture)
{
   return m_pAlphaMap.GetPointer(ppTexture);
}

////////////////////////////////////////

void cSplatBuilder::AddTriangle(uint i0, uint i1, uint i2)
{
   m_indices.push_back(i0);
   m_indices.push_back(i1);
   m_indices.push_back(i2);
}

////////////////////////////////////////

tResult cSplatBuilder::CreateIndexBuffer(IIndexBuffer * * ppIndexBuffer)
{
   if (ppIndexBuffer == NULL)
   {
      return E_POINTER;
   }

   if ((m_indices.size() % 3) != 0)
   {
      return E_FAIL;
   }

   return E_NOTIMPL;
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

#ifndef NDEBUG
static void WriteBitmapFile(const char * pszFileName, BITMAPINFO * pBmInfo, void * pBits)
{
   int headerSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

   int bitsSize = abs(pBmInfo->bmiHeader.biWidth * pBmInfo->bmiHeader.biHeight * pBmInfo->bmiHeader.biBitCount / 8);

   BITMAPFILEHEADER fileHeader;
   fileHeader.bfType = 0x4D42;
   fileHeader.bfReserved1 = 0;
   fileHeader.bfReserved2 = 0;
   fileHeader.bfOffBits = headerSize;
   fileHeader.bfSize = headerSize + bitsSize;

   BITMAPINFOHEADER infoHeader;
   memcpy(&infoHeader, &pBmInfo->bmiHeader, sizeof(BITMAPINFOHEADER));

   HANDLE hFile = CreateFile(pszFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
   if (hFile != NULL && hFile != INVALID_HANDLE_VALUE)
   {
      DWORD bytesWritten;
      WriteFile(hFile, &fileHeader, sizeof(BITMAPFILEHEADER), &bytesWritten, NULL);
      WriteFile(hFile, &infoHeader, sizeof(BITMAPINFOHEADER), &bytesWritten, NULL);
      WriteFile(hFile, pBits, bitsSize, &bytesWritten, NULL);
      CloseHandle(hFile);
   }
}
#endif

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
   BITMAPINFO bmi = {0};
   bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
   bmi.bmiHeader.biWidth = kTilesPerChunk * 2;
   bmi.bmiHeader.biHeight = -((int)kTilesPerChunk * 2);
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

   uint zStart = iChunkZ * kTilesPerChunk;
   uint zEnd = zStart + kTilesPerChunk;
   uint xStart = iChunkX * kTilesPerChunk;
   uint xEnd = xStart + kTilesPerChunk;

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

   cStr tileName;
   Verify(m_pTileSet->GetTileName(m_tile, &tileName) == S_OK);

   tChar szFile[MAX_PATH];
   wsprintf(szFile, "%sAlpha%d%d.bmp", tileName.c_str(), iChunkX, iChunkZ);
#ifdef _DEBUG
   WriteBitmapFile(szFile, &bmi, pBitmapBits);
#endif

   cImageData * pImage = new cImageData;
   if (pImage != NULL)
   {
      if (pImage->Create(bmi.bmiHeader.biWidth, abs(bmi.bmiHeader.biHeight), kPF_RGBA8888, pBitmapBits))
      {
         TextureCreate(pImage, &m_pAlphaMap);
      }

      delete pImage;
      pImage = NULL;
   }

   DeleteObject(hBitmap), hBitmap = NULL;
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
   tSplatBuilders::iterator iter = m_splats.begin();
   tSplatBuilders::iterator end = m_splats.end();
   for (; iter != end; iter++)
   {
      delete *iter;
   }
   m_splats.clear();
}

////////////////////////////////////////

tResult cTerrainChunk::Create(const tTerrainQuads & quads,
                              uint nQuadsX, uint nQuadsZ,
                              uint iChunkX, uint iChunkZ, 
                              IEditorTileSet * pTileSet,
                              cTerrainChunk * * ppChunk)
{
   if (pTileSet == NULL || ppChunk == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<cTerrainChunk> pChunk(new cTerrainChunk);
   if (!pChunk)
   {
      return E_OUTOFMEMORY;
   }

   pChunk->m_vertices.resize(kTilesPerChunk * kTilesPerChunk * 4);
   uint iVert = 0;

   typedef std::map<uint, cSplatBuilder *> tSplatBuilderMap;
   tSplatBuilderMap splatBuilders;

   uint ix = iChunkX * kTilesPerChunk;
   uint iz = iChunkZ * kTilesPerChunk;
   uint cx = kTilesPerChunk, cz = kTilesPerChunk;

   for (uint z = iz; z < (iz + cz); z++)
   {
      for (uint x = ix; x < (ix + cx); x++)
      {
         uint iQuad = (z * nQuadsZ) + x;
         const sTerrainQuad & quad = quads[iQuad];

         cSplatBuilder * pSplatBuilder = NULL;

         if (splatBuilders.find(quad.tile) == splatBuilders.end())
         {
            pSplatBuilder = new cSplatBuilder(pTileSet, quad.tile);
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

         pChunk->m_vertices[iVert+0].uv2 = tVec2((float)(x-ix) / kTilesPerChunk, (float)(z-iz) / kTilesPerChunk);
         pChunk->m_vertices[iVert+1].uv2 = tVec2((float)(x-ix+1) / kTilesPerChunk, (float)(z-iz) / kTilesPerChunk);
         pChunk->m_vertices[iVert+2].uv2 = tVec2((float)(x-ix+1) / kTilesPerChunk, (float)(z-iz+1) / kTilesPerChunk);
         pChunk->m_vertices[iVert+3].uv2 = tVec2((float)(x-ix) / kTilesPerChunk, (float)(z-iz+1) / kTilesPerChunk);

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

   *ppChunk = CTAddRef(pChunk);

   return S_OK;
}

////////////////////////////////////////

void cTerrainChunk::Render(IRenderDevice * pRenderDevice)
{
   // HACK: see comments above
   InitGLExtensions();

   glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);

   glEnable(GL_BLEND);

   glDisableClientState(GL_EDGE_FLAG_ARRAY);
   glDisableClientState(GL_INDEX_ARRAY);
   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_NORMAL_ARRAY);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glDisableClientState(GL_COLOR_ARRAY);

   const byte * pVertexData = (const byte *)(sTerrainVertex *)&m_vertices[0];

   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, sizeof(sTerrainVertex), pVertexData + offsetof(sTerrainVertex, pos));

   glEnableClientState(GL_COLOR_ARRAY);
   glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(sTerrainVertex), pVertexData + offsetof(sTerrainVertex, color));

   pfnglClientActiveTextureARB(GL_TEXTURE0_ARB);
   glTexCoordPointer(2, GL_FLOAT, sizeof(sTerrainVertex), pVertexData + offsetof(sTerrainVertex, uv1));
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);

   pfnglClientActiveTextureARB(GL_TEXTURE1_ARB);
   glTexCoordPointer(2, GL_FLOAT, sizeof(sTerrainVertex), pVertexData + offsetof(sTerrainVertex, uv2));
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);

   tSplatBuilders::iterator iter = m_splats.begin();
   tSplatBuilders::iterator end = m_splats.end();
   for (; iter != end; iter++)
   {
      {
         cAutoIPtr<ITexture> pTexture;
         if ((*iter)->GetTexture(&pTexture) == S_OK)
         {
            HANDLE tex;
            if (pTexture->GetTextureHandle(&tex) == S_OK)
            {
               pfnglActiveTextureARB(GL_TEXTURE0_ARB);
               glBindTexture(GL_TEXTURE_2D, (uint)tex);
               glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
               glEnable(GL_TEXTURE_2D);
            }
         }
      }

      {
         cAutoIPtr<ITexture> pAlphaMap;
         if ((*iter)->GetAlphaMap(&pAlphaMap) == S_OK)
         {
            HANDLE alpha;
            if (pAlphaMap->GetTextureHandle(&alpha) == S_OK)
            {
               pfnglActiveTextureARB(GL_TEXTURE1_ARB);
               glBindTexture(GL_TEXTURE_2D, (uint)alpha);
               glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
               glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
               glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_REPLACE);
               glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
               glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
               glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE1_ARB);
               glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
               glEnable(GL_TEXTURE_2D);
            }
         }
      }

      glDrawElements(GL_TRIANGLES, (*iter)->GetIndexCount(), GL_UNSIGNED_INT, (*iter)->GetIndexPtr());
   }

   glPopAttrib();
}

/////////////////////////////////////////////////////////////////////////////
