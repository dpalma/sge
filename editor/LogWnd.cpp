///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "LogWnd.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

static const COLORREF g_logColors[] =
{
   RGB(255,0,0),     // kError
   RGB(0,255,255),   // kWarning
   GetSysColor(COLOR_WINDOWTEXT), // kInfo
   RGB(0,0,255),     // kDebug
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

cLogWndItemRender::cLogWndItemRender(WTL::CDCHandle dc, const CRect & startRect,
                                     HFONT hFont, bool bCalcOnly)
 : m_dc(dc),
   m_rect(startRect),
   m_hOldFont(NULL),
   m_bCalcOnly(bCalcOnly),
   m_rightSide(startRect.right),
   m_totalHeight(0)
{
   if (hFont != NULL)
   {
      m_hOldFont = m_dc.SelectFont(hFont);
   }
}

////////////////////////////////////////

cLogWndItemRender::cLogWndItemRender(const cLogWndItemRender & other)
 : m_hOldFont(NULL)
{
   operator =(other);
}

////////////////////////////////////////

cLogWndItemRender::~cLogWndItemRender()
{
   if (m_hOldFont != NULL)
   {
      m_dc.SelectFont(m_hOldFont);
      m_hOldFont = NULL;
   }
}

////////////////////////////////////////

const cLogWndItemRender & cLogWndItemRender::operator =(const cLogWndItemRender & other)
{
   // Do not copy m_hOldFont
   m_dc = other.m_dc;
   m_rect = other.m_rect;
   m_bCalcOnly = other.m_bCalcOnly;
   m_rightSide = other.m_rightSide;
   m_rects.resize(other.m_rects.size());
   std::copy(other.m_rects.begin(), other.m_rects.end(), m_rects.begin());
   m_totalHeight = other.m_totalHeight;
   return *this;
}

////////////////////////////////////////

void cLogWndItemRender::operator ()(const cLogWndItem & item)
{
   Invoke(item);
}

////////////////////////////////////////

void cLogWndItemRender::Invoke(const cLogWndItem & item)
{
   m_dc.DrawText(item.GetString(), -1, &m_rect, DT_FLAGS | DT_CALCRECT);

   if (!m_bCalcOnly)
   {
      CRect rect(m_rect);
      rect.right = kLeftColumnWidth;
      RenderLeftColumn(rect);

      rect = m_rect;
      rect.OffsetRect(kLeftColumnWidth + kLeftColumnMargin, 0);

      COLORREF oldTextColor = m_dc.SetTextColor(g_logColors[item.GetSeverity()]);
      m_dc.DrawText(item.GetString(), -1, &rect, DT_FLAGS);
      m_dc.SetTextColor(oldTextColor);
   }

   m_totalHeight += m_rect.Height();

   m_rects.push_back(m_rect);

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

const std::vector<CRect> & cLogWndItemRender::GetRects() const
{
   return m_rects;
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

BOOL cLogWnd::PreTranslateMessage(MSG * pMsg)
{
   return FALSE;
}

////////////////////////////////////////

void cLogWnd::DoPaint(WTL::CDCHandle dc)
{
   CRect rect;
   Verify(GetClientRect(&rect));

   cLogWndItemRender renderer(dc, rect, WTL::AtlGetDefaultGuiFont());

   cLogWndItemRender result = std::for_each(m_items.begin(), m_items.end(), renderer);

   if (result.GetTotalHeight() < rect.Height())
   {
      renderer.RenderLeftColumn(
         CRect(rect.left,
               result.GetTotalHeight(),
               rect.left + cLogWndItemRender::kLeftColumnWidth,
               rect.bottom));
   }
}

////////////////////////////////////////

void cLogWnd::UpdateScrollInfo()
{
   CRect rect;
   Verify(GetClientRect(&rect));

   WTL::CDC dc(GetDC());

   cLogWndItemRender renderer(WTL::CDCHandle(dc), rect, WTL::AtlGetDefaultGuiFont(), true);

   cLogWndItemRender result = std::for_each(m_items.begin(), m_items.end(), renderer);

   SetScrollSize(1, result.GetTotalHeight());
}

///////////////////////////////////////////////////////////////////////////////
