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

   cMapSettings mapSettings;

   tResult result = AccessEditorApp()->GetMapSettings(&mapSettings);
   if (result == S_OK)
   {
      DebugMsg3("Create %d x %d map with tileset \"%s\"\n",
         mapSettings.GetXDimension(),
         mapSettings.GetZDimension(),
         mapSettings.GetTileSet());
   }
   else
   {
      ErrorMsg("Error getting map settings\n");
      return FALSE;
   }

	return TRUE;
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

bool cEditorDoc::SetTerrain(IRenderDevice * pRenderDevice, 
                            const char * pszHeightData, 
                            float heightScale, 
                            const char * pszTexture)
{
   Assert(pszHeightData != NULL);
   Assert(m_pHeightMap == NULL);
   Assert(m_pGround == NULL);

   m_pHeightMap = new cHeightMap(heightScale);
   if (m_pHeightMap != NULL)
   {
      if (m_pHeightMap->Load(pszHeightData))
      {
         m_pGround = new cTiledGround();
         if (m_pGround != NULL)
         {
            if (m_pGround->SetTexture(pszTexture))
            {
               if (m_pGround->Init(64, 64, m_pHeightMap))
               {
                  return m_pGround->CreateBuffers(pRenderDevice);
               }
            }
         }
      }
   }

   return false;
}

float cEditorDoc::GetElevation(float nx, float nz) const
{
   Assert(m_pHeightMap != NULL);
   uint size = m_pHeightMap->GetSize() - 1;
   return m_pHeightMap->Height(Round(nx * size), Round(nz * size));
}

bool cEditorDoc::GetDimensions(uint * pxd, uint * pzd) const
{
   if (m_pHeightMap != NULL)
   {
      uint size = m_pHeightMap->GetSize();
      if (pxd != NULL)
      {
         *pxd = size;
      }
      if (pzd != NULL)
      {
         *pzd = size;
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

 //  cEditorView * pEditorView = NULL;

	//POSITION pos = GetFirstViewPosition();
 //  for (CView * pView = GetNextView(pos); pView != NULL; pView = GetNextView(pos))
 //  {
 //     pEditorView = DYNAMIC_DOWNCAST(cEditorView, pView);
 //     if (pEditorView != NULL)
 //     {
 //        break;
 //     }
 //  }

 //  if (!pEditorView)
 //  {
 //     TRACE0("Have no cEditorView from which to get a rendering device\n");
 //     return FALSE;
 //  }

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
