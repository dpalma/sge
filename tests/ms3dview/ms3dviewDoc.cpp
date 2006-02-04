/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "ms3dviewDoc.h"

#include "entityapi.h"

#include "resourceapi.h"
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
// c3dmodelDoc

IMPLEMENT_DYNCREATE(c3dmodelDoc, CDocument)

BEGIN_MESSAGE_MAP(c3dmodelDoc, CDocument)
	//{{AFX_MSG_MAP(c3dmodelDoc)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_RENDERING, OnUpdateRendering)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// c3dmodelDoc construction/destruction

c3dmodelDoc::c3dmodelDoc()
{
}

c3dmodelDoc::~c3dmodelDoc()
{
}

BOOL c3dmodelDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// c3dmodelDoc diagnostics

#ifdef _DEBUG
void c3dmodelDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void c3dmodelDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// c3dmodelDoc commands

BOOL c3dmodelDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
   DeleteContents();
   SetModifiedFlag();  // dirty during de-serialize

   cFileSpec file(lpszPathName);

   UseGlobal(ResourceManager);
   pResourceManager->AddDirectory(file.GetPath().c_str());

   Assert(!m_pModel);
   if (pResourceManager->Load(file.GetFileName(), kRT_Model, NULL, (void**)&m_pModel) != S_OK)
   {
      DeleteContents();
      return E_FAIL;
   }

   UseGlobal(EntityManager);
   pEntityManager->SpawnEntity(file.GetFileName(), tVec3(0,0,0));

   SetModifiedFlag(FALSE);

   return TRUE;
}

void c3dmodelDoc::DeleteContents() 
{
	m_model.erase();
   m_pModel = NULL;

   UseGlobal(EntityManager);
   pEntityManager->RemoveAll();

	CDocument::DeleteContents();
}

void c3dmodelDoc::OnUpdateRendering(CCmdUI * pCmdUI)
{
   // TODO
   CString string;
   Verify(string.LoadString(IDS_RENDERING_SOFTWARE));
   pCmdUI->SetText(string);
}
