///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "historywnd.h"

#include <afxole.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int kDefaultMaxEntries = 1000;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTextDataSource
//

class cTextDataSource : public COleDataSource
{
public:
   bool SetText(const tChar * pszText);
};

///////////////////////////////////////

bool cTextDataSource::SetText(const tChar * pszText)
{
   if (m_nSize > 0)
   {
      DebugMsg1("Already set text for OLE data source at 0x%08X\n", (long)this);
      return false;
   }

   Assert(AfxIsValidString(pszText, 1));

   HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, _tcslen(pszText) + 1);
   if (hMem == NULL)
   {
      return false;
   }

   char * pMem = (char *)GlobalLock(hMem);
   if (pMem == NULL)
   {
      GlobalFree(hMem);
      return false;
   }

   _tcscpy(pMem, pszText);
   GlobalUnlock(hMem);

   CacheGlobalData(CF_TEXT, hMem);

   return true;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cHistoryWnd
//

///////////////////////////////////////

IMPLEMENT_DYNCREATE(cHistoryWnd, CScrollWnd)

BEGIN_MESSAGE_MAP(cHistoryWnd, CScrollWnd)
   //{{AFX_MSG_MAP(cHistoryWnd)
   ON_WM_PAINT()
   ON_WM_ERASEBKGND()
   ON_WM_CREATE()
   ON_WM_VSCROLL()
   ON_WM_CONTEXTMENU()
   ON_WM_LBUTTONDOWN()
   ON_WM_LBUTTONUP()
   ON_WM_MOUSEMOVE()
   ON_WM_SETCURSOR()
   ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
   ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
   ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
   ON_WM_DESTROY()
   //}}AFX_MSG_MAP
   ON_MESSAGE(WM_SETFONT, OnSetFont)
END_MESSAGE_MAP()

///////////////////////////////////////

cHistoryWnd::cHistoryWnd()
 : m_textColor(GetSysColor(COLOR_WINDOWTEXT)),
   m_bkColor(GetSysColor(COLOR_WINDOW)),
   m_nMaxEntries(kDefaultMaxEntries),
   m_nCharHeight(0),
   m_nAvgCharWidth(0),
   m_nMaxEntryLen(0),
   m_maxEntrySize(0),
   m_bAtEnd(TRUE),
   m_nAddsSinceLastPaint(0),
   m_pSelAnchor(NULL),
   m_pSelDrag(NULL)
{
   m_entries.reserve(m_nMaxEntries);
}

///////////////////////////////////////

cHistoryWnd::~cHistoryWnd()
{
   ASSERT_VALID(this);
   Clear();
}

///////////////////////////////////////

void cHistoryWnd::AddLine(LPCTSTR pszLine, COLORREF textColor)
{
   if (textColor == CLR_INVALID)
   {
      textColor = GetTextColor();
   }
   cEntry * pEntry = new cEntry(pszLine, textColor);
   m_entries.push_back(pEntry);

   if (pEntry->GetTextLen() > m_nMaxEntryLen)
   {
      m_nMaxEntryLen = pEntry->GetTextLen();
   }

   CDC * pDC = GetDC();
   CFont * pOldFont = pDC->SelectObject(&m_font);
   CSize s = pDC->GetTextExtent(pszLine, lstrlen(pszLine));
   if (s.cx > m_maxEntrySize)
   {
      m_maxEntrySize = s.cx;
   }
   pDC->SelectObject(pOldFont);
   ReleaseDC(pDC);

   EnforceMaxEntries();

   UpdateScrollInfo();

   // update the scroll position only if it's already at the very end
   if (m_bAtEnd)
   {
      CPoint ptScroll = GetScrollPosition();
      ptScroll.y = m_entries.size() * m_nCharHeight;
      ScrollToPosition(ptScroll);
   }

   if (++m_nAddsSinceLastPaint > 15)
   {
      UpdateWindow();
   }
   else
   {
      Invalidate();
   }
}

///////////////////////////////////////

void cHistoryWnd::Clear()
{
   ClearSel();

   m_nMaxEntryLen = 0;
   m_maxEntrySize = 0;

   tEntries::iterator iter;
   for (iter = m_entries.begin(); iter != m_entries.end(); iter++)
   {
      delete *iter;
   }
   m_entries.clear();

   // Normally, when we clear the contents we want to update the scrollbar
   // calibration. However, we don't want to do this if the window has been
   // destroyed (i.e., Clear() is being called from the destructor).
   if (IsWindow(GetSafeHwnd()))
   {
      m_bAtEnd = true;
      UpdateScrollInfo();
      Invalidate();
   }
}

///////////////////////////////////////

void cHistoryWnd::SetMaxEntries(int max)
{
   m_nMaxEntries = max;
   m_entries.reserve(max);
   EnforceMaxEntries();
}

///////////////////////////////////////

void cHistoryWnd::SetFont(CFont * pFont, BOOL bRedraw /*= TRUE*/)
{
   ASSERT_VALID(pFont);

   m_font.DeleteObject();

   LOGFONT lf;
   VERIFY(pFont->GetLogFont(&lf));
   VERIFY(m_font.CreateFontIndirect(&lf));

   UpdateFontMetrics();
   UpdateScrollInfo();

   if (bRedraw)
   {
      Invalidate();
   }
}

///////////////////////////////////////

bool cHistoryWnd::CopySelection()
{
   bool result = false; // assume failure

   CString sel;
   if (GetSelection(&sel) && sel.GetLength() > 0)
   {
      cTextDataSource tds;
      if (result = tds.SetText(sel))
      {
         tds.SetClipboard();
      }
#if 0
      if (OpenClipboard())
      {
         HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, sel.GetLength() + 1);
         if (hMem)
         {
            char* pData = (char*)GlobalLock(hMem);
            Assert(pData != NULL);
            strcpy(pData, (LPCTSTR)sel);
            GlobalUnlock(hMem);

            result = (SetClipboardData(CF_TEXT, hMem) != NULL);

            if (!result)
            {
               WarnMsg("SetClipboardData() failed. This is highly unusual.\n");
               GlobalFree(hMem);
            }
         }

         Verify(CloseClipboard());
      }
#endif
   }

   return result;
}

///////////////////////////////////////

bool cHistoryWnd::GetSelection(CString * pSel)
{
   if (m_startSel.iEntry > -1 && m_endSel.iEntry > -1)
   {
      Assert(pSel != NULL);
      pSel->Empty();

      for (int i = m_startSel.iEntry; i < m_entries.size(); i++)
      {
         int iStart, iEnd;

         if (i == m_startSel.iEntry)
            iStart = m_startSel.iCharHit;
         else
            iStart = 0;

         if (i == m_endSel.iEntry)
            iEnd = m_endSel.iCharHit;
         else
            iEnd = m_entries[i]->GetTextLen();

         CString s = m_entries[i]->GetText().Mid(iStart, iEnd - iStart);

         if (pSel->GetLength())
            *pSel += '\n';
         *pSel += s;

         // if we just did the last entry in the selection, break out of the loop
         if (i == m_endSel.iEntry)
            break;
      }

      return true;
   }

   return false;
}

///////////////////////////////////////

void cHistoryWnd::EnforceMaxEntries()
{
   while (!m_entries.empty() && m_entries.size() > GetMaxEntries())
   {
      Assert(m_entries[0] != NULL);
      cEntry * pRemoved = m_entries[0];
      m_entries.erase(m_entries.begin());

      // if the entry removed was the longest one, find the new longest entry
      if (pRemoved->GetTextLen() == m_nMaxEntryLen)
      {
         m_nMaxEntryLen = 0;

         tEntries::iterator iter;
         for (iter = m_entries.begin(); iter != m_entries.end(); iter++)
         {
            m_nMaxEntryLen = max((*iter)->GetTextLen(), m_nMaxEntryLen);
         }
      }

      // if the entry removed was the biggest one, find the new biggest entry
      CDC * pDC = GetDC();
      CFont * pOldFont = pDC->SelectObject(&m_font);

      CSize size = pDC->GetTextExtent(pRemoved->GetText());

      if (size.cx == m_maxEntrySize)
      {
         m_maxEntrySize = 0;

         tEntries::iterator iter;
         for (iter = m_entries.begin(); iter != m_entries.end(); iter++)
         {
            CSize size = pDC->GetTextExtent((*iter)->GetText());
            if (size.cx > m_maxEntrySize)
               m_maxEntrySize = size.cx;
         }
      }

      pDC->SelectObject(pOldFont);
      ReleaseDC(pDC);

      delete pRemoved;
   }
}

///////////////////////////////////////

void cHistoryWnd::UpdateScrollInfo()
{
   CRect r;
   GetClientRect(&r);

   CSize total(
      m_maxEntrySize,
      m_entries.size() * m_nCharHeight + r.Height() % m_nCharHeight);

   CSize page(m_nAvgCharWidth, m_nCharHeight);
   if (m_nAvgCharWidth < r.Width())
   {
      page.cx = (r.Width() / m_nAvgCharWidth) * m_nAvgCharWidth;
   }
   if (m_nCharHeight < r.Height())
   {
      page.cy = (r.Height() / m_nCharHeight) * m_nCharHeight;
   }

   SetScrollSizes(MM_TEXT, total, page, CSize(m_nAvgCharWidth, m_nCharHeight));
}

///////////////////////////////////////

void cHistoryWnd::UpdateFontMetrics()
{
   CDC * pDC = GetDC();
   ASSERT_VALID(pDC);

   CFont * pOldFont = pDC->SelectObject(&m_font);

   TEXTMETRIC tm;
   pDC->GetTextMetrics(&tm);

   m_nCharHeight = tm.tmHeight;
   m_nAvgCharWidth = tm.tmAveCharWidth;

   pDC->SelectObject(pOldFont);

   ReleaseDC(pDC);
}

///////////////////////////////////////

bool cHistoryWnd::HitTest(const CPoint & point, sHitTestInfo * pHitTest)
{
   CPoint origin = GetScrollPosition();

   CPoint test(point.x + origin.x, point.y + origin.y);

   CDC * pDC = GetDC();

   CFont * pOldFont = pDC->SelectObject(&m_font);

   CRect r;
   GetClientRect(&r);

   r.bottom = r.top + m_nCharHeight;

   bool bHit = false;
   int index;
   tEntries::iterator iter;
   for (index = 0, iter = m_entries.begin(); iter != m_entries.end(); index++, iter++)
   {
      if (test.y >= r.top && test.y < r.bottom)
      {
         if (pHitTest)
         {
            pHitTest->Reset();

            pHitTest->pEntryHit = *iter;
            pHitTest->iEntry = index;

            CSize charSize;
            LPCTSTR pChar = (*iter)->GetText();
            for (int charX = 0; *pChar != '\0'; pChar++, charX += charSize.cx)
            {
               charSize = pDC->GetTextExtent(pChar, 1);

               if ((test.x >= charX) && (test.x < (charX + charSize.cx)))
               {
                  pHitTest->iCharHit = pChar - (*iter)->GetText();
                  pHitTest->charX = charX;
                  break;
               }
            }

            if (*pChar == '\0')
            {
               pChar--;
               pHitTest->iCharHit = (*iter)->GetTextLen();// - 1;
               pHitTest->charX = charX;
            }

            pHitTest->rectHit = r;

            bHit = true;
         }

         break;
      }

      r.OffsetRect(0, r.Height());
   }

   pDC->SelectObject(pOldFont);

   ReleaseDC(pDC);

   return bHit;
}

///////////////////////////////////////

bool cHistoryWnd::HitTestSelection(const CPoint & point)
{
   if (m_startSel.iEntry < 0 || m_endSel.iEntry < 0)
      return false;

   CPoint origin = GetScrollPosition();

   CPoint test(point.x + origin.x, point.y + origin.y);

   CDC * pDC = GetDC();

   CFont * pOldFont = pDC->SelectObject(&m_font);

   CRect r = m_startSel.rectHit;

   bool bHit = false;

   if (m_startSel.iEntry > -1)
   {
      for (int i = m_startSel.iEntry; i < m_entries.size(); i++)
      {
         if (i == m_startSel.iEntry)
         {
            r.left = m_startSel.charX;
         }
         else
         {
            r.left = 0;
         }

         if (i == m_endSel.iEntry)
         {
            r.right = m_endSel.charX;
         }
         else
         {
            CSize size = pDC->GetTextExtent(m_entries[i]->GetText());
            r.right = size.cx;
         }

         if (r.PtInRect(test))
         {
            bHit = true;
            break;
         }

         // if we just did the last entry in the selection, break out of the loop
         if (i == m_endSel.iEntry)
         {
            break;
         }

         r.OffsetRect(0, r.Height());
      }
   }

   pDC->SelectObject(pOldFont);

   ReleaseDC(pDC);

   return bHit;
}

///////////////////////////////////////
// Clean up ALL selection data

void cHistoryWnd::ClearSel()
{
   m_startSel.Reset();
   m_endSel.Reset();

   m_pSelAnchor = NULL;
   m_pSelDrag = NULL;
}

///////////////////////////////////////

template <class T>
void Swap(T & a, T & b)
{
   T temp;
   temp = a;
   a = b;
   b = temp;
}

///////////////////////////////////////

void cHistoryWnd::UpdateSelDrag(const CPoint & point)
{
   CRect r;
   GetClientRect(&r);

   if (point.y < r.top)
   {
      SendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0));
   }
   else if (point.y > r.bottom)
   {
      SendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0));
   }

   if (point.x > r.right)
   {
      SendMessage(WM_HSCROLL, MAKEWPARAM(SB_LINERIGHT, 0));
   }
   else if (point.x < r.left)
   {
      SendMessage(WM_HSCROLL, MAKEWPARAM(SB_LINELEFT, 0));
   }

   if (HitTest(point, m_pSelDrag))
   {
      int iAnchorEntry = m_pSelAnchor->iEntry;
      int iDragEntry = m_pSelDrag->iEntry;

      // is the drag pos less than the anchor pos?
      bool bDragLTAnchor = (iDragEntry < iAnchorEntry) || 
         ((iDragEntry == iAnchorEntry) && (m_pSelDrag->iCharHit < m_pSelAnchor->iCharHit));

      // is the drag pos equal to the anchor pos?
      bool bDragEqAnchor = (iDragEntry == iAnchorEntry) && (m_pSelDrag->iCharHit == m_pSelAnchor->iCharHit);

      // is the drag pos greater than the anchor pos?
      bool bDragGTAnchor = (iDragEntry > iAnchorEntry) || 
         ((iDragEntry == iAnchorEntry) && (m_pSelDrag->iCharHit > m_pSelAnchor->iCharHit));

      if ((m_bDragGTEAnchor && bDragLTAnchor) ||
          (!m_bDragGTEAnchor && (bDragGTAnchor || bDragEqAnchor)))
      {
         Swap(*m_pSelAnchor, *m_pSelDrag);
         Swap(m_pSelAnchor, m_pSelDrag);
         m_bDragGTEAnchor = !m_bDragGTEAnchor;
      }

      Invalidate(FALSE);

      UpdateWindow();
   }
}

///////////////////////////////////////

void cHistoryWnd::GetVisibleRange(int * pStart, int * pEnd)
{
   Assert(pStart != NULL);
   Assert(pEnd != NULL);
   CPoint scroll = GetScrollPosition();
   CRect rect;
   GetClientRect(rect);
   Assert(m_nCharHeight > 0);
   *pStart = scroll.y / m_nCharHeight;
   *pEnd = (scroll.y + rect.Height()) / m_nCharHeight;
}

///////////////////////////////////////

void cHistoryWnd::AddContextMenuItems(CMenu* pMenu)
{
   ASSERT_VALID(pMenu);
   Verify(pMenu->AppendMenu(MF_STRING, ID_EDIT_COPY, "&Copy"));
   Verify(pMenu->AppendMenu(MF_SEPARATOR));
   Verify(pMenu->AppendMenu(MF_STRING, ID_EDIT_CLEAR, "Clea&r"));
}

///////////////////////////////////////

#ifdef _DEBUG
void cHistoryWnd::AssertValid() const
{
   CScrollWnd::AssertValid();

   ASSERT_VALID(&m_font);

   Assert(m_entries.size() <= m_nMaxEntries);
}
#endif /* _DEBUG */

///////////////////////////////////////

#ifdef _DEBUG
void cHistoryWnd::Dump(CDumpContext& dc) const
{
   CScrollWnd::Dump(dc);

#define DUMP_MEMBER(member) \
   dc << "\n" #member " = " << member

   DUMP_MEMBER(m_textColor);
   DUMP_MEMBER(m_nMaxEntries);
//   DUMP_MEMBER(m_entries);
   DUMP_MEMBER(m_font);
   DUMP_MEMBER(m_nCharHeight);
   DUMP_MEMBER(m_nAvgCharWidth);
   DUMP_MEMBER(m_nMaxEntryLen);
   DUMP_MEMBER(m_maxEntrySize);
   DUMP_MEMBER(m_entries.size());
   DUMP_MEMBER(m_bAtEnd);
   DUMP_MEMBER(m_nAddsSinceLastPaint);
//   DUMP_MEMBER(m_startSel);
//   DUMP_MEMBER(m_endSel);
   DUMP_MEMBER(m_pSelAnchor);
   DUMP_MEMBER(m_pSelDrag);
   DUMP_MEMBER(m_bDragGTEAnchor);
}
#endif /* _DEBUG */

///////////////////////////////////////

void cHistoryWnd::OnPaint() 
{
   CPaintDC dc(this);

   // set window origin, clipping, etc.
   OnPrepareDC(&dc);

   int savedDC = dc.SaveDC();

   CFont * pOldFont = dc.SelectObject(&m_font);

   int start, end;
   GetVisibleRange(&start, &end);

   CRect r;
   r.SetRect(0, start * m_nCharHeight, m_totalDev.cx, m_totalDev.cy);

   r.bottom = r.top + m_nCharHeight;

   bool bInSel = false;

   int index;
   tEntries::iterator iter;
   for (index = 0, iter = (m_entries.begin() + start);
        iter != m_entries.end();
        index++, iter++)
   {
      if ((index == m_startSel.iEntry) && (index == m_endSel.iEntry))
      {
         CRect clip;

         clip = r;
         clip.right = m_startSel.charX;

         dc.SetBkColor(GetBkColor());
         dc.SetTextColor((*iter)->GetTextColor());
         dc.ExtTextOut(r.left, r.top, ETO_OPAQUE | ETO_CLIPPED, &clip, (*iter)->GetText(), (*iter)->GetTextLen(), NULL);

         clip.left = clip.right;
         clip.right = m_endSel.charX;

         dc.SetBkColor(GetSysColor(COLOR_HIGHLIGHT));
         dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
         dc.ExtTextOut(r.left, r.top, ETO_OPAQUE | ETO_CLIPPED, &clip, (*iter)->GetText(), (*iter)->GetTextLen(), NULL);

         clip.left = clip.right;
         clip.right = r.right;

         dc.SetBkColor(GetBkColor());
         dc.SetTextColor((*iter)->GetTextColor());
         dc.ExtTextOut(r.left, r.top, ETO_OPAQUE | ETO_CLIPPED, &clip, (*iter)->GetText(), (*iter)->GetTextLen(), NULL);
      }
      else if (index == m_startSel.iEntry)
      {
         bInSel = true;

         CRect clip;

         clip = r;
         clip.right = m_startSel.charX;

         dc.SetBkColor(GetBkColor());
         dc.SetTextColor((*iter)->GetTextColor());
         dc.ExtTextOut(r.left, r.top, ETO_OPAQUE | ETO_CLIPPED, &clip, (*iter)->GetText(), (*iter)->GetTextLen(), NULL);

         CSize textSize = dc.GetTextExtent((*iter)->GetText(), (*iter)->GetTextLen());

         clip = r;
         clip.left = m_startSel.charX;
         clip.right = textSize.cx;

         dc.SetBkColor(GetSysColor(COLOR_HIGHLIGHT));
         dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
         dc.ExtTextOut(r.left, r.top, ETO_OPAQUE | ETO_CLIPPED, &clip, (*iter)->GetText(), (*iter)->GetTextLen(), NULL);

         clip.left = clip.right;
         clip.right = r.right;

         dc.ExtTextOut(r.left, r.top, ETO_OPAQUE | ETO_CLIPPED, &clip, NULL, 0, NULL);
      }
      else if (index == m_endSel.iEntry)
      {
         bInSel = false;

         CRect clip;

         clip = r;
         clip.right = m_endSel.charX;

         dc.SetBkColor(GetSysColor(COLOR_HIGHLIGHT));
         dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
         dc.ExtTextOut(r.left, r.top, ETO_OPAQUE | ETO_CLIPPED, &clip, (*iter)->GetText(), (*iter)->GetTextLen(), NULL);

         clip = r;
         clip.left = m_endSel.charX;

         dc.SetBkColor(GetBkColor());
         dc.SetTextColor((*iter)->GetTextColor());
         dc.ExtTextOut(r.left, r.top, ETO_OPAQUE | ETO_CLIPPED, &clip, (*iter)->GetText(), (*iter)->GetTextLen(), NULL);
      }
      else
      {
         if (bInSel)
         {
            CSize textSize = dc.GetTextExtent((*iter)->GetText(), (*iter)->GetTextLen());

            CRect clip;
            clip = r;
            clip.right = textSize.cx;

            dc.SetBkColor(GetSysColor(COLOR_HIGHLIGHT));
            dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
            dc.ExtTextOut(r.left, r.top, ETO_OPAQUE | ETO_CLIPPED, &clip, (*iter)->GetText(), (*iter)->GetTextLen(), NULL);

            clip.left = clip.right;
            clip.right = r.right;
            dc.ExtTextOut(r.left, r.top, ETO_OPAQUE | ETO_CLIPPED, &clip, NULL, 0, NULL);
         }
         else
         {
            dc.SetBkColor(GetBkColor());
            dc.SetTextColor((*iter)->GetTextColor());
            dc.ExtTextOut(r.left, r.top, ETO_OPAQUE, &r, (*iter)->GetText(), (*iter)->GetTextLen(), NULL);
         }
      }

      // Prepare to draw the next entry
      r.OffsetRect(0, r.Height());
   }

   dc.RestoreDC(savedDC);

   dc.SelectObject(pOldFont);

   m_nAddsSinceLastPaint = 0;
}

///////////////////////////////////////

BOOL cHistoryWnd::OnEraseBkgnd(CDC * pDC) 
{
   CRect rect;
   GetClientRect(&rect);
   pDC->FillSolidRect(rect, GetBkColor());
   return TRUE;
}

///////////////////////////////////////

int cHistoryWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
   if (CScrollWnd::OnCreate(lpCreateStruct) == -1)
      return -1;

   // Set mapping mode to MM_TEXT
   SetScrollSizes(MM_TEXT, CSize(0,0), CSize(0,0), CSize(0,0));

   UpdateFontMetrics();

   AfxOleInit();

   return 0;
}

///////////////////////////////////////

void cHistoryWnd::OnDestroy() 
{
   CScrollWnd::OnDestroy();

   AfxOleTerm();
}

///////////////////////////////////////

void cHistoryWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
   SCROLLINFO info;
   GetScrollInfo(SB_VERT, &info);

   if (info.nPos >= (info.nMax - info.nPage))
   {
      m_bAtEnd = true;
   }
   else
   {
      m_bAtEnd = false;
   }

   CScrollWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

///////////////////////////////////////

void cHistoryWnd::OnContextMenu(CWnd* pWnd, CPoint point) 
{
   // We're purposely NOT calling CScrollWnd::OnContextMenu because its
   // context menu has all kinds of scroll zooming wackiness that we don't want.

   // TODO Make the context menu route through the command system

   CMenu contextMenu;
   VERIFY(contextMenu.CreatePopupMenu());

   AddContextMenuItems(&contextMenu);

   contextMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}

///////////////////////////////////////

void cHistoryWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
   if (HitTestSelection(point))
   {
      CString sel;
      if (GetSelection(&sel) && sel.GetLength() > 0)
      {
         cTextDataSource tds;
         VERIFY(tds.SetText(sel));
         tds.DoDragDrop();
      }
   }
   else
   {
      ClearSel();
      Invalidate(FALSE);
      UpdateWindow();

      if (HitTest(point, &m_startSel))
      {
         SetCapture();

         m_bDragGTEAnchor = TRUE;
         m_pSelAnchor = &m_startSel;
         m_pSelDrag = &m_endSel;
      }
   }

   CScrollWnd::OnLButtonDown(nFlags, point);
}

///////////////////////////////////////

void cHistoryWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
   if (GetCapture() == this)
   {
      if (m_startSel.pEntryHit == NULL)
      {
         if (!m_entries.empty())
         {
            m_startSel.pEntryHit = m_entries[0];
            m_startSel.iEntry = 0;
            m_startSel.iCharHit = 0;
         }
      }

      if (m_endSel.pEntryHit == NULL)
      {
         ClearSel();
         Invalidate(FALSE);
      }

      ReleaseCapture();
   }

   CScrollWnd::OnLButtonUp(nFlags, point);
}

///////////////////////////////////////

void cHistoryWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
   if (GetCapture() == this)
   {
      UpdateSelDrag(point);
   }

   CScrollWnd::OnMouseMove(nFlags, point);
}

///////////////////////////////////////

BOOL cHistoryWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
   // Give DefWindowProc() first crack
//   if (Default())
//      return TRUE;

   if (nHitTest == HTCLIENT)
   {
      CPoint pt(GetMessagePos());
      ScreenToClient(&pt);
      if (HitTestSelection(pt))
         SetCursor(LoadCursor(NULL, IDC_ARROW));
      else
         SetCursor(LoadCursor(NULL, IDC_IBEAM));
      return TRUE;
   }

   return CScrollWnd::OnSetCursor(pWnd, nHitTest, message);
//   return FALSE;
}

///////////////////////////////////////

void cHistoryWnd::OnEditCopy() 
{
   CopySelection();
}

///////////////////////////////////////

void cHistoryWnd::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
   pCmdUI->Enable((m_startSel.iEntry > -1) && (m_endSel.iEntry > -1));
}

///////////////////////////////////////

void cHistoryWnd::OnEditClear() 
{
   Clear();
}

///////////////////////////////////////

LRESULT cHistoryWnd::OnSetFont(WPARAM wParam, LPARAM lParam)
{
   SetFont(CFont::FromHandle((HFONT)wParam), LOWORD(lParam));
   return 0;
}

///////////////////////////////////////

cHistoryWnd::sHitTestInfo::sHitTestInfo()
 : pEntryHit(NULL),
   iEntry(-1),
   iCharHit(-1),
   rectHit(0,0,0,0),
   charX(-1)
{
}

///////////////////////////////////////

void cHistoryWnd::sHitTestInfo::Reset()
{
   pEntryHit = NULL;
   iEntry = -1;
   iCharHit = -1;
   rectHit.SetRectEmpty();
   charX = -1;
}

///////////////////////////////////////////////////////////////////////////////
