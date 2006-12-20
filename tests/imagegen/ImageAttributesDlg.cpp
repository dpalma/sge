/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ImageAttributesDlg.h"

#include "tech/imageapi.h"


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cImageAttributesDlg
//

////////////////////////////////////////

cImageAttributesDlg::cImageAttributesDlg()
 : m_pixelFormat(kPF_RGBA8888)
 , m_width(256)
 , m_height(256)
{
}

/////////////////////////////////////////////////////////////////////////////
// cImageAttributesDlg message handlers

LRESULT cImageAttributesDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   CenterWindow(GetParent());

   DoDataExchange(FALSE);

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

   int iSel = -1;
   for (int i = 0; i < _countof(pixelFormatDescriptions); ++i)
   {
      ePixelFormat pf = pixelFormatDescriptions[i].pixelFormat;
      LPCTSTR psz = pixelFormatDescriptions[i].pszDescription;
      int iItem = m_pixelFormatComboBox.AddString(psz);
      if (iItem != CB_ERR)
         m_pixelFormatComboBox.SetItemData(iItem, pf);
      if (pf == m_pixelFormat)
         iSel = i;
   }
   m_pixelFormatComboBox.SetCurSel(iSel);

   return TRUE;
}

LRESULT cImageAttributesDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   if (DoDataExchange(TRUE))
   {
      EndDialog(wID);
   }
   return 0;
}

LRESULT cImageAttributesDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   EndDialog(wID);
   return 0;
}

/////////////////////////////////////////////////////////////////////////////
