///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "FooWnd.h"

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
// CLASS: cFooWndItem
//

////////////////////////////////////////

cFooWndItem::cFooWndItem()
 : m_color(CLR_INVALID)
{
}

////////////////////////////////////////

cFooWndItem::cFooWndItem(const tChar * pszString, size_t length, COLORREF color)
 : m_string((pszString != NULL) ? std::string(pszString, length) : ""),
   m_color(color)
{
}

////////////////////////////////////////

cFooWndItem::cFooWndItem(const std::string & string, COLORREF color)
 : m_string(string),
   m_color(color)
{
}

////////////////////////////////////////

cFooWndItem::cFooWndItem(const cFooWndItem & other)
 : m_string(other.m_string),
   m_color(other.m_color)
{
}

////////////////////////////////////////

cFooWndItem::~cFooWndItem()
{
}

////////////////////////////////////////

const cFooWndItem & cFooWndItem::operator =(const cFooWndItem & other)
{
   m_string = other.m_string;
   m_color = other.m_color;
   return *this;
}

////////////////////////////////////////

const tChar * cFooWndItem::GetString() const
{
   return m_string.c_str();
}

////////////////////////////////////////

COLORREF cFooWndItem::GetColor() const
{
   return m_color;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFooWndItemRender
//

////////////////////////////////////////

cFooWndItemRender::cFooWndItemRender(CWindow wnd, WTL::CDCHandle dc, bool bCalcOnly)
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

cFooWndItemRender::cFooWndItemRender(const cFooWndItemRender & other)
 : m_hOldFont(NULL)
{
   operator =(other);
}

////////////////////////////////////////

cFooWndItemRender::~cFooWndItemRender()
{
   if (m_hOldFont != NULL)
   {
      m_dc.SelectFont(m_hOldFont);
      m_hOldFont = NULL;
   }
}

////////////////////////////////////////

const cFooWndItemRender & cFooWndItemRender::operator =(const cFooWndItemRender & other)
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

void cFooWndItemRender::operator ()(const cFooWndItem & item)
{
   Invoke(item);
}

////////////////////////////////////////

void cFooWndItemRender::Invoke(const cFooWndItem & item)
{
   m_dc.DrawText(item.GetString(), -1, &m_rect, DT_FLAGS | DT_CALCRECT);

   if (!m_bCalcOnly)
   {
      COLORREF textColor = item.GetColor();
      if (textColor != CLR_INVALID)
      {
         m_dc.SetTextColor(textColor);
      }

      m_dc.DrawText(item.GetString(), -1, &m_rect, DT_FLAGS);
   }

   m_totalHeight += m_rect.Height();

   m_rects.push_back(m_rect);

   m_rect.top = m_rect.bottom;
   m_rect.right = m_rightSide;
}

////////////////////////////////////////

const std::vector<CRect> & cFooWndItemRender::GetRects() const
{
   return m_rects;
}

////////////////////////////////////////

int cFooWndItemRender::GetTotalHeight() const
{
   return m_totalHeight;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFooWnd
//

////////////////////////////////////////

cFooWnd::cFooWnd()
{
}

////////////////////////////////////////

tResult cFooWnd::AddString(const tChar * pszString,
                           size_t length /*=-1*/,
                           COLORREF color /*=CLR_INVALID*/)
{
   if (pszString == NULL)
   {
      return E_POINTER;
   }

   std::string s(pszString);
   TrimTrailingSpace(&s);

   m_items.push_back(cFooWndItem(s, color));

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

tResult cFooWnd::Clear()
{
   m_items.clear();

   UpdateScrollInfo();
   Invalidate();

   return S_OK;
}

////////////////////////////////////////

BOOL cFooWnd::PreTranslateMessage(MSG * pMsg)
{
   return FALSE;
}

////////////////////////////////////////

void cFooWnd::DoPaint(WTL::CDCHandle dc)
{
   std::for_each(m_items.begin(), m_items.end(), cFooWndItemRender(m_hWnd, dc));
}

////////////////////////////////////////

void cFooWnd::UpdateScrollInfo()
{
   cFooWndItemRender result = std::for_each(m_items.begin(), m_items.end(),
      cFooWndItemRender(m_hWnd, WTL::CDCHandle(WTL::CDC(GetDC())), true));

   int totalHeight = result.GetTotalHeight();

   SetScrollSize(1, totalHeight);
}

///////////////////////////////////////////////////////////////////////////////
