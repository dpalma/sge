/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorDoc.h"

#include "editorView.h" // @HACK: really need to access the view?

#include "resmgr.h"
#include "readwriteapi.h"
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
{
	// TODO: add one-time construction code here

}

cEditorDoc::~cEditorDoc()
{
}

BOOL cEditorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

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
// cEditorDoc commands

BOOL cEditorDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
   cEditorView * pEditorView = NULL;

	POSITION pos = GetFirstViewPosition();
   for (CView * pView = GetNextView(pos); pView != NULL; pView = GetNextView(pos))
   {
      pEditorView = DYNAMIC_DOWNCAST(cEditorView, pView);
      if (pEditorView != NULL)
      {
         break;
      }
   }

   if (!pEditorView)
   {
      TRACE0("Have no cEditorView from which to get a rendering device\n");
      return FALSE;
   }

   cFileSpec file(lpszPathName);

   UseGlobal(ResourceManager);
   pResourceManager->AddSearchPath(file.GetPath().GetPath());

   // TODO

   return TRUE;
}

void cEditorDoc::DeleteContents() 
{
   // TODO
	
	CDocument::DeleteContents();
}
