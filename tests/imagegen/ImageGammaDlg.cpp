// ImageGammaDlg.cpp : implementation file
//

#include "stdhdr.h"
#include "ImageGammaDlg.h"


// cImageGammaDlg dialog

cImageGammaDlg::cImageGammaDlg()
 : m_bApplyGamma(FALSE)
 , m_gamma(0)
{

}

cImageGammaDlg::~cImageGammaDlg()
{
}


// cImageGammaDlg message handlers

LRESULT cImageGammaDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   CenterWindow(GetParent());
   DoDataExchange(FALSE);
   return TRUE;
}

LRESULT cImageGammaDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   if (DoDataExchange(TRUE))
   {
      EndDialog(wID);
   }
   return 0;
}

LRESULT cImageGammaDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   EndDialog(wID);
   return 0;
}
