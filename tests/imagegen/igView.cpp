/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "igDoc.h"
#include "igView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cImageGenView
//

////////////////////////////////////////

IMPLEMENT_DYNCREATE(cImageGenView, CScrollView)

////////////////////////////////////////

BEGIN_MESSAGE_MAP(cImageGenView, CScrollView)
	//{{AFX_MSG_MAP(cImageGenView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cImageGenView construction/destruction

////////////////////////////////////////

cImageGenView::cImageGenView()
{
}

////////////////////////////////////////

cImageGenView::~cImageGenView()
{
}

/////////////////////////////////////////////////////////////////////////////
// cImageGenView operations


/////////////////////////////////////////////////////////////////////////////
// cImageGenView drawing

void cImageGenView::OnDraw(CDC * pDC)
{
	cImageGenDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);
}

/////////////////////////////////////////////////////////////////////////////
// cImageGenView diagnostics

#ifdef _DEBUG
////////////////////////////////////////

void cImageGenView::AssertValid() const
{
	CScrollView::AssertValid();
}

////////////////////////////////////////

void cImageGenView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

////////////////////////////////////////

cImageGenDoc* cImageGenView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(cImageGenDoc)));
	return (cImageGenDoc*)m_pDocument;
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// cImageGenView message handlers

////////////////////////////////////////

int cImageGenView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

////////////////////////////////////////

void cImageGenView::OnDestroy() 
{
	CScrollView::OnDestroy();

}

////////////////////////////////////////

void cImageGenView::OnSize(UINT nType, int cx, int cy) 
{
	CScrollView::OnSize(nType, cx, cy);

}

////////////////////////////////////////

void cImageGenView::OnInitialUpdate() 
{
   CRect rect;
   GetClientRect(rect);
   SetScaleToFitSize(rect.Size());

   cImageGenDoc * pDoc = GetDocument();
   ASSERT_VALID(pDoc);

   CView::OnInitialUpdate();
}

////////////////////////////////////////

void cImageGenView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
   cImageGenDoc * pDoc = GetDocument();
   ASSERT_VALID(pDoc);

}

///////////////////////////////////////////////////////////////////////////////
