/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorDoc.h"
#include "heightmap.h"
#include "editorapi.h"

#include "render.h"
#include "material.h"

#include "resmgr.h"
#include "readwriteapi.h"
#include "filespec.h"
#include "filepath.h"
#include "globalobj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

sVertexElement g_mapVertexDecl[] =
{
   { kVDU_TexCoord, kVDT_Float2, 0 },
   { kVDU_Color, kVDT_Float3 },
   { kVDU_Position, kVDT_Float3 },
};

uint g_nMapVertexMembers = _countof(g_mapVertexDecl);

/////////////////////////////////////////////////////////////////////////////
// cEditorDoc

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
   m_xDim(0),
   m_zDim(0)
{
	// TODO: add one-time construction code here

}

cEditorDoc::~cEditorDoc()
{
   Assert(m_pHeightMap == NULL);
}

BOOL cEditorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

   Assert(AccessEditorApp() != NULL);

   delete m_pHeightMap, m_pHeightMap = NULL;

   m_xDim = 0;
   m_zDim = 0;

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
      pEditorTileManager->GetTileSet(mapSettings.GetTileSet(), &pTileSet);

      m_xDim = mapSettings.GetXDimension();
      m_zDim = mapSettings.GetZDimension();

      if (InitializeVertices(m_xDim, m_zDim, pTileSet, 0, pHeightMap))
      {
         bResult = TRUE;
      }
//      m_pGround = new cTiledGround();
//      if (m_pGround != NULL)
//      {
//         if (m_pGround->Init(m_xDim, m_zDim, pTileSet, 0, pHeightMap))
//         {
//            bResult = TRUE;
//         }
//      }
   }

	return bResult;
}

bool cEditorDoc::InitializeVertices(uint xDim, uint zDim, IEditorTileSet * pTileSet,
                                    uint defaultTile, cHeightMap * pHeightMap)
{
   Assert(pTileSet != NULL);

   cAutoIPtr<IEditorTile> pTile;
   if (pTileSet->GetTile(defaultTile, &pTile) != S_OK)
   {
      return false;
   }

   Assert(!m_pMaterial);
   if (pTileSet->GetMaterial(&m_pMaterial) != S_OK)
   {
      return false;
   }

   static const int kStepSize = 16;
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

   uint nQuads = xDim * zDim;

   m_vertices.resize(nQuads * 4);

   int index = 0;

   float z1 = 0;
   float z2 = kStepSize;

   for (int iz = 0; iz < zDim; iz++, z1 += kStepSize, z2 += kStepSize)
   {
      float x1 = 0;
      float x2 = kStepSize;

      for (int ix = 0; ix < xDim; ix++, x1 += kStepSize, x2 += kStepSize)
      {
         uint tile = rand() & (nTileImages - 1);
         uint tileRow = tile / pTile->GetHorizontalImageCount();
         uint tileCol = tile % pTile->GetHorizontalImageCount();

#define Height(xx,zz) ((pHeightMap != NULL) ? pHeightMap->Height(Round(xx),Round(zz)) : 0)

         m_vertices[index].uv1 = tVec2(tileCol * tileTexWidth, tileRow * tileTexHeight);
         m_vertices[index].rgb = tVec3(kRed,kGreen,kBlue);
         m_vertices[index++].pos = tVec3(x1, Height(x1,z1), z1);

         m_vertices[index].uv1 = tVec2((tileCol + 1) * tileTexWidth, tileRow * tileTexHeight);
         m_vertices[index].rgb = tVec3(kRed,kGreen,kBlue);
         m_vertices[index++].pos = tVec3(x2, Height(x2,z1), z1);

         m_vertices[index].uv1 = tVec2((tileCol + 1) * tileTexWidth, (tileRow + 1) * tileTexHeight);
         m_vertices[index].rgb = tVec3(kRed,kGreen,kBlue);
         m_vertices[index++].pos = tVec3(x2, Height(x2,z2), z2);

         m_vertices[index].uv1 = tVec2(tileCol * tileTexWidth, (tileRow + 1) * tileTexHeight);
         m_vertices[index].rgb = tVec3(kRed,kGreen,kBlue);
         m_vertices[index++].pos = tVec3(x1, Height(x1,z2), z2);

#undef Height
      }
   }

   return true;
}

/////////////////////////////////////////////////////////////////////////////
// cEditorDoc serialization

void cEditorDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
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

float cEditorDoc::GetElevation(float nx, float nz) const
{
   Assert(m_pHeightMap != NULL);
   uint size = m_pHeightMap->GetSize() - 1;
   return m_pHeightMap->Height(Round(nx * size), Round(nz * size));
}

void cEditorDoc::GetMapDimensions(uint * pXDim, uint * pZDim) const
{
   if (pXDim != NULL)
   {
      *pXDim = m_xDim;
   }

   if (pZDim != NULL)
   {
      *pZDim = m_zDim;
   }
}

static const int kStepSize = 16; // TODO HACK hard-coded constant

void cEditorDoc::GetMapExtents(uint * pXExt, uint * pZExt) const
{
   if (pXExt != NULL)
   {
      *pXExt = kStepSize * m_xDim;
   }

   if (pZExt != NULL)
   {
      *pZExt = kStepSize * m_zDim;
   }
}

/////////////////////////////////////////////////////////////////////////////
// cEditorDoc commands

BOOL cEditorDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

   cFileSpec file(lpszPathName);

   UseGlobal(ResourceManager);
   pResourceManager->AddSearchPath(file.GetPath().GetPath());

   // TODO

   return TRUE;
}

void cEditorDoc::DeleteContents() 
{
   delete m_pHeightMap, m_pHeightMap = NULL;

   m_xDim = 0;
   m_zDim = 0;

   SafeRelease(m_pMaterial);

   m_vertices.clear();

	CDocument::DeleteContents();
}
