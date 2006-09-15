/////////////////////////////////////////////////////////////////////////////
// $Id: editorDoc.cpp 1344 2006-05-15 17:53:55Z dpalma $

#include "stdhdr.h"

#include "igDoc.h"

#include "resource.h"

#include "engineapi.h"

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
//
// CLASS: cImageGenDoc
//

IMPLEMENT_DYNCREATE(cImageGenDoc, CDocument)

BEGIN_MESSAGE_MAP(cImageGenDoc, CDocument)
	//{{AFX_MSG_MAP(cImageGenDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cImageGenDoc construction/destruction

cImageGenDoc::cImageGenDoc()
{
}

cImageGenDoc::~cImageGenDoc()
{
}

BOOL cImageGenDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// Add reinitialization code here (SDI documents will reuse this document)

   BOOL bResult = FALSE;


   bResult = TRUE;


	return bResult;
}

/////////////////////////////////////////////////////////////////////////////
// cImageGenDoc serialization

void cImageGenDoc::Serialize(CArchive& ar)
{
}

/////////////////////////////////////////////////////////////////////////////
// cImageGenDoc diagnostics

#ifdef _DEBUG
void cImageGenDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void cImageGenDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// cImageGenDoc operations


/////////////////////////////////////////////////////////////////////////////
// cImageGenDoc commands
