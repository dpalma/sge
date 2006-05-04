/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_TOOLPALETTEBAR_H
#define INCLUDED_TOOLPALETTEBAR_H

#include "ToolPalette.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolPaletteBar
//

class cToolPaletteBar : public cEditorControlBar
{
   DECLARE_DYNCREATE(cToolPaletteBar)

// Construction
public:
   cToolPaletteBar();

// Attributes
public:

// Operations
public:

// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(cToolPaletteBar)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
   virtual ~cToolPaletteBar();

   // Generated message map functions
protected:
   //{{AFX_MSG(cToolPaletteBar)
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
   afx_msg void OnToolPaletteItemCheck(NMHDR * pNmhdr, LRESULT * pResult);
   afx_msg void OnToolPaletteItemDestroy(NMHDR * pNmhdr, LRESULT * pResult);
   DECLARE_MESSAGE_MAP()

   HTOOLGROUP CreateStandardToolGroup();

private:
   CToolTipCtrl m_tooltip;
   cToolPalette m_toolPalette;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !INCLUDED_TOOLPALETTEBAR_H
