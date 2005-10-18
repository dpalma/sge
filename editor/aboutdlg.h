/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_ABOUTDLG_H)
#define INCLUDED_ABOUTDLG_H

#include "resource.h"       // main symbols

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace WTL { class CHyperLink; }

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: CAboutDlg
//

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();
   ~CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
   afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()

private:
   std::vector<WTL::CHyperLink*> m_hyperLinks;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_ABOUTDLG_H)
