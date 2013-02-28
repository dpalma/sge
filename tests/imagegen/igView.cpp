/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "igView.h"
#include "igDoc.h"


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cImageGenView
//

////////////////////////////////////////

cImageGenView::cImageGenView(const cImageGenDoc * pDoc)
 : m_pDoc(pDoc)
{
}

////////////////////////////////////////

cImageGenView::~cImageGenView()
{
}

////////////////////////////////////////

void cImageGenView::DoPaint(WTL::CDCHandle dc)
{
   if (!m_bitmap.IsNull())
   {
      WTL::CRect rect;
      GetClientRect(rect);

      BITMAP bm = {0};
      m_bitmap.GetBitmap(&bm);

      WTL::CDC memDC;
      if (memDC.CreateCompatibleDC(dc))
      {
         WTL::CBitmapHandle oldBitmap = memDC.SelectBitmap(m_bitmap);

         dc.BitBlt(rect.left, rect.top, bm.bmWidth, bm.bmHeight, memDC, 0, 0, SRCCOPY);

         memDC.SelectBitmap(oldBitmap);
      }
   }
}

////////////////////////////////////////

void cImageGenView::Update() 
{
   if (!m_bitmap.IsNull())
      m_bitmap.DeleteObject();

   const IImage * pImage = GetDocument()->AccessImage();
   if (pImage != NULL)
   {
      HBITMAP hbm = NULL;
      ImageToWindowsBitmap(const_cast<IImage*>(pImage), &hbm);
      m_bitmap.Attach(hbm);

	   SetScrollOffset(0, 0, FALSE);
      SetScrollSize(WTL::CSize(pImage->GetWidth(), pImage->GetHeight()));
   }

   Invalidate();
}

///////////////////////////////////////////////////////////////////////////////
