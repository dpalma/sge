///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "LogWnd.h"

#include "dbgalloc.h" // must be last header

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
 : m_severity(kInfo),
   m_bExpanded(false)
{
}

////////////////////////////////////////

cLogWndItem::cLogWndItem(const std::string & string, const std::string & detail, eLogSeverity severity)
 : m_string(string),
   m_detail(detail),
   m_severity(severity),
   m_bExpanded(false)
{
}

////////////////////////////////////////

cLogWndItem::cLogWndItem(const cLogWndItem & other)
 : m_string(other.m_string),
   m_detail(other.m_detail),
   m_severity(other.m_severity),
   m_bExpanded(other.m_bExpanded)
{
}

////////////////////////////////////////

cLogWndItem::~cLogWndItem()
{
}

////////////////////////////////////////

const cLogWndItem & cLogWndItem::operator =(const cLogWndItem & other)
{
   m_string = other.m_string;
   m_detail = other.m_detail;
   m_severity = other.m_severity;
   m_bExpanded = other.m_bExpanded;
   return *this;
}

////////////////////////////////////////

const tChar * cLogWndItem::GetString() const
{
   return m_string.c_str();
}

////////////////////////////////////////

bool cLogWndItem::HasDetail() const
{
   return !m_detail.empty();
}

////////////////////////////////////////

const tChar * cLogWndItem::GetDetail() const
{
   return m_detail.c_str();
}

////////////////////////////////////////

eLogSeverity cLogWndItem::GetSeverity() const
{
   return m_severity;
}

////////////////////////////////////////

bool cLogWndItem::IsExpanded() const
{
   return m_bExpanded;
}

////////////////////////////////////////

void cLogWndItem::ToggleExpand()
{
   m_bExpanded = !m_bExpanded;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLogWndItemRender
//

////////////////////////////////////////

cLogWndItemRender::cLogWndItemRender(CWindow wnd, WTL::CDCHandle dc, bool bCalcOnly)
 : m_dc(dc),
   m_hOldFont(NULL),
   m_bCalcOnly(bCalcOnly),
   m_totalHeight(0)
{
   Assert(wnd.IsWindow());
   Verify(wnd.GetClientRect(&m_rect));
   m_rightSide = m_rect.right;

   HFONT hFont = WTL::AtlGetDefaultGuiFont();
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
      m_dc.DrawText(item.GetString(), -1, &m_rect, DT_FLAGS);
   }

   m_totalHeight += m_rect.Height();

   m_rects.push_back(m_rect);

   m_rect.top = m_rect.bottom;
   m_rect.right = m_rightSide;
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

tResult cLogWnd::AddString(const tChar * pszString,
                           size_t length /*=-1*/,
                           COLORREF color /*=CLR_INVALID*/)
{
   if (pszString == NULL)
   {
      return E_POINTER;
   }

   std::string s(pszString);
   TrimTrailingSpace(&s);

   m_items.push_back(cLogWndItem(s));

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
   std::for_each(m_items.begin(), m_items.end(), cLogWndItemRender(m_hWnd, dc));
}

////////////////////////////////////////

void cLogWnd::UpdateScrollInfo()
{
   cLogWndItemRender result = std::for_each(m_items.begin(), m_items.end(),
      cLogWndItemRender(m_hWnd, WTL::CDCHandle(WTL::CDC(GetDC())), true));

   int totalHeight = result.GetTotalHeight();

   SetScrollSize(1, totalHeight);
}

///////////////////////////////////////////////////////////////////////////////
