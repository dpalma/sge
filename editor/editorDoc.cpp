/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorDoc.h"
#include "tiledground.h"
#include "heightmap.h"
#include "editorapi.h"

#include "resmgr.h"
#include "readwriteapi.h"
#include "filespec.h"
#include "filepath.h"
#include "globalobj.h"
#include "techmath.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
 : m_pGround(NULL),
   m_pHeightMap(NULL)
{
	// TODO: add one-time construction code here

}

cEditorDoc::~cEditorDoc()
{
   Assert(m_pGround == NULL);
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
   delete m_pGround, m_pGround = NULL;

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

      m_pGround = new cTiledGround();
      if (m_pGround != NULL)
      {
         if (m_pGround->Init(mapSettings.GetXDimension(),
                             mapSettings.GetZDimension(),
                             pTileSet, 0,
                             pHeightMap))
         {
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

bool cEditorDoc::GetDimensions(uint * pxd, uint * pzd) const
{
   if (m_pGround != NULL)
   {
      if (pxd != NULL)
      {
         *pxd = m_pGround->GetXDimension();
      }

      if (pzd != NULL)
      {
         *pzd = m_pGround->GetZDimension();
      }

      return true;
   }

   return false;
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
   delete m_pGround, m_pGround = NULL;
   delete m_pHeightMap, m_pHeightMap = NULL;

	CDocument::DeleteContents();
}
