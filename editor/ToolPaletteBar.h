/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_TOOLPALETTEBAR_H
#define INCLUDED_TOOLPALETTEBAR_H

#include "afxcomtools.h"
#include "editorapi.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

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
	//}}AFX_MSG
   DECLARE_MESSAGE_MAP()

private:
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !INCLUDED_TOOLPALETTEBAR_H
