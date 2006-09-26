// ImageGammaDlg.cpp : implementation file
//

#include "stdhdr.h"
#include "ImageGammaDlg.h"


// cImageGammaDlg dialog

IMPLEMENT_DYNAMIC(cImageGammaDlg, CDialog)

cImageGammaDlg::cImageGammaDlg(CWnd* pParent /*=NULL*/)
	: CDialog(cImageGammaDlg::IDD, pParent)
   , m_bApplyGamma(FALSE)
   , m_gamma(0)
{

}

cImageGammaDlg::~cImageGammaDlg()
{
}

void cImageGammaDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Check(pDX, IDC_APPLY_GAMMA, m_bApplyGamma);
   DDX_Text(pDX, IDC_GAMMA_VALUE, m_gamma);
	DDV_MinMaxFloat(pDX, m_gamma, .2, 5);
}


BEGIN_MESSAGE_MAP(cImageGammaDlg, CDialog)
END_MESSAGE_MAP()


// cImageGammaDlg message handlers
