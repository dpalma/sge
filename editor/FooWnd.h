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
//
// CLASS: cFooWndItem
//

class cFooWndItem
{
public:
   cFooWndItem();
   explicit cFooWndItem(const tChar * pszString, size_t length = -1, COLORREF color = CLR_INVALID);
   explicit cFooWndItem(const std::string & string, COLORREF color = CLR_INVALID);
   cFooWndItem(const cFooWndItem & other);

   ~cFooWndItem();

   const cFooWndItem & operator =(const cFooWndItem & other);

   const tChar * GetString() const;
   COLORREF GetColor() const;

private:
   std::string m_string;
   COLORREF m_color;
};


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

   void operator ()(const cFooWndItem & item);

   void Invoke(const cFooWndItem & item);

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


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFooWnd
//

class cFooWnd : public WTL::CScrollWindowImpl<cFooWnd>
{
   typedef WTL::CScrollWindowImpl<cFooWnd> tBase;

   enum { kMaxItems = 1000 };

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
   typedef std::vector<cFooWndItem> tItems;
   tItems m_items;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_FOOWND_H
