/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_ABOUTDLG_H)
#define INCLUDED_ABOUTDLG_H

#include "resource.h"       // main symbols

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAboutDlg
//

class cAboutDlg : public CDialogImpl<cAboutDlg>
{
public:
   enum { IDD = IDD_ABOUTBOX };

   BEGIN_MSG_MAP(cAboutDlg)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
      COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
   END_MSG_MAP()

   LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
   LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_ABOUTDLG_H)
