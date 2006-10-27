/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ImageAttributesDlg.h"

#include "tech/imageapi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cImageAttributesDlg
//

////////////////////////////////////////

cImageAttributesDlg::cImageAttributesDlg(CWnd* pParent /*=NULL*/)
 : CDialog(cImageAttributesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(cImageAttributesDlg)
   m_pixelFormat = kPF_RGBA8888;
   m_width = 256;
   m_height = 256;
	//}}AFX_DATA_INIT
}

void cImageAttributesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(cImageAttributesDlg)
   DDX_CBIndex(pDX, IDC_PIXEL_FORMAT, m_pixelFormat);
	DDX_Text(pDX, IDC_WIDTH, m_width);
	DDX_Text(pDX, IDC_HEIGHT, m_height);
	DDV_MinMaxInt(pDX, m_width, 1, 1024);
	DDV_MinMaxInt(pDX, m_height, 1, 1024);
	//}}AFX_DATA_MAP
}

////////////////////////////////////////

BEGIN_MESSAGE_MAP(cImageAttributesDlg, CDialog)
	//{{AFX_MSG_MAP(cImageAttributesDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cImageAttributesDlg message handlers

BOOL cImageAttributesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

   static const struct
   {
      ePixelFormat pixelFormat;
      LPCTSTR pszDescription;
   }
   pixelFormatDescriptions[] =
   {
      { kPF_Grayscale,     _T("Grayscale")   },
      { kPF_ColorMapped,   _T("ColorMapped") },
      { kPF_RGB555,        _T("RGB555")      },
      { kPF_BGR555,        _T("BGR555")      },
      { kPF_RGB565,        _T("RGB565")      },
      { kPF_BGR565,        _T("BGR565")      },
      { kPF_RGBA1555,      _T("RGBA1555")    },
      { kPF_BGRA1555,      _T("BGRA1555")    },
      { kPF_RGB888,        _T("RGB888")      },
      { kPF_BGR888,        _T("BGR888")      },
      { kPF_RGBA8888,      _T("RGBA8888")    },
      { kPF_BGRA8888,      _T("BGRA8888")    },
   };

   for (int i = 0; i < _countof(pixelFormatDescriptions); ++i)
   {
      ePixelFormat pf = pixelFormatDescriptions[i].pixelFormat;
      LPCTSTR psz = pixelFormatDescriptions[i].pszDescription;
      SendDlgItemMessage(IDC_PIXEL_FORMAT, CB_ADDSTRING, 0, (LPARAM)psz);
   }

   UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////
