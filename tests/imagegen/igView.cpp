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

   if (m_bitmap.GetSafeHandle() != NULL)
   {
      CRect rect;
      GetClientRect(rect);

      BITMAP bm = {0};
      m_bitmap.GetBitmap(&bm);

      CDC memDC;
      if (memDC.CreateCompatibleDC(pDC))
      {
         CBitmap * pOldBitmap = memDC.SelectObject(&m_bitmap);

         pDC->BitBlt(rect.left, rect.top, bm.bmWidth, bm.bmHeight, &memDC, 0, 0, SRCCOPY);

         memDC.SelectObject(pOldBitmap);
      }
   }
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
   cImageGenDoc * pDoc = GetDocument();
   ASSERT_VALID(pDoc);

   cAutoIPtr<IImage> pImage(CTAddRef(pDoc->AccessImage()));
   if (!!pImage)
   {
      SetScrollSizes(MM_TEXT, CSize(pImage->GetWidth(), pImage->GetHeight()));
   }

   CView::OnInitialUpdate();
}

////////////////////////////////////////

void cImageGenView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
   cImageGenDoc * pDoc = GetDocument();
   ASSERT_VALID(pDoc);

   m_bitmap.DeleteObject();

   cAutoIPtr<IImage> pImage(CTAddRef(pDoc->AccessImage()));
   if (!!pImage)
   {
      HBITMAP hbm = NULL;
      ImageToWindowsBitmap(pImage, &hbm);
      m_bitmap.Attach(hbm);
   }

   Invalidate();
}

///////////////////////////////////////////////////////////////////////////////
