// ms3dviewDoc.cpp : implementation of the CMs3dviewDoc class
//

#include "stdafx.h"
#include "ms3dview.h"

#include "ms3dviewDoc.h"

#include "ms3dviewView.h" // @HACK: really need to access the view?

#include "resmgr.h"
#include "ReadWriteAPI.h"
#include "FileSpec.h"

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
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_RENDERING, OnUpdateRendering)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMs3dviewDoc construction/destruction

CMs3dviewDoc::CMs3dviewDoc() : m_pResourceManager(ResourceManagerCreate())
{
	// TODO: add one-time construction code here

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

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CMs3dviewDoc serialization

void CMs3dviewDoc::Serialize(CArchive& ar)
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
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
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
      TRACE0("Have no CMs3dviewView from which to get a rendering device\n");
      return FALSE;
   }

   cAutoIPtr<IReader> pReader = FileCreateReader(cFileSpec(lpszPathName));

   if (!pReader
      || m_mesh.Read(pReader, pMs3dView->AccessRenderDevice(), m_pResourceManager) != S_OK)
   {
      m_mesh.Reset();
      return FALSE;
   }
	
	return TRUE;
}

void CMs3dviewDoc::DeleteContents() 
{
   m_mesh.Reset();
	
	CDocument::DeleteContents();
}

void CMs3dviewDoc::OnUpdateRendering(CCmdUI * pCmdUI)
{
   UINT stringId = m_mesh.IsRenderingSoftware() ? IDS_RENDERING_SOFTWARE : IDS_RENDERING_VERTEX_PROGRAM;
   CString string;
   VERIFY(string.LoadString(stringId));
   pCmdUI->SetText(string);
}
