///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_LOGWND_H
#define INCLUDED_LOGWND_H

#include <atlscrl.h>

#include <vector>
#include <string>

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
   cLogWndItem(eLogSeverity severity, const std::string & string);
   cLogWndItem(const cLogWndItem & other);

   ~cLogWndItem();

   const cLogWndItem & operator =(const cLogWndItem & other);

   eLogSeverity GetSeverity() const;
   const tChar * GetString() const;

private:
   eLogSeverity m_severity;
   std::string m_string;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLogWndItemRender
//

class cLogWndItemRender
{
public:
   enum
   {
      kLeftColumnWidth = 18,
   };

   cLogWndItemRender(HDC hDC, const CRect & startRect, bool bCalcOnly = false);
   cLogWndItemRender(const cLogWndItemRender & other);
   ~cLogWndItemRender();

   const cLogWndItemRender & operator =(const cLogWndItemRender & other);

   void Render(const cLogWndItem & item);

   void RenderLeftColumn(const CRect & rect);

   int GetTotalHeight() const;

private:
   CDCHandle m_dc;
   CRect m_rect;
   bool m_bCalcOnly;
   int m_rightSide;
   int m_totalHeight;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLogWnd
//

typedef CWinTraitsOR<0, WS_EX_CLIENTEDGE> tLogWndTraits;

class cLogWnd : public CScrollWindowImpl<cLogWnd, CWindow, tLogWndTraits>
{
   typedef CScrollWindowImpl<cLogWnd, CWindow, tLogWndTraits> tBase;

   enum { kMaxItems = 1000 };

public:
   DECLARE_WND_CLASS("LogWnd")

   cLogWnd();
   ~cLogWnd();

   tResult AddString(const tChar * pszString, size_t length = -1);
   tResult AddString(eLogSeverity severity, const tChar * pszString, size_t length = -1);
   tResult Clear();

   BEGIN_MSG_MAP_EX(cLogWnd)
      CHAIN_MSG_MAP(tBase)
      MSG_WM_DESTROY(OnDestroy)
      MSG_WM_SETFONT(OnSetFont)
      MSG_WM_SETCURSOR(OnSetCursor)
      MSG_WM_MOUSEMOVE(OnMouseMove)
      MSG_WM_LBUTTONDOWN(OnLButtonDown)
      MSG_WM_LBUTTONUP(OnLButtonUp)
   END_MSG_MAP()

   void DoPaint(CDCHandle dc);

   void OnDestroy();
   void OnSetFont(HFONT hFont, BOOL bRedraw);
   LRESULT OnSetCursor(HWND hWnd, UINT hitTest, UINT message);
   void OnMouseMove(UINT flags, CPoint point);
   void OnLButtonDown(UINT flags, CPoint point);
   void OnLButtonUp(UINT flags, CPoint point);

protected:
   void UpdateScrollInfo();

private:
   typedef std::vector<cLogWndItem> tItems;
   tItems m_items;

   CFont m_font;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_LOGWND_H
