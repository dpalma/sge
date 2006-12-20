/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_IMAGEGAMMADLG_H)
#define INCLUDED_IMAGEGAMMADLG_H

#include <atlddx.h>

#include "resource.h"       // main symbols

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cImageGammaDlg
//

class cImageGammaDlg : public CDialogImpl<cImageGammaDlg>
                     , public WTL::CWinDataExchange<cImageGammaDlg>
{
public:
   enum { IDD = IDD_IMAGE_GAMMA };

   BEGIN_MSG_MAP(cImageGammaDlg)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_ID_HANDLER(IDOK, OnOK)
      COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
   END_MSG_MAP()

   cImageGammaDlg();
   ~cImageGammaDlg();

   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
   LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled);

   BEGIN_DDX_MAP(cImageGammaDlg)
      DDX_CHECK(IDC_APPLY_GAMMA, m_bApplyGamma)
	   DDX_FLOAT_RANGE(IDC_GAMMA_VALUE, m_gamma, .2, 5)
   END_DDX_MAP()

   BOOL m_bApplyGamma;
   float m_gamma;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_IMAGEGAMMADLG_H)
