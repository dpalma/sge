/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_TOOLPALETTEBAR_H
#define INCLUDED_TOOLPALETTEBAR_H

#include "afxcomtools.h"
#include "editorapi.h"

#include <vector>

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cButtonPanel
//

class cButtonPanel
{
public:
   cButtonPanel();
   ~cButtonPanel();

   void AddButton(CButton * pButton);

   void Clear();

   void Reposition(LPCRECT pRect, BOOL bRepaint = TRUE);

   void HandleClick(uint buttonId);

   void SetMargins(LPCRECT pMargins);

private:
   typedef std::vector<CButton *> tButtons;
   tButtons m_buttons;

   CRect m_margins;
};

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolPaletteBar
//

class cToolPaletteBar : public CSizingControlBarG,
                        public cComObject<IMPLEMENTS(IEditorTileManagerListener),
                                          cAfxComServices<cToolPaletteBar> >
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

   virtual void OnDefaultTileSetChange(IEditorTileSet * pTileSet);

protected:

   // Generated message map functions
protected:
   //{{AFX_MSG(cToolPaletteBar)
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
   afx_msg void OnButtonClicked(uint buttonId);
   DECLARE_MESSAGE_MAP()

   void ClearButtons();
   void RepositionButtons(BOOL bRepaint = TRUE);

private:
   cButtonPanel m_buttonPanel;

   CToolTipCtrl m_tooltip;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !INCLUDED_TOOLPALETTEBAR_H
