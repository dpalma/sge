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
   void UpdateHorizontalExtent(HDC hDC);
   int GetHorizontalExtent() const;

private:
   CString m_text;
   COLORREF m_textColor;
   int m_horizontalExtent;
};

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

///////////////////////////////////////

typedef std::vector<cLogWndLine *> tLogWndLines;


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

   bool operator <(const cLogWndLocation & other) const;
   bool operator >(const cLogWndLocation & other) const;
   bool operator <=(const cLogWndLocation & other) const;
   bool operator >=(const cLogWndLocation & other) const;
   bool operator ==(const cLogWndLocation & other) const;

   int iLine; // index of the line hit
   int iChar; // index of the character within the line that was hit
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLogWndSelection
//

class cLogWndSelection
{
   cLogWndSelection(const cLogWndSelection & other);
   void operator =(const cLogWndSelection & other);

public:
   cLogWndSelection();
   ~cLogWndSelection();

   void Clear();
   bool IsEmpty() const;

   bool IsWithinSelection(const cLogWndLocation & loc) const;

   const cLogWndLocation & GetStart() const;
   const cLogWndLocation & GetEnd() const;

   void DragSelectBegin(const cLogWndLocation & loc);
   void DragSelectUpdate(const cLogWndLocation & loc);
   void DragSelectEnd();
   void DragSelectCancel();

   bool GetSelection(const tLogWndLines & lines, CString * pSel) const;

private:
   cLogWndLocation m_startSel, m_endSel;
   cLogWndLocation m_savedStartSel, m_savedEndSel;
   cLogWndLocation * m_pSelAnchor;
   cLogWndLocation * m_pSelDrag;
   bool m_bDragGTEAnchor; // is drag point >= (i.e. right and bottom of) anchor point?
};

////////////////////////////////////////

inline const cLogWndLocation & cLogWndSelection::GetStart() const
{
   return m_startSel;
}

////////////////////////////////////////

inline const cLogWndLocation & cLogWndSelection::GetEnd() const
{
   return m_endSel;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLogWndImpl
//

template <typename T, typename TBase = CWindow, typename TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE cLogWndImpl : public WTL::CScrollWindowImpl<T, TBase, TWinTraits>
{
   typedef WTL::CScrollWindowImpl<T, TBase, TWinTraits> tBase;

public:
	DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

   cLogWndImpl();
   ~cLogWndImpl();

   COLORREF SetTextColor(COLORREF textColor);
   COLORREF GetTextColor() const;

   COLORREF SetBkColor(COLORREF bkColor);
   COLORREF GetBkColor() const;

   bool SetFont(HFONT hFont, BOOL bRedraw = TRUE);
   WTL::CFontHandle GetFont() const;

protected:
   void OnSetFont(HFONT hFont, BOOL bRedraw);

   BEGIN_MSG_MAP_EX(cLogWndImpl)
      CHAIN_MSG_MAP(tBase)
      MSG_WM_SETFONT(OnSetFont)
   END_MSG_MAP()

private:

   COLORREF m_textColor, m_bkColor;

   WTL::CFont m_font;
};

////////////////////////////////////////

template <typename T, typename TBase, typename TWinTraits>
cLogWndImpl<T, TBase, TWinTraits>::cLogWndImpl()
 : m_textColor(CLR_INVALID)
 , m_bkColor(CLR_INVALID)
{
}

////////////////////////////////////////

template <typename T, typename TBase, typename TWinTraits>
cLogWndImpl<T, TBase, TWinTraits>::~cLogWndImpl()
{
}

////////////////////////////////////////

template <typename T, typename TBase, typename TWinTraits>
inline COLORREF cLogWndImpl<T, TBase, TWinTraits>::SetTextColor(COLORREF textColor) 
{
   COLORREF c = m_textColor;
   m_textColor = textColor;
   return c;
}

////////////////////////////////////////

template <typename T, typename TBase, typename TWinTraits>
inline COLORREF cLogWndImpl<T, TBase, TWinTraits>::GetTextColor() const
{
   return (m_textColor == CLR_INVALID) ? GetSysColor(COLOR_WINDOWTEXT) : m_textColor;
}

////////////////////////////////////////

template <typename T, typename TBase, typename TWinTraits>
inline COLORREF cLogWndImpl<T, TBase, TWinTraits>::SetBkColor(COLORREF bkColor) 
{
   COLORREF c = m_bkColor;
   m_bkColor = bkColor;
   return c;
}

////////////////////////////////////////

template <typename T, typename TBase, typename TWinTraits>
inline COLORREF cLogWndImpl<T, TBase, TWinTraits>::GetBkColor() const
{
   return (m_bkColor == CLR_INVALID) ? GetSysColor(COLOR_WINDOW) : m_bkColor;
}

////////////////////////////////////////

template <typename T, typename TBase, typename TWinTraits>
bool cLogWndImpl<T, TBase, TWinTraits>::SetFont(HFONT hFont, BOOL bRedraw)
{
   LOGFONT logFont = {0};
   if (GetObject(hFont, sizeof(LOGFONT), &logFont))
   {
      WTL::CFont newFont;
      if (newFont.CreateFontIndirect(&logFont))
      {
         m_font.Attach(newFont.Detach());

         T * pT = static_cast<T*>(this);
         pT->UpdateFontMetrics();
         pT->UpdateScrollInfo();

         if (bRedraw)
         {
            Invalidate();
         }

         return true;
      }
   }
   return false;
}

////////////////////////////////////////

template <typename T, typename TBase, typename TWinTraits>
WTL::CFontHandle cLogWndImpl<T, TBase, TWinTraits>::GetFont() const
{
   return WTL::CFontHandle(m_font);
}

////////////////////////////////////////

template <typename T, typename TBase, typename TWinTraits>
void cLogWndImpl<T, TBase, TWinTraits>::OnSetFont(HFONT hFont, BOOL bRedraw)
{
   SetFont(hFont, bRedraw);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLogWnd
//

class cLogWnd : public cLogWndImpl<cLogWnd>
{
   typedef cLogWndImpl<cLogWnd> tBase;

public:
   cLogWnd();
   ~cLogWnd();

   DECLARE_WND_CLASS(NULL)

   void AddText(LPCTSTR pszText, int textLength = -1, COLORREF textColor = CLR_INVALID);
   void Clear();

   void SetMaxLines(int nMaxLines);
   uint GetMaxLines() const;

   bool CopySelection();
   bool GetSelection(CString * pSel);

   // CScrollWindowImpl handles WM_PAINT and delegates to this method
   void DoPaint(WTL::CDCHandle dc);

   void DoScroll(int nType, int nScrollCode, int& cxyOffset, int cxySizeAll, int cxySizePage, int cxySizeLine);

   void UpdateScrollInfo();
   void UpdateFontMetrics();

protected:
   void EnforceMaxLines();
   bool GetHitLocation(const CPoint & point, cLogWndLocation * pHitTest);
   bool GetHitLocation(const CPoint & point, int * piLine, int * piChar) const;
   bool GetHitLine(const CPoint & point, int * piLine) const;
   void GetVisibleRange(int * pStart, int * pEnd);
   virtual void AddContextMenuItems(WTL::CMenu * pMenu);

   BEGIN_MSG_MAP_EX(cLogWnd)
      CHAIN_MSG_MAP(tBase)
      MSG_WM_CREATE(OnCreate)
      MSG_WM_DESTROY(OnDestroy)
      MSG_WM_ERASEBKGND(OnEraseBkgnd)
      MSG_WM_CONTEXTMENU(OnContextMenu)
      MSG_WM_SETCURSOR(OnSetCursor)
      MSG_WM_MOUSEMOVE(OnMouseMove)
      MSG_WM_LBUTTONDOWN(OnLButtonDown)
      MSG_WM_LBUTTONUP(OnLButtonUp)
      MSG_WM_CANCELMODE(OnCancelMode)
      COMMAND_ID_HANDLER_EX(ID_EDIT_COPY, OnEditCopy)
      COMMAND_ID_HANDLER_EX(ID_EDIT_CLEAR_ALL, OnEditClearAll)
   END_MSG_MAP()

   LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct);
   void OnDestroy();
   LRESULT OnEraseBkgnd(WTL::CDCHandle dc);
   void OnContextMenu(HWND hWnd, CPoint point);
   LRESULT OnSetCursor(HWND hWnd, UINT hitTest, UINT message);
   void OnMouseMove(UINT flags, CPoint point);
   void OnLButtonDown(UINT flags, CPoint point);
   void OnLButtonUp(UINT flgs, CPoint point);
   void OnCancelMode();
   void OnEditCopy(UINT code, int id, HWND hWnd);
   void OnEditClearAll(UINT code, int id, HWND hWnd);

private:
   tLogWndLines::size_type m_nMaxLines;
   tLogWndLines m_lines;

   int m_lineHeight; // height of a single line; determined by the selected font
   int m_maxLineHorizontalExtent; // horizontal size of the entry with the largest font dimensions
   bool m_bAtEnd; // is the window scrolled to the very bottom?
   int m_nAddsSinceLastPaint;

   cLogWndSelection m_selection;
};

///////////////////////////////////////

inline uint cLogWnd::GetMaxLines() const
{
   return m_nMaxLines;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_LOGWND_H
