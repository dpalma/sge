/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorDoc.h"
#include "heightmap.h"
#include "editorapi.h"

#include "render.h"
#include "material.h"

#include "readwriteapi.h"
#include "filespec.h"
#include "filepath.h"
#include "globalobj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int kTilesPerBlock = 6;

/////////////////////////////////////////////////////////////////////////////

sVertexElement g_mapVertexDecl[] =
{
   { kVDU_TexCoord, kVDT_Float2, 0 },
   { kVDU_Color, kVDT_Float3 },
   { kVDU_Position, kVDT_Float3 },
};

uint g_nMapVertexMembers = _countof(g_mapVertexDecl);

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrain
//

////////////////////////////////////////

cTerrain::cTerrain()
 : m_xDim(0),
   m_zDim(0),
   m_tileSize(0),
   m_xBlocks(0),
   m_zBlocks(0)
{
}

////////////////////////////////////////

cTerrain::~cTerrain()
{
}

////////////////////////////////////////

bool cTerrain::Create(uint xDim, uint zDim, int stepSize,
                      IEditorTileSet * pTileSet, uint defaultTile,
                      cHeightMap * pHeightMap)
{
   InitializeVertices(xDim, zDim, stepSize, pHeightMap);

   Assert(pTileSet != NULL);

   cAutoIPtr<IEditorTile> pTile;
   if (pTileSet->GetTile(defaultTile, &pTile) != S_OK)
   {
      return false;
   }

   static const float kRed = 0.75f, kGreen = 0.75f, kBlue = 0.75f;

   uint nTileImages = pTile->GetHorizontalImageCount() * pTile->GetVerticalImageCount();

   Assert(IsPowerOfTwo(nTileImages));

   if (!IsPowerOfTwo(nTileImages))
   {
      DebugMsg("Number of variations in a tile must be a power of two\n");
      return false;
   }

   float tileTexWidth = 1.0f / pTile->GetHorizontalImageCount();
   float tileTexHeight = 1.0f / pTile->GetVerticalImageCount();

   int index = 0;

   float z1 = 0;
   float z2 = stepSize;

   for (int iz = 0; iz < zDim; iz++, z1 += stepSize, z2 += stepSize)
   {
      float x1 = 0;
      float x2 = stepSize;

      for (int ix = 0; ix < xDim; ix++, x1 += stepSize, x2 += stepSize)
      {
         uint tile = rand() & (nTileImages - 1);
         uint tileRow = tile / pTile->GetHorizontalImageCount();
         uint tileCol = tile % pTile->GetHorizontalImageCount();

         m_vertices[index].uv1 = tVec2(tileCol * tileTexWidth, tileRow * tileTexHeight);
         m_vertices[index++].rgb = tVec3(kRed,kGreen,kBlue);

         m_vertices[index].uv1 = tVec2((tileCol + 1) * tileTexWidth, tileRow * tileTexHeight);
         m_vertices[index++].rgb = tVec3(kRed,kGreen,kBlue);

         m_vertices[index].uv1 = tVec2((tileCol + 1) * tileTexWidth, (tileRow + 1) * tileTexHeight);
         m_vertices[index++].rgb = tVec3(kRed,kGreen,kBlue);

         m_vertices[index].uv1 = tVec2(tileCol * tileTexWidth, (tileRow + 1) * tileTexHeight);
         m_vertices[index++].rgb = tVec3(kRed,kGreen,kBlue);
      }
   }

   m_tileSize = stepSize;

   if (xDim > kTilesPerBlock)
   {
      m_xBlocks = xDim / kTilesPerBlock;
   }

   if (zDim > kTilesPerBlock)
   {
      m_zBlocks = zDim / kTilesPerBlock;
   }

   return true;
}

////////////////////////////////////////

void cTerrain::GetDimensions(uint * pxd, uint * pzd) const
{
   if (pxd != NULL)
   {
      *pxd = m_xDim;
   }

   if (pzd != NULL)
   {
      *pzd = m_zDim;
   }
}

////////////////////////////////////////

void cTerrain::GetExtents(uint * px, uint * pz) const
{
   if (px != NULL)
   {
      *px = m_tileSize * m_xDim;
   }

   if (pz != NULL)
   {
      *pz = m_tileSize * m_zDim;
   }
}

////////////////////////////////////////

const sMapVertex * cTerrain::GetVertexPointer() const
{
   return &m_vertices[0];
}

////////////////////////////////////////

size_t cTerrain::GetVertexCount() const
{
   return m_vertices.size();
}

////////////////////////////////////////

void cTerrain::Render(IRenderDevice * pRenderDevice)
{
}

////////////////////////////////////////

void cTerrain::InitializeVertices(uint xDim, uint zDim, int stepSize, cHeightMap * pHeightMap)
{
   uint nQuads = xDim * zDim;

   m_vertices.resize(nQuads * 4);

   int index = 0;

   float z1 = 0;
   float z2 = stepSize;

   for (int iz = 0; iz < zDim; iz++, z1 += stepSize, z2 += stepSize)
   {
      float x1 = 0;
      float x2 = stepSize;

      for (int ix = 0; ix < xDim; ix++, x1 += stepSize, x2 += stepSize)
      {
#define Height(xx,zz) ((pHeightMap != NULL) ? pHeightMap->Height(Round(xx),Round(zz)) : 0)

         m_vertices[index++].pos = tVec3(x1, Height(x1,z1), z1);
         m_vertices[index++].pos = tVec3(x2, Height(x2,z1), z1);
         m_vertices[index++].pos = tVec3(x2, Height(x2,z2), z2);
         m_vertices[index++].pos = tVec3(x1, Height(x1,z2), z2);

#undef Height
      }
   }

   m_xDim = xDim;
   m_zDim = zDim;
}

////////////////////////////////////////

bool cTerrain::CreateTerrainBlocks()
{
   return false;
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorDoc
//

IMPLEMENT_DYNCREATE(cEditorDoc, CDocument)

BEGIN_MESSAGE_MAP(cEditorDoc, CDocument)
	//{{AFX_MSG_MAP(cEditorDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cEditorDoc construction/destruction

cEditorDoc::cEditorDoc()
 : m_pHeightMap(NULL),
   m_pTerrain(NULL)
{
}

cEditorDoc::~cEditorDoc()
{
   Assert(m_pHeightMap == NULL);
}

BOOL cEditorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// Add reinitialization code here (SDI documents will reuse this document)

   Assert(AccessEditorApp() != NULL);

   SafeRelease(m_pMaterial);

   delete m_pTerrain, m_pTerrain = NULL;

   delete m_pHeightMap, m_pHeightMap = NULL;

   BOOL bResult = FALSE;

   cMapSettings mapSettings;

   if (AccessEditorApp()->GetMapSettings(&mapSettings) == S_OK)
   {
      cHeightMap * pHeightMap = NULL;
      if (mapSettings.GetHeightData() == kHeightData_HeightMap)
      {
         pHeightMap = new cHeightMap(0.25f); // TODO: hard-coded height scale
         if (pHeightMap != NULL)
         {
            if (!pHeightMap->Load(mapSettings.GetHeightMap()))
            {
               delete pHeightMap;
               pHeightMap = NULL;
            }
         }
      }

      m_pHeightMap = pHeightMap;

      cAutoIPtr<IEditorTileSet> pTileSet;

      UseGlobal(EditorTileManager);
      if (pEditorTileManager->GetTileSet(mapSettings.GetTileSet(), &pTileSet) == S_OK)
      {
         pEditorTileManager->SetDefaultTileSet(mapSettings.GetTileSet());
      }

      Assert(m_pTerrain == NULL);
      m_pTerrain = new cTerrain;
      if (m_pTerrain != NULL)
      {
         if (m_pTerrain->Create(mapSettings.GetXDimension(),
                                mapSettings.GetZDimension(),
                                kDefaultStepSize,
                                pTileSet, 0, pHeightMap))
         {
            Assert(!m_pMaterial);
            if (pTileSet->GetMaterial(&m_pMaterial) != S_OK)
            {
            }

            bResult = TRUE;
         }
      }
   }

	return bResult;
}

/////////////////////////////////////////////////////////////////////////////
// cEditorDoc serialization

void cEditorDoc::Serialize(CArchive& ar)
{
   Assert(!"This method should never be called");
}

/////////////////////////////////////////////////////////////////////////////
// cEditorDoc diagnostics

#ifdef _DEBUG
void cEditorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void cEditorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// cEditorDoc operations

void cEditorDoc::GetMapDimensions(uint * pXDim, uint * pZDim) const
{
   if (m_pTerrain != NULL)
   {
      m_pTerrain->GetDimensions(pXDim, pZDim);
   }
}

void cEditorDoc::GetMapExtents(uint * pXExt, uint * pZExt) const
{
   if (m_pTerrain != NULL)
   {
      m_pTerrain->GetExtents(pXExt, pZExt);
   }
}

/////////////////////////////////////////////////////////////////////////////
// cEditorDoc commands

BOOL cEditorDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

   // TODO

   return TRUE;
}

BOOL cEditorDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	// TODO: Add your specialized code here and/or call the base class

	return CDocument::OnSaveDocument(lpszPathName);
}

void cEditorDoc::DeleteContents() 
{
   delete m_pHeightMap, m_pHeightMap = NULL;

   delete m_pTerrain, m_pTerrain = NULL;

   SafeRelease(m_pMaterial);

	CDocument::DeleteContents();
}
