/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_OUTPUTBAR_H
#define INCLUDED_OUTPUTBAR_H

#include "historywnd.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cOutputBar
//

class cOutputBar : public CSizingControlBarG
{
// Construction
public:
   cOutputBar();

// Attributes
public:

// Operations
public:

// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(cOutputBar)
   //}}AFX_VIRTUAL

// Implementation
public:
   virtual ~cOutputBar();

protected:
   CEdit m_wndChild;
   CFont m_font;

   // Generated message map functions
protected:
   //{{AFX_MSG(cOutputBar)
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !INCLUDED_OUTPUTBAR_H
