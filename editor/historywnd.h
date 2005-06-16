///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_HISTORYWND_H
#define INCLUDED_HISTORYWND_H

#include "scrollwnd.h"

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cHistoryWndItem
//

class cHistoryWndItem
{
public:
   cHistoryWndItem(LPCTSTR pszText, int textLength, COLORREF textColor);
   cHistoryWndItem(const cHistoryWndItem & other);
   ~cHistoryWndItem();

   const cHistoryWndItem & operator =(const cHistoryWndItem & other);

   const CString & GetText() const;
   int GetTextLen() const;
   COLORREF GetTextColor() const;

private:
   CString m_text;
   COLORREF m_textColor;
};

///////////////////////////////////////

typedef std::vector<cHistoryWndItem *> tHistoryWndItems;

///////////////////////////////////////

inline const CString & cHistoryWndItem::GetText() const
{
   return m_text;
}

///////////////////////////////////////

inline int cHistoryWndItem::GetTextLen() const
{
   return GetText().GetLength();
}

///////////////////////////////////////

inline COLORREF cHistoryWndItem::GetTextColor() const
{
   return m_textColor;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cHistoryWndHitTestInfo
//

class cHistoryWndHitTestInfo
{
public:
   cHistoryWndHitTestInfo();
   cHistoryWndHitTestInfo(const cHistoryWndHitTestInfo & other);
   ~cHistoryWndHitTestInfo();

   const cHistoryWndHitTestInfo & operator =(const cHistoryWndHitTestInfo & other);

   void Reset();

   int iItem; // index of the item hit
   int iChar; // index of the character within the item that was hit
   CRect rect;
   int charX;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cHistoryWnd
//

class cHistoryWnd : public CScrollWnd
{
   DECLARE_DYNCREATE_EX(cHistoryWnd)

public:
   cHistoryWnd();
   virtual ~cHistoryWnd();

   void AddText(LPCTSTR pszText, int textLength = -1, COLORREF textColor = CLR_INVALID);
   void Clear();

   COLORREF SetTextColor(COLORREF color);
   COLORREF GetTextColor() const;

   COLORREF SetBkColor(COLORREF color);
   COLORREF GetBkColor() const;

   void SetMaxEntries(int max);
   size_t GetMaxEntries() const;

   void SetFont(CFont * pFont, BOOL bRedraw = TRUE);

   bool CopySelection();
   bool GetSelection(CString * pSel);

protected:
   void EnforceMaxEntries();
   void UpdateScrollInfo();
   void UpdateFontMetrics();
   bool HitTest(const CPoint & point, cHistoryWndHitTestInfo * pHitTest);
   bool HitTestSelection(const CPoint & point);
   void ClearSel();
   void UpdateSelDrag(const CPoint & point);
   void GetVisibleRange(int * pStart, int * pEnd);
   virtual void AddContextMenuItems(CMenu * pMenu);

#ifdef _DEBUG
   virtual void AssertValid() const;
   virtual void Dump(CDumpContext& dc) const;
#endif

   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(cHistoryWnd)
   //}}AFX_VIRTUAL

   DECLARE_MESSAGE_MAP()

   // Generated message map functions
   //{{AFX_MSG(cHistoryWnd)
   afx_msg void OnPaint();
   afx_msg BOOL OnEraseBkgnd(CDC* pDC);
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
   afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
   afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
   afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
   afx_msg void OnMouseMove(UINT nFlags, CPoint point);
   afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
   afx_msg void OnEditCopy();
   afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
   afx_msg void OnEditClear();
   afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

   LRESULT OnSetFont(WPARAM wParam, LPARAM lParam);

private:
   COLORREF m_textColor, m_bkColor;
   tHistoryWndItems::size_type m_nMaxEntries;
   tHistoryWndItems m_entries;

   CFont m_font;
   int m_nCharHeight; // height of the tallest character in the current font (I think)
   int m_nAvgCharWidth; // average width of a character in the current font
   int m_nMaxEntryLen; // strlen of the longest entry in the list
   int m_maxEntrySize; // horizontal size of the entry with the largest font dimensions
   bool m_bAtEnd; // is the window scrolled to the very bottom?
   int m_nAddsSinceLastPaint;

   cHistoryWndHitTestInfo m_startSel;
   cHistoryWndHitTestInfo m_endSel;
   cHistoryWndHitTestInfo * m_pSelAnchor;
   cHistoryWndHitTestInfo * m_pSelDrag;
   bool m_bDragGTEAnchor; // is drag point >= (i.e. right and bottom of) anchor point?
};

///////////////////////////////////////

inline COLORREF cHistoryWnd::SetTextColor(COLORREF color) 
{
   COLORREF c = m_textColor;
   m_textColor = color;
   return c;
}

///////////////////////////////////////

inline COLORREF cHistoryWnd::GetTextColor() const
{
   return m_textColor;
}

///////////////////////////////////////

inline COLORREF cHistoryWnd::SetBkColor(COLORREF color) 
{
   COLORREF c = m_bkColor;
   m_bkColor = color;
   return c;
}

///////////////////////////////////////

inline COLORREF cHistoryWnd::GetBkColor() const
{
   return m_bkColor;
}

///////////////////////////////////////

inline size_t cHistoryWnd::GetMaxEntries() const
{
   return m_nMaxEntries;
}

///////////////////////////////////////////////////////////////////////////////

#endif /* !INCLUDED_HISTORYWND_H */
