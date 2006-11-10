/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ShaderTestDoc.h"

#include "engine/entityapi.h"

#include "tech/resourceapi.h"
#include "tech/filespec.h"
#include "tech/filepath.h"
#include "tech/globalobj.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// cShaderTestDoc

IMPLEMENT_DYNCREATE(cShaderTestDoc, CDocument)

BEGIN_MESSAGE_MAP(cShaderTestDoc, CDocument)
	//{{AFX_MSG_MAP(cShaderTestDoc)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_RENDERING, OnUpdateRendering)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cShaderTestDoc construction/destruction

cShaderTestDoc::cShaderTestDoc()
{
}

cShaderTestDoc::~cShaderTestDoc()
{
}

BOOL cShaderTestDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// cShaderTestDoc diagnostics

#ifdef _DEBUG
void cShaderTestDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void cShaderTestDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// cShaderTestDoc commands

BOOL cShaderTestDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
   DeleteContents();
   SetModifiedFlag();  // dirty during de-serialize

   cFileSpec file(lpszPathName);
   cFilePath path;
   file.GetPath(&path);

   UseGlobal(ResourceManager);
   pResourceManager->AddDirectory(path.CStr());

   if (EntityCreateRenderComponent(file.GetFileName(), &m_pRenderComponent) != S_OK)
   {
      DeleteContents();
      return E_FAIL;
   }

   //IModel * pModel = NULL;
   //if (pResourceManager->Load(file.GetFileName(), kRT_Model, NULL, (void**)&pModel) != S_OK)
   //{
   //   DeleteContents();
   //   return E_FAIL;
   //}

   //m_pModel = CTAddRef(pModel);

   SetModifiedFlag(FALSE);

   return TRUE;
}

void cShaderTestDoc::DeleteContents() 
{
	m_model.erase();
   SafeRelease(m_pModel);

   UseGlobal(EntityManager);
   pEntityManager->RemoveAll();

	CDocument::DeleteContents();
}

void cShaderTestDoc::OnUpdateRendering(CCmdUI * pCmdUI)
{
   // TODO
   CString string;
   Verify(string.LoadString(IDS_RENDERING_SOFTWARE));
   pCmdUI->SetText(string);
}
