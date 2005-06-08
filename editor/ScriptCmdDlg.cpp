/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ScriptCmdDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cScriptCmdDlg
//

////////////////////////////////////////

BEGIN_MESSAGE_MAP(cScriptCmdDlg, CDialog)
	//{{AFX_MSG_MAP(cScriptCmdDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

////////////////////////////////////////

cScriptCmdDlg::cScriptCmdDlg(CWnd * pParentWnd)
 : CDialog(cScriptCmdDlg::IDD, pParentWnd)
{
	//{{AFX_DATA_INIT(cScriptCmdDlg)
	//}}AFX_DATA_INIT
}

////////////////////////////////////////

void cScriptCmdDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(cScriptCmdDlg)
	DDX_Text(pDX, IDC_SCRIPT_COMMAND, m_scriptCommand);
	//}}AFX_DATA_MAP
}

////////////////////////////////////////

BOOL cScriptCmdDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////
