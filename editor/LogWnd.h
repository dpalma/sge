///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_LOGWND_H
#define INCLUDED_LOGWND_H

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
// CLASS: cLogWndItem
//

class cLogWndItem
{
public:
   cLogWndItem();
   explicit cLogWndItem(const std::string & string, const std::string & detail = "",
      eLogSeverity severity = kInfo);
   cLogWndItem(const cLogWndItem & other);

   ~cLogWndItem();

   const cLogWndItem & operator =(const cLogWndItem & other);

   const tChar * GetString() const;
   bool HasDetail() const;
   const tChar * GetDetail() const;
   eLogSeverity GetSeverity() const;
   bool IsExpanded() const;
   void ToggleExpand();

private:
   std::string m_string;
   std::string m_detail;
   eLogSeverity m_severity;
   bool m_bExpanded;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLogWndItemRender
//

class cLogWndItemRender
{
   enum { DT_FLAGS = DT_LEFT | DT_TOP | DT_WORDBREAK };

public:
   cLogWndItemRender(CWindow wnd, WTL::CDCHandle dc, bool bCalcOnly = false);

   cLogWndItemRender(const cLogWndItemRender & other);

   ~cLogWndItemRender();

   const cLogWndItemRender & operator =(const cLogWndItemRender & other);

   void operator ()(const cLogWndItem & item);

   void Invoke(const cLogWndItem & item);

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
// CLASS: cLogWnd
//

class cLogWnd : public WTL::CScrollWindowImpl<cLogWnd>
{
   typedef WTL::CScrollWindowImpl<cLogWnd> tBase;

   enum { kMaxItems = 1000 };

public:
   DECLARE_WND_CLASS(NULL)

   cLogWnd();

   tResult AddString(const tChar * pszString, size_t length = -1, COLORREF color = CLR_INVALID);
   tResult Clear();

   BOOL PreTranslateMessage(MSG * pMsg);

   BEGIN_MSG_MAP(cLogWnd)
      CHAIN_MSG_MAP(tBase)
   END_MSG_MAP()

   void DoPaint(WTL::CDCHandle dc);

protected:
   void UpdateScrollInfo();

private:
   typedef std::vector<cLogWndItem> tItems;
   tItems m_items;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_LOGWND_H
