/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "CameraDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cCameraDlg
//

////////////////////////////////////////

BEGIN_MESSAGE_MAP(cCameraDlg, CDialog)
	//{{AFX_MSG_MAP(cCameraDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

////////////////////////////////////////

cCameraDlg::cCameraDlg(CWnd * pParentWnd)
 : CDialog(cCameraDlg::IDD, pParentWnd)
{
	//{{AFX_DATA_INIT(cCameraDlg)
	//}}AFX_DATA_INIT
}

////////////////////////////////////////

void cCameraDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(cCameraDlg)
   DDX_Text(pDX, IDC_FOV, m_fov);
   DDX_Text(pDX, IDC_ZNEAR, m_znear);
   DDX_Text(pDX, IDC_ZFAR, m_zfar);
   DDX_Text(pDX, IDC_ELEVATION, m_elevation);
   DDX_Text(pDX, IDC_PITCH, m_pitch);
	//}}AFX_DATA_MAP

   if (!pDX->m_bSaveAndValidate)
   {
      SendDlgItemMessage(IDC_SPIN_FOV, UDM_SETRANGE32, 0, 120);
      SendDlgItemMessage(IDC_SPIN_ZNEAR, UDM_SETRANGE32, 0, 5000);
      SendDlgItemMessage(IDC_SPIN_ZFAR, UDM_SETRANGE32, 0, 5000);
      SendDlgItemMessage(IDC_SPIN_ELEVATION, UDM_SETRANGE32, 0, 200);
      SendDlgItemMessage(IDC_SPIN_PITCH, UDM_SETRANGE32, 0, 90);
   }
}

////////////////////////////////////////

void cCameraDlg::OnOK()
{
   CDialog::OnOK();
}

////////////////////////////////////////

BOOL cCameraDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////
