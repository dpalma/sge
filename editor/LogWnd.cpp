///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "LogWnd.h"

#include <afxole.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int kDefaultMaxLines = 1000;

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
// CLASS: cLogWndLine
//

///////////////////////////////////////

cLogWndLine::cLogWndLine(LPCTSTR pszText, int textLength, COLORREF textColor)
 : m_text(pszText, textLength),
   m_textColor(textColor)
{
}

///////////////////////////////////////

cLogWndLine::cLogWndLine(const cLogWndLine & other)
 : m_text(other.m_text),
   m_textColor(other.m_textColor)
{
}

///////////////////////////////////////

cLogWndLine::~cLogWndLine()
{
}

///////////////////////////////////////

const cLogWndLine & cLogWndLine::operator =(const cLogWndLine & other)
{
   m_text = other.m_text;
   m_textColor = other.m_textColor;
   return *this;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLogWndHitTestInfo
//

///////////////////////////////////////

cLogWndHitTestInfo::cLogWndHitTestInfo()
 : iItem(-1),
   iChar(-1),
   rect(0,0,0,0),
   charX(-1)
{
}

///////////////////////////////////////

cLogWndHitTestInfo::cLogWndHitTestInfo(const cLogWndHitTestInfo & other)
 : iItem(other.iItem),
   iChar(other.iChar),
   rect(other.rect),
   charX(other.charX)
{
}

///////////////////////////////////////

cLogWndHitTestInfo::~cLogWndHitTestInfo()
{
}

///////////////////////////////////////

const cLogWndHitTestInfo & cLogWndHitTestInfo::operator =(const cLogWndHitTestInfo & other)
{
   iItem = other.iItem;
   iChar = other.iChar;
   rect = other.rect;
   charX = other.charX;
   return *this;
}

///////////////////////////////////////

void cLogWndHitTestInfo::Reset()
{
   iItem = -1;
   iChar = -1;
   rect.SetRectEmpty();
   charX = -1;
}



///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLogWnd
//

///////////////////////////////////////

cLogWnd::cLogWnd()
 : m_textColor(GetSysColor(COLOR_WINDOWTEXT)),
   m_bkColor(GetSysColor(COLOR_WINDOW)),
   m_nMaxLines(kDefaultMaxLines),
   m_lineHeight(0),
   m_maxEntrySize(0),
   m_bAtEnd(true),
   m_nAddsSinceLastPaint(0),
   m_pSelAnchor(NULL),
   m_pSelDrag(NULL)
{
}

///////////////////////////////////////

cLogWnd::~cLogWnd()
{
   Clear();
}

///////////////////////////////////////

void cLogWnd::AddText(LPCTSTR pszText, int textLength, COLORREF textColor)
{
   Assert(AfxIsValidString(pszText, lstrlen(pszText)));
   Assert((textLength < 0) || (textLength <= lstrlen(pszText)));

   if (!IsWindow())
   {
      return;
   }

   static const tChar szLineBreakChars[] = _T("\r\n");

   if (textLength < 0)
   {
      textLength = lstrlen(pszText);
   }

   if (textColor == CLR_INVALID)
   {
      textColor = GetTextColor();
   }

   WTL::CDC dc(::GetDC(m_hWnd));
   HFONT hOldFont = dc.SelectFont(m_font);

   int breakLength = _tcscspn(pszText, szLineBreakChars);
   while ((breakLength <= textLength) && (breakLength > 0))
   {
      cLogWndLine * pEntry = new cLogWndLine(pszText, breakLength, textColor);
      m_lines.push_back(pEntry);

      CSize extent;
      dc.GetTextExtent(pszText, breakLength, &extent);
      if (extent.cx > m_maxEntrySize)
      {
         m_maxEntrySize = extent.cx;
      }

      pszText += breakLength;
      while ((*pszText != 0) && (_tcschr(szLineBreakChars, *pszText) != NULL))
      {
         pszText++;
      }
      breakLength = _tcscspn(pszText, szLineBreakChars);
   }

   dc.SelectFont(hOldFont);

   EnforceMaxLines();

   UpdateScrollInfo();

   // update the scroll position only if it's already at the very end
   if (m_bAtEnd)
   {
      ScrollBottom();
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

void cLogWnd::Clear()
{
   ClearSel();

   m_maxEntrySize = 0;

   tLogWndLines::iterator iter;
   for (iter = m_lines.begin(); iter != m_lines.end(); iter++)
   {
      delete *iter;
   }
   m_lines.clear();

   // Normally, when we clear the contents we want to update the scrollbar
   // calibration. However, we don't want to do this if the window has been
   // destroyed (i.e., Clear() is being called from the destructor).
   if (IsWindow())
   {
      m_bAtEnd = true;
      UpdateScrollInfo();
      Invalidate();
   }
}

///////////////////////////////////////

void cLogWnd::SetMaxLines(int nMaxLines)
{
   m_nMaxLines = nMaxLines;
   m_lines.reserve(nMaxLines);
   EnforceMaxLines();
}

///////////////////////////////////////

void cLogWnd::SetFont(HFONT hFont, BOOL bRedraw)
{
   LOGFONT lf;
   if (GetObject(hFont, sizeof(LOGFONT), &lf))
   {
      WTL::CFont newFont;
      if (newFont.CreateFontIndirect(&lf))
      {
         m_font.Attach(newFont.Detach());
      }
   }

   UpdateFontMetrics();
   UpdateScrollInfo();

   if (bRedraw)
   {
      Invalidate();
   }
}

///////////////////////////////////////

bool cLogWnd::CopySelection()
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

bool cLogWnd::GetSelection(CString * pSel)
{
   if (m_startSel.iItem > -1 && m_endSel.iItem > -1)
   {
      Assert(pSel != NULL);
      pSel->Empty();

      for (uint i = m_startSel.iItem; i < m_lines.size(); i++)
      {
         int iStart, iEnd;

         if (i == m_startSel.iItem)
         {
            iStart = m_startSel.iChar;
         }
         else
         {
            iStart = 0;
         }

         if (i == m_endSel.iItem)
         {
            iEnd = m_endSel.iChar;
         }
         else
         {
            iEnd = m_lines[i]->GetTextLen();
         }

         CString s = m_lines[i]->GetText().Mid(iStart, iEnd - iStart);

         if (pSel->GetLength())
         {
            *pSel += '\n';
         }
         *pSel += s;

         // if we just did the last entry in the selection, break out of the loop
         if (i == m_endSel.iItem)
         {
            break;
         }
      }

      return true;
   }

   return false;
}

///////////////////////////////////////

void cLogWnd::EnforceMaxLines()
{
   while (!m_lines.empty() && m_lines.size() > GetMaxLines())
   {
      Assert(m_lines[0] != NULL);
      cLogWndLine * pRemoved = m_lines[0];
      m_lines.erase(m_lines.begin());

      // if the entry removed was the biggest one, find the new biggest entry
      WTL::CDC dc(::GetDC(m_hWnd));
      HFONT hOldFont = dc.SelectFont(m_font);

      CSize size;
      dc.GetTextExtent(pRemoved->GetText(), -1, &size);

      if (size.cx == m_maxEntrySize)
      {
         m_maxEntrySize = 0;

         tLogWndLines::iterator iter;
         for (iter = m_lines.begin(); iter != m_lines.end(); iter++)
         {
            CSize size;
            if (dc.GetTextExtent((*iter)->GetText(), -1, &size) && size.cx > m_maxEntrySize)
            {
               m_maxEntrySize = size.cx;
            }
         }
      }

      dc.SelectFont(hOldFont);

      delete pRemoved;
   }
}

///////////////////////////////////////

void cLogWnd::UpdateScrollInfo()
{
   CRect r;
   GetClientRect(&r);

   CSize total(
      m_maxEntrySize,
      m_lines.size() * m_lineHeight + r.Height() % m_lineHeight);

   if (total.cx > 0 && total.cy > 0)
   {
      SetScrollSize(total);
   }
}

///////////////////////////////////////

void cLogWnd::UpdateFontMetrics()
{
   WTL::CDC dc(::GetDC(m_hWnd));
   HFONT hOldFont = dc.SelectFont(m_font);

   TEXTMETRIC tm;
   dc.GetTextMetrics(&tm);

   m_lineHeight = tm.tmHeight;

   dc.SelectFont(hOldFont);
}

///////////////////////////////////////

bool cLogWnd::HitTest(const CPoint & point, cLogWndHitTestInfo * pHitTest)
{
   WTL::CDC dc(::GetDC(m_hWnd));
   HFONT hOldFont = dc.SelectFont(m_font);

   CRect r;
   GetClientRect(&r);

   r.bottom = r.top + m_lineHeight;

   bool bHit = false;
   int index;
   tLogWndLines::iterator iter;
   for (index = 0, iter = m_lines.begin(); iter != m_lines.end(); index++, iter++)
   {
      if (point.y >= r.top && point.y < r.bottom)
      {
         if (pHitTest)
         {
            pHitTest->Reset();

            pHitTest->iItem = index;

            CSize charSize;
            LPCTSTR pChar = (*iter)->GetText();
            for (int charX = 0; *pChar != '\0'; pChar++, charX += charSize.cx)
            {
               dc.GetTextExtent(pChar, 1, &charSize);

               if ((point.x >= charX) && (point.x < (charX + charSize.cx)))
               {
                  pHitTest->iChar = pChar - (*iter)->GetText();
                  pHitTest->charX = charX;
                  break;
               }
            }

            if (*pChar == '\0')
            {
               pChar--;
               pHitTest->iChar = (*iter)->GetTextLen();// - 1;
               pHitTest->charX = charX;
            }

            pHitTest->rect = r;

            bHit = true;
         }

         break;
      }

      r.OffsetRect(0, r.Height());
   }

   dc.SelectFont(hOldFont);

   return bHit;
}

///////////////////////////////////////

bool cLogWnd::HitTestSelection(const CPoint & point)
{
   if (m_startSel.iItem < 0 || m_endSel.iItem < 0)
      return false;

   WTL::CDC dc(::GetDC(m_hWnd));
   HFONT hOldFont = dc.SelectFont(m_font);

   CRect r = m_startSel.rect;

   bool bHit = false;

   if (m_startSel.iItem > -1)
   {
      for (uint i = m_startSel.iItem; i < m_lines.size(); i++)
      {
         if (i == m_startSel.iItem)
         {
            r.left = m_startSel.charX;
         }
         else
         {
            r.left = 0;
         }

         if (i == m_endSel.iItem)
         {
            r.right = m_endSel.charX;
         }
         else
         {
            CSize size;
            dc.GetTextExtent(m_lines[i]->GetText(), -1, &size);
            r.right = size.cx;
         }

         if (r.PtInRect(point))
         {
            bHit = true;
            break;
         }

         // if we just did the last entry in the selection, break out of the loop
         if (i == m_endSel.iItem)
         {
            break;
         }

         r.OffsetRect(0, r.Height());
      }
   }

   dc.SelectFont(hOldFont);

   return bHit;
}

///////////////////////////////////////
// Clean up ALL selection data

void cLogWnd::ClearSel()
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

void cLogWnd::UpdateSelDrag(const CPoint & point)
{
   CRect r;
   GetClientRect(&r);

   if (point.y < r.top)
   {
      ScrollLineUp();
   }
   else if (point.y > r.bottom)
   {
      ScrollLineDown();
   }

   if (point.x > r.right)
   {
      ScrollLineRight();
   }
   else if (point.x < r.left)
   {
      ScrollLineLeft();
   }

   if (HitTest(point, m_pSelDrag))
   {
      int iAnchorEntry = m_pSelAnchor->iItem;
      int iDragEntry = m_pSelDrag->iItem;

      // is the drag pos less than the anchor pos?
      bool bDragLTAnchor = (iDragEntry < iAnchorEntry) || 
         ((iDragEntry == iAnchorEntry) && (m_pSelDrag->iChar < m_pSelAnchor->iChar));

      // is the drag pos equal to the anchor pos?
      bool bDragEqAnchor = (iDragEntry == iAnchorEntry) && (m_pSelDrag->iChar == m_pSelAnchor->iChar);

      // is the drag pos greater than the anchor pos?
      bool bDragGTAnchor = (iDragEntry > iAnchorEntry) || 
         ((iDragEntry == iAnchorEntry) && (m_pSelDrag->iChar > m_pSelAnchor->iChar));

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

void cLogWnd::GetVisibleRange(int * pStart, int * pEnd)
{
   Assert(m_lineHeight > 0);
   CRect rect;
   GetClientRect(rect);
   if (pStart != NULL)
   {
      *pStart = m_ptOffset.y / m_lineHeight;
   }
   if (pEnd != NULL)
   {
      *pEnd = (m_ptOffset.y + rect.Height()) / m_lineHeight;
   }
}

///////////////////////////////////////

void cLogWnd::AddContextMenuItems(WTL::CMenu* pMenu)
{
   if (pMenu != NULL)
   {
      static const UINT stdCmdIds[] = { ID_EDIT_COPY, ID_EDIT_CLEAR_ALL };
      for (UINT i = 0; i < _countof(stdCmdIds); i++)
      {
         CString str;
         if (str.LoadString(stdCmdIds[i]))
         {
            int index = str.ReverseFind('\n');
            if (index != -1)
            {
               str.Delete(0, index + 1);
            }
            if (i > 0 && pMenu->GetMenuItemCount() > 0)
            {
               Verify(pMenu->AppendMenu(MF_SEPARATOR));
            }
            Verify(pMenu->AppendMenu(MF_STRING, stdCmdIds[i], str));
         }
      }
   }
}

///////////////////////////////////////

void cLogWnd::DoPaint(WTL::CDCHandle dc)
{
   if (m_lines.empty())
   {
      return;
   }

   int savedDC = dc.SaveDC();

   HFONT hOldFont = dc.SelectFont(m_font);

   int iStart, iEnd;
   GetVisibleRange(&iStart, &iEnd);

   CRect client;
   GetClientRect(client);

   CRect r(client.left, iStart * m_lineHeight, client.right, 0);
   r.bottom = r.top + m_lineHeight;

   bool bInSel = false;

   tLogWndLines::iterator iter = m_lines.begin() + iStart;
   for (int index = 0; (iter != m_lines.end()) && (index <= iEnd); index++, iter++)
   {
      if ((index == m_startSel.iItem) && (index == m_endSel.iItem))
      {
         CRect clip(r);
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
      else if (index == m_startSel.iItem)
      {
         bInSel = true;

         CRect clip(r);
         clip.right = m_startSel.charX;

         dc.SetBkColor(GetBkColor());
         dc.SetTextColor((*iter)->GetTextColor());
         dc.ExtTextOut(r.left, r.top, ETO_OPAQUE | ETO_CLIPPED, &clip, (*iter)->GetText(), (*iter)->GetTextLen(), NULL);

         CSize textSize;
         dc.GetTextExtent((*iter)->GetText(), (*iter)->GetTextLen(), &textSize);

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
      else if (index == m_endSel.iItem)
      {
         bInSel = false;

         CRect clip(r);
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
            CSize textSize;
            dc.GetTextExtent((*iter)->GetText(), (*iter)->GetTextLen(), &textSize);

            CRect clip(r);
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

   if (r.top < client.bottom)
   {
      r.bottom = client.bottom;
      dc.FillSolidRect(r, GetBkColor());
   }

   dc.RestoreDC(savedDC);

   dc.SelectFont(hOldFont);

   m_nAddsSinceLastPaint = 0;
}

///////////////////////////////////////

void cLogWnd::DoScroll(int nType, int nScrollCode, int& cxyOffset, int cxySizeAll, int cxySizePage, int cxySizeLine)
{
   tBase::DoScroll(nType, nScrollCode, cxyOffset, cxySizeAll, cxySizePage, cxySizeLine);

   SCROLLINFO info;
   GetScrollInfo(SB_VERT, &info);

   if (info.nPos >= (int)(info.nMax - info.nPage))
   {
      m_bAtEnd = true;
   }
   else
   {
      m_bAtEnd = false;
   }
}

///////////////////////////////////////

LRESULT cLogWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
   UpdateFontMetrics();
   Verify(AfxOleInit());
   return 0;
}

///////////////////////////////////////

void cLogWnd::OnDestroy()
{
   AfxOleTerm();
}

///////////////////////////////////////

void cLogWnd::OnSetFont(HFONT hFont, BOOL bRedraw)
{
   SetFont(hFont, bRedraw);
}

///////////////////////////////////////

LRESULT cLogWnd::OnEraseBkgnd(WTL::CDCHandle dc)
{
   if (m_lines.empty())
   {
      CRect rect;
      GetClientRect(&rect);
      dc.FillSolidRect(rect, GetBkColor());
   }
   return TRUE;
}

///////////////////////////////////////

void cLogWnd::OnContextMenu(HWND hWnd, CPoint point)
{
   WTL::CMenu contextMenu;
   Verify(contextMenu.CreatePopupMenu());

   AddContextMenuItems(&contextMenu);

   if ((m_startSel.iItem > -1) && (m_endSel.iItem > -1))
   {
      contextMenu.EnableMenuItem(ID_EDIT_COPY, MF_BYCOMMAND | MF_ENABLED);
   }
   else
   {
      contextMenu.EnableMenuItem(ID_EDIT_COPY, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
   }

   contextMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, m_hWnd);
}

///////////////////////////////////////

LRESULT cLogWnd::OnSetCursor(HWND hWnd, UINT hitTest, UINT message)
{
   if (hitTest == HTCLIENT)
   {
      CPoint pt(GetMessagePos());
      ScreenToClient(&pt);
      if (HitTestSelection(pt))
      {
         SetCursor(LoadCursor(NULL, IDC_ARROW));
      }
      else
      {
         SetCursor(LoadCursor(NULL, IDC_IBEAM));
      }
      return TRUE;
   }

   return FALSE;
}

///////////////////////////////////////

void cLogWnd::OnMouseMove(UINT flags, CPoint point)
{
   if (GetCapture() == m_hWnd)
   {
      UpdateSelDrag(point);
   }
}

///////////////////////////////////////

void cLogWnd::OnLButtonDown(UINT flags, CPoint point)
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

         m_bDragGTEAnchor = true;
         m_pSelAnchor = &m_startSel;
         m_pSelDrag = &m_endSel;
      }
   }
}

///////////////////////////////////////

void cLogWnd::OnLButtonUp(UINT flags, CPoint point)
{
   if (GetCapture() == m_hWnd)
   {
      if (m_startSel.iItem == -1)
      {
         if (!m_lines.empty())
         {
            m_startSel.iItem = 0;
            m_startSel.iChar = 0;
         }
      }

      if (m_endSel.iItem == -1)
      {
         ClearSel();
         Invalidate(FALSE);
      }

      ReleaseCapture();
   }
}

///////////////////////////////////////

void cLogWnd::OnEditCopy(UINT code, int id, HWND hWnd)
{
   CopySelection();
}

///////////////////////////////////////

void cLogWnd::OnEditClearAll(UINT code, int id, HWND hWnd)
{
   Clear();
}

///////////////////////////////////////////////////////////////////////////////
