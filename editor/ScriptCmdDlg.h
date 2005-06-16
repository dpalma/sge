/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_SCRIPTCMDDLG_H)
#define INCLUDED_SCRIPTCMDDLG_H

#include "resource.h"       // main symbols

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cScriptCmdDlg
//

class cScriptCmdDlg : public CDialog
{
public:
	cScriptCmdDlg(CWnd * pParentWnd);

   CString m_scriptCommand;

   static CString m_lastScriptCommand;

// Dialog Data
	//{{AFX_DATA(cScriptCmdDlg)
	enum { IDD = IDD_SCRIPTCOMMAND };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cScriptCmdDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(cScriptCmdDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_SCRIPTCMDDLG_H)
