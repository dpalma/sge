///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_FOOWND_H
#define INCLUDED_FOOWND_H

#include <atlgdi.h>
#include <atlscrl.h>

#include <vector>
#include <string>
#include <algorithm>

#if _MSC_VER > 1000
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

inline void TrimTrailingSpace(std::string * pString)
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
// CLASS: cFooWndItemRender
//

class cFooWndItemRender
{
   enum { DT_FLAGS = DT_LEFT | DT_TOP | DT_WORDBREAK };

public:
   cFooWndItemRender(CWindow wnd, WTL::CDCHandle dc, bool bCalcOnly = false);

   cFooWndItemRender(const cFooWndItemRender & other);

   ~cFooWndItemRender();

   const cFooWndItemRender & operator =(const cFooWndItemRender & other);

   operator ()(const std::string & s);

   const std::vector<CRect> & GetRects() const;

   int GetTotalHeight() const;

private:
   WTL::CDCHandle m_dc;
   HFONT m_hOldFont;
   CRect m_rect;
   bool m_bCalcOnly;
   int m_rightSide;
   std::vector<CRect> m_rects;
   int m_totalHeight;
};

////////////////////////////////////////

inline cFooWndItemRender::cFooWndItemRender(CWindow wnd, WTL::CDCHandle dc, bool bCalcOnly)
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

inline cFooWndItemRender::cFooWndItemRender(const cFooWndItemRender & other)
 : m_hOldFont(NULL)
{
   operator =(other);
}

////////////////////////////////////////

inline cFooWndItemRender::~cFooWndItemRender()
{
   if (m_hOldFont != NULL)
   {
      m_dc.SelectFont(m_hOldFont);
      m_hOldFont = NULL;
   }
}

////////////////////////////////////////

inline const cFooWndItemRender & cFooWndItemRender::operator =(const cFooWndItemRender & other)
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

inline cFooWndItemRender::operator ()(const std::string & s)
{
   m_dc.DrawText(s.c_str(), s.length(), &m_rect, DT_FLAGS | DT_CALCRECT);
   if (!m_bCalcOnly)
   {
      m_dc.DrawText(s.c_str(), s.length(), &m_rect, DT_FLAGS);
   }
   m_totalHeight += m_rect.Height();
   m_rects.push_back(m_rect);
   m_rect.top = m_rect.bottom;
   m_rect.right = m_rightSide;
}

////////////////////////////////////////

inline const std::vector<CRect> & cFooWndItemRender::GetRects() const
{
   return m_rects;
}

////////////////////////////////////////

inline int cFooWndItemRender::GetTotalHeight() const
{
   return m_totalHeight;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFooWnd
//

class cFooWnd : public WTL::CScrollWindowImpl<cFooWnd>
{
   typedef WTL::CScrollWindowImpl<cFooWnd> tBase;

   enum { kMaxEntries = 1000 };

public:
   DECLARE_WND_CLASS(NULL)

   cFooWnd();

   tResult AddString(const tChar * pszString, size_t length = -1, COLORREF color = CLR_INVALID);
   tResult Clear();

   BOOL PreTranslateMessage(MSG * pMsg);

   BEGIN_MSG_MAP(cFooWnd)
      CHAIN_MSG_MAP(tBase)
   END_MSG_MAP()

   void DoPaint(WTL::CDCHandle dc);

protected:
   void UpdateScrollInfo();

private:
   typedef std::vector<std::string> tStrings;
   tStrings m_strings;
};

////////////////////////////////////////

inline cFooWnd::cFooWnd()
{
}

////////////////////////////////////////

inline tResult cFooWnd::AddString(const tChar * pszString,
                                  size_t length /*=-1*/,
                                  COLORREF color /*=CLR_INVALID*/)
{
   if (pszString == NULL)
   {
      return E_POINTER;
   }

   std::string s(pszString);
   TrimTrailingSpace(&s);

   m_strings.push_back(s);

   while (m_strings.size() > kMaxEntries)
   {
      m_strings.erase(m_strings.begin());
   }

   UpdateScrollInfo();
   ScrollBottom();
   Invalidate();

   return S_OK;
}

////////////////////////////////////////

inline tResult cFooWnd::Clear()
{
   m_strings.clear();

   UpdateScrollInfo();
   Invalidate();

   return S_OK;
}

////////////////////////////////////////

inline BOOL cFooWnd::PreTranslateMessage(MSG * pMsg)
{
   return FALSE;
}

////////////////////////////////////////

inline void cFooWnd::DoPaint(WTL::CDCHandle dc)
{
   std::for_each(m_strings.begin(), m_strings.end(), cFooWndItemRender(m_hWnd, dc));
}

////////////////////////////////////////

inline void cFooWnd::UpdateScrollInfo()
{
   cFooWndItemRender result = std::for_each(m_strings.begin(),
      m_strings.end(), cFooWndItemRender(m_hWnd, WTL::CDCHandle(WTL::CDC(GetDC())), true));

   int totalHeight = result.GetTotalHeight();

   SetScrollSize(1, totalHeight);
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_FOOWND_H
