/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "terrainrender.h"
#include "editorapi.h"

#include "materialapi.h"
#include "renderapi.h"
#include "textureapi.h"

#include <algorithm>
#include <GL/gl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int kTilesPerChunk = 32;

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
   m_bTerrainChanged = true;
   
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
         if (cTerrainChunk::Create(ix * kTilesPerChunk, iz * kTilesPerChunk,
            kTilesPerChunk, kTilesPerChunk, m_pModel->GetTerrainQuads(),
            nTilesX, nTilesZ, pTileSet, &pChunk) == S_OK)
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

cSplatBuilder::cSplatBuilder(ITexture * pTexture)
 : m_pTexture(CTAddRef(pTexture))
{
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

tResult cTerrainChunk::Create(uint ix, uint iz, uint cx, uint cz,
                              const tTerrainQuads & quads,
                              uint nQuadsX, uint nQuadsZ,
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

   pChunk->m_vertices.resize(cx * cz * 4);
   uint iVert = 0;

   typedef std::map<uint, cSplatBuilder *> tSplatBuilderMap;
   tSplatBuilderMap splatBuilders;

   for (uint z = iz; z < (iz + cz); z++)
   {
      uint zPrev = (z > iz) ? (z - 1) : (iz + cz - 1);
      uint zNext = (z < (iz + cz - 1)) ? (z + 1) : iz;

      for (uint x = ix; x < (ix + cx); x++)
      {
         uint iQuad = (z * nQuadsZ) + x;
         const sTerrainQuad & quad = quads[iQuad];

         cSplatBuilder * pSplatBuilder = NULL;

         if (splatBuilders.find(quad.tile) == splatBuilders.end())
         {
            cAutoIPtr<ITexture> pTexture;
            if (pTileSet->GetTileTexture(quad.tile, &pTexture) == S_OK)
            {
               pSplatBuilder = new cSplatBuilder(pTexture);
               if (pSplatBuilder != NULL)
               {
                  splatBuilders[quad.tile] = pSplatBuilder;
               }
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
      pChunk->m_splats.push_back(iter->second);
   }
   splatBuilders.clear();

   *ppChunk = CTAddRef(pChunk);

   return S_OK;
}

////////////////////////////////////////

void cTerrainChunk::Render(IRenderDevice * pRenderDevice)
{
   glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);

   glDisableClientState(GL_EDGE_FLAG_ARRAY);
   glDisableClientState(GL_INDEX_ARRAY);
   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_NORMAL_ARRAY);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glDisableClientState(GL_COLOR_ARRAY);

   const byte * pVertexData = (const byte *)(sTerrainVertex *)&m_vertices[0];

   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, sizeof(sTerrainVertex), pVertexData + sizeof(tVec2) + sizeof(uint32));

   glEnableClientState(GL_COLOR_ARRAY);
   glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(sTerrainVertex), pVertexData + sizeof(tVec2));

   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//   glClientActiveTextureARB(GL_TEXTURE0_ARB);
   glTexCoordPointer(2, GL_FLOAT, sizeof(sTerrainVertex), pVertexData);

   tSplatBuilders::iterator iter = m_splats.begin();
   tSplatBuilders::iterator end = m_splats.end();
   for (; iter != end; iter++)
   {
      cAutoIPtr<ITexture> pTexture;
      if ((*iter)->GetTexture(&pTexture) == S_OK)
      {
         HANDLE tex;
         if (pTexture->GetTextureHandle(&tex) == S_OK)
         {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, (uint)tex);
         }
      }

      glDrawElements(GL_TRIANGLES, (*iter)->GetIndexCount(), GL_UNSIGNED_INT, (*iter)->GetIndexPtr());
   }

   glPopAttrib();
}

/////////////////////////////////////////////////////////////////////////////
