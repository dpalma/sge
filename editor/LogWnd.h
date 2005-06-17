///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_LOGWND_H
#define INCLUDED_LOGWND_H

#include <atlgdi.h>
#include <atlcrack.h>
#include <atlscrl.h>

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLogWndLine
//

class cLogWndLine
{
public:
   cLogWndLine(LPCTSTR pszText, int textLength, COLORREF textColor);
   cLogWndLine(const cLogWndLine & other);
   ~cLogWndLine();

   const cLogWndLine & operator =(const cLogWndLine & other);

   const CString & GetText() const;
   int GetTextLen() const;
   COLORREF GetTextColor() const;

private:
   CString m_text;
   COLORREF m_textColor;
};

///////////////////////////////////////

typedef std::vector<cLogWndLine *> tLogWndLines;

///////////////////////////////////////

inline const CString & cLogWndLine::GetText() const
{
   return m_text;
}

///////////////////////////////////////

inline int cLogWndLine::GetTextLen() const
{
   return GetText().GetLength();
}

///////////////////////////////////////

inline COLORREF cLogWndLine::GetTextColor() const
{
   return m_textColor;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLogWndLocation
//

class cLogWndLocation
{
public:
   cLogWndLocation();
   cLogWndLocation(const cLogWndLocation & other);
   ~cLogWndLocation();

   const cLogWndLocation & operator =(const cLogWndLocation & other);

   int iLine; // index of the line hit
   int iChar; // index of the character within the line that was hit
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLogWnd
//

class cLogWnd : public WTL::CScrollWindowImpl<cLogWnd>
{
   typedef WTL::CScrollWindowImpl<cLogWnd> tBase;

public:
   cLogWnd();
   ~cLogWnd();

   DECLARE_WND_CLASS(NULL)

   void AddText(LPCTSTR pszText, int textLength = -1, COLORREF textColor = CLR_INVALID);
   void Clear();

   COLORREF SetTextColor(COLORREF textColor);
   COLORREF GetTextColor() const;

   COLORREF SetBkColor(COLORREF bkColor);
   COLORREF GetBkColor() const;

   void SetMaxLines(int nMaxLines);
   uint GetMaxLines() const;

   void SetFont(HFONT hFont, BOOL bRedraw = TRUE);

   bool CopySelection();
   bool GetSelection(CString * pSel);

   // CScrollWindowImpl handles WM_PAINT and delegates to this method
   void DoPaint(WTL::CDCHandle dc);

   void DoScroll(int nType, int nScrollCode, int& cxyOffset, int cxySizeAll, int cxySizePage, int cxySizeLine);

protected:
   void EnforceMaxLines();
   void UpdateScrollInfo();
   void UpdateFontMetrics();
   bool GetHitLocation(const CPoint & point, cLogWndLocation * pHitTest);
   bool GetHitLocation(const CPoint & point, int * piLine, int * piChar) const;
   bool GetHitLine(const CPoint & point, int * piLine) const;
   bool HitTestSelection(const CPoint & point);
   void ClearSel();
   void UpdateSelDrag(const CPoint & point);
   void GetVisibleRange(int * pStart, int * pEnd);
   virtual void AddContextMenuItems(WTL::CMenu * pMenu);

   BEGIN_MSG_MAP_EX(cLogWnd)
      CHAIN_MSG_MAP(tBase)
      MSG_WM_CREATE(OnCreate)
      MSG_WM_DESTROY(OnDestroy)
      MSG_WM_SETFONT(OnSetFont)
      MSG_WM_ERASEBKGND(OnEraseBkgnd)
      MSG_WM_CONTEXTMENU(OnContextMenu)
      MSG_WM_SETCURSOR(OnSetCursor)
      MSG_WM_MOUSEMOVE(OnMouseMove)
      MSG_WM_LBUTTONDOWN(OnLButtonDown)
      MSG_WM_LBUTTONUP(OnLButtonUp)
      COMMAND_ID_HANDLER_EX(ID_EDIT_COPY, OnEditCopy)
      COMMAND_ID_HANDLER_EX(ID_EDIT_CLEAR_ALL, OnEditClearAll)
   END_MSG_MAP()

   LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct);
   void OnDestroy();
   void OnSetFont(HFONT hFont, BOOL bRedraw);
   LRESULT OnEraseBkgnd(WTL::CDCHandle dc);
   void OnContextMenu(HWND hWnd, CPoint point);
   LRESULT OnSetCursor(HWND hWnd, UINT hitTest, UINT message);
   void OnMouseMove(UINT flags, CPoint point);
   void OnLButtonDown(UINT flags, CPoint point);
   void OnLButtonUp(UINT flags, CPoint point);
   void OnEditCopy(UINT code, int id, HWND hWnd);
   void OnEditClearAll(UINT code, int id, HWND hWnd);

private:
   COLORREF m_textColor, m_bkColor;
   tLogWndLines::size_type m_nMaxLines;
   tLogWndLines m_lines;

   WTL::CFont m_font;
   int m_lineHeight; // height of a single line; determined by the selected font
   int m_maxEntrySize; // horizontal size of the entry with the largest font dimensions
   bool m_bAtEnd; // is the window scrolled to the very bottom?
   int m_nAddsSinceLastPaint;

   cLogWndLocation m_startSel;
   cLogWndLocation m_endSel;
   cLogWndLocation * m_pSelAnchor;
   cLogWndLocation * m_pSelDrag;
   bool m_bDragGTEAnchor; // is drag point >= (i.e. right and bottom of) anchor point?
};

///////////////////////////////////////

inline COLORREF cLogWnd::SetTextColor(COLORREF textColor) 
{
   COLORREF c = m_textColor;
   m_textColor = textColor;
   return c;
}

///////////////////////////////////////

inline COLORREF cLogWnd::GetTextColor() const
{
   return m_textColor;
}

///////////////////////////////////////

inline COLORREF cLogWnd::SetBkColor(COLORREF bkColor) 
{
   COLORREF c = m_bkColor;
   m_bkColor = bkColor;
   return c;
}

///////////////////////////////////////

inline COLORREF cLogWnd::GetBkColor() const
{
   return m_bkColor;
}

///////////////////////////////////////

inline uint cLogWnd::GetMaxLines() const
{
   return m_nMaxLines;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_LOGWND_H
