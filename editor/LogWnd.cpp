///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "LogWnd.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

static const COLORREF g_logColors[] =
{
   RGB(255,0,0),                    // kError
   GetSysColor(COLOR_WINDOWTEXT),   // kWarning
   GetSysColor(COLOR_WINDOWTEXT),   // kInfo
   RGB(0,0,255),                    // kDebug
};

static const int kLeftColumnMargin = 1;
static const COLORREF kLeftColumnColor = GetSysColor(COLOR_3DFACE);
static const COLORREF kLeftColumnBorderColor = GetSysColor(COLOR_3DDKSHADOW);

///////////////////////////////////////////////////////////////////////////////

static void TrimTrailingSpace(std::string * pString)
{
   Assert(pString != NULL);
   if (pString != NULL)
   {
      std::string::size_type iLastNonSpace = pString->find_last_not_of(" \t\r\n");
      if (iLastNonSpace != std::string::npos)
      {
         pString->erase(iLastNonSpace + 1);
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLogWndItem
//

////////////////////////////////////////

cLogWndItem::cLogWndItem()
 : m_severity(kInfo)
{
}

////////////////////////////////////////

cLogWndItem::cLogWndItem(eLogSeverity severity, const std::string & string)
 : m_severity(severity),
   m_string(string)
{
}

////////////////////////////////////////

cLogWndItem::cLogWndItem(const cLogWndItem & other)
 : m_severity(other.m_severity),
   m_string(other.m_string)
{
}

////////////////////////////////////////

cLogWndItem::~cLogWndItem()
{
}

////////////////////////////////////////

const cLogWndItem & cLogWndItem::operator =(const cLogWndItem & other)
{
   m_severity = other.m_severity;
   m_string = other.m_string;
   return *this;
}

////////////////////////////////////////

eLogSeverity cLogWndItem::GetSeverity() const
{
   return m_severity;
}

////////////////////////////////////////

const tChar * cLogWndItem::GetString() const
{
   return m_string.c_str();
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLogWndItemRender
//

////////////////////////////////////////

cLogWndItemRender::cLogWndItemRender(HDC hDC, const CRect & startRect,
                                     bool bCalcOnly)
 : m_dc(hDC),
   m_rect(startRect),
   m_bCalcOnly(bCalcOnly),
   m_rightSide(startRect.right),
   m_totalHeight(0)
{
}

////////////////////////////////////////

cLogWndItemRender::cLogWndItemRender(const cLogWndItemRender & other)
{
   operator =(other);
}

////////////////////////////////////////

cLogWndItemRender::~cLogWndItemRender()
{
}

////////////////////////////////////////

const cLogWndItemRender & cLogWndItemRender::operator =(const cLogWndItemRender & other)
{
   m_dc = other.m_dc;
   m_rect = other.m_rect;
   m_bCalcOnly = other.m_bCalcOnly;
   m_rightSide = other.m_rightSide;
   m_totalHeight = other.m_totalHeight;
   return *this;
}

////////////////////////////////////////

void cLogWndItemRender::Render(const cLogWndItem & item)
{
   static const uint kDrawTextFlags = DT_LEFT | DT_TOP | DT_WORDBREAK;

   m_dc.DrawText(item.GetString(), -1, &m_rect, kDrawTextFlags | DT_CALCRECT);

   if (!m_bCalcOnly)
   {
      CRect rect(m_rect);
      rect.right = kLeftColumnWidth;
      RenderLeftColumn(rect);

      rect = m_rect;
      rect.OffsetRect(kLeftColumnWidth + kLeftColumnMargin, 0);

      COLORREF oldTextColor = m_dc.SetTextColor(g_logColors[item.GetSeverity()]);
      m_dc.DrawText(item.GetString(), -1, &rect, kDrawTextFlags);
      m_dc.SetTextColor(oldTextColor);
   }

   m_totalHeight += m_rect.Height();

   m_rect.top = m_rect.bottom;
   m_rect.right = m_rightSide;
}

////////////////////////////////////////

void cLogWndItemRender::RenderLeftColumn(const CRect & rect)
{
   CRect r2(rect);
   r2.right -= 1;
   COLORREF oldBkColor = m_dc.SetBkColor(kLeftColumnColor);
   m_dc.ExtTextOut(0, 0, ETO_OPAQUE, r2, NULL, 0, NULL);
   r2 = rect;
   r2.left = r2.right - 1;
   m_dc.SetBkColor(kLeftColumnBorderColor);
   m_dc.ExtTextOut(0, 0, ETO_OPAQUE, r2, NULL, 0, NULL);
   m_dc.SetBkColor(oldBkColor);
}

////////////////////////////////////////

int cLogWndItemRender::GetTotalHeight() const
{
   return m_totalHeight;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLogWnd
//

////////////////////////////////////////

cLogWnd::cLogWnd()
{
}

////////////////////////////////////////

cLogWnd::~cLogWnd()
{
}

////////////////////////////////////////

tResult cLogWnd::AddString(const tChar * pszString, size_t length /*=-1*/)
{
   return AddString(kInfo, pszString, length);
}

////////////////////////////////////////

tResult cLogWnd::AddString(eLogSeverity severity, const tChar * pszString, size_t length)
{
   if (pszString == NULL)
   {
      return E_POINTER;
   }

   std::string s(pszString, (length != -1) ? length : std::string::npos);
   TrimTrailingSpace(&s);

   m_items.push_back(cLogWndItem(severity, s));

   while (m_items.size() > kMaxItems)
   {
      m_items.erase(m_items.begin());
   }

   UpdateScrollInfo();
   ScrollBottom();
   Invalidate();

   return S_OK;
}

////////////////////////////////////////

tResult cLogWnd::Clear()
{
   m_items.clear();

   UpdateScrollInfo();
   Invalidate();

   return S_OK;
}

////////////////////////////////////////

void cLogWnd::DoPaint(CDCHandle dc)
{
   CRect rect;
   Verify(GetClientRect(&rect));

   HFONT hOldFont = dc.SelectFont(m_font.IsNull() ? AtlGetDefaultGuiFont() : m_font);

   cLogWndItemRender renderer(dc, rect);

   tItems::const_iterator iter = m_items.begin();
   tItems::const_iterator end = m_items.end();
   for (; iter != end; iter++)
   {
      renderer.Render(*iter);
   }

   if (renderer.GetTotalHeight() < rect.Height())
   {
      renderer.RenderLeftColumn(
         CRect(rect.left,
               renderer.GetTotalHeight(),
               rect.left + cLogWndItemRender::kLeftColumnWidth,
               rect.bottom));
   }

   dc.SelectFont(hOldFont);
}

////////////////////////////////////////

void cLogWnd::OnDestroy()
{
   SetFont(NULL, FALSE);

   m_items.clear();
}

////////////////////////////////////////

void cLogWnd::OnSetFont(HFONT hFont, BOOL bRedraw)
{
   if (!m_font.IsNull())
   {
      m_font.DeleteObject();
   }

   if (hFont != NULL)
   {
      LOGFONT logFont = {0};
      if (GetObject(hFont, sizeof(LOGFONT), &logFont))
      {
         m_font.CreateFontIndirect(&logFont);
      }
   }
}

////////////////////////////////////////

LRESULT cLogWnd::OnSetCursor(HWND hWnd, UINT hitTest, UINT message)
{
   if (hitTest == HTCLIENT)
   {
      SetCursor(LoadCursor(NULL, IDC_IBEAM));
      return TRUE;
   }

   SetMsgHandled(FALSE);
   return 0;
}

////////////////////////////////////////

void cLogWnd::OnMouseMove(UINT flags, CPoint point)
{
}

////////////////////////////////////////

void cLogWnd::OnLButtonDown(UINT flags, CPoint point)
{
}

////////////////////////////////////////

void cLogWnd::OnLButtonUp(UINT flags, CPoint point)
{
}

////////////////////////////////////////

void cLogWnd::UpdateScrollInfo()
{
   CRect rect;
   Verify(GetClientRect(&rect));

   CDC dc(GetDC());

   HFONT hOldFont = dc.SelectFont(m_font.IsNull() ? AtlGetDefaultGuiFont() : m_font);

   cLogWndItemRender renderer(dc, rect, true);

   tItems::const_iterator iter = m_items.begin();
   tItems::const_iterator end = m_items.end();
   for (; iter != end; iter++)
   {
      renderer.Render(*iter);
   }

   SetScrollSize(1, renderer.GetTotalHeight());

   dc.SelectFont(hOldFont);
}

///////////////////////////////////////////////////////////////////////////////
