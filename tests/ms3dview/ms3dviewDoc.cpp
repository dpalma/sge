/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "ms3dviewDoc.h"

#include "ms3dviewView.h" // TODO HACK: really need to access the view?

#include "resmgr.h"
#include "filespec.h"
#include "filepath.h"
#include "globalobj.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMs3dviewDoc

IMPLEMENT_DYNCREATE(CMs3dviewDoc, CDocument)

BEGIN_MESSAGE_MAP(CMs3dviewDoc, CDocument)
	//{{AFX_MSG_MAP(CMs3dviewDoc)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_OPTIMIZE, OnUpdateToolsOptimize)
	ON_COMMAND(ID_TOOLS_OPTIMIZE, OnToolsOptimize)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_RENDERING, OnUpdateRendering)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMs3dviewDoc construction/destruction

CMs3dviewDoc::CMs3dviewDoc()
{
}

CMs3dviewDoc::~CMs3dviewDoc()
{
}

BOOL CMs3dviewDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

   SafeRelease(m_pMesh);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMs3dviewDoc diagnostics

#ifdef _DEBUG
void CMs3dviewDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMs3dviewDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMs3dviewDoc commands

BOOL CMs3dviewDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
   CMs3dviewView * pMs3dView = NULL;

	POSITION pos = GetFirstViewPosition();
   for (CView * pView = GetNextView(pos); pView != NULL; pView = GetNextView(pos))
   {
      pMs3dView = DYNAMIC_DOWNCAST(CMs3dviewView, pView);
      if (pMs3dView != NULL)
      {
         break;
      }
   }

   if (!pMs3dView)
   {
      DebugMsg("Have no appropriate view from which to get a rendering device\n");
      return FALSE;
   }

   DeleteContents();
   SetModifiedFlag();  // dirty during de-serialize

   UseGlobal(ResourceManager);
   pResourceManager->AddSearchPath(cFileSpec(lpszPathName).GetPath().GetPath());

   Assert(!m_pMesh);
   m_pMesh = new cMs3dMesh;
   if (m_pMesh->Load(lpszPathName, pMs3dView->AccessRenderDevice(), pResourceManager) != S_OK)
   {
   	DeleteContents();
      return FALSE;
   }

   SetModifiedFlag(FALSE);

   return TRUE;
}

void CMs3dviewDoc::DeleteContents() 
{
   SafeRelease(m_pMesh);
	
	CDocument::DeleteContents();
}

void CMs3dviewDoc::OnUpdateRendering(CCmdUI * pCmdUI)
{
   UINT stringId = (m_pMesh != NULL) && m_pMesh->IsRenderingSoftware() ? IDS_RENDERING_SOFTWARE : IDS_RENDERING_VERTEX_PROGRAM;
   CString string;
   VERIFY(string.LoadString(stringId));
   pCmdUI->SetText(string);
}

void CMs3dviewDoc::OnUpdateToolsOptimize(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!!m_pMesh);
}

void CMs3dviewDoc::OnToolsOptimize() 
{
	// TODO: Add your command handler code here
	
}
