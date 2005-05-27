/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MSPLUGINEXPORTDLG_H
#define INCLUDED_MSPLUGINEXPORTDLG_H

#include "resource.h"

#include <atlddx.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMsPlugInExportDlg
//

class cMsPlugInExportDlg : public CDialogImpl<cMsPlugInExportDlg>,
                           public CDialogResize<cMsPlugInExportDlg>,
                           public CWinDataExchange<cMsPlugInExportDlg>
{
public:
   cMsPlugInExportDlg();
   ~cMsPlugInExportDlg();

   enum { IDD = IDD_EXPORT };

private:
   BEGIN_MSG_MAP(cMsPlugInExportDlg)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_ID_HANDLER(IDOK, OnOK)
      COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
      COMMAND_ID_HANDLER(IDC_BROWSE, OnBrowse)
      CHAIN_MSG_MAP(CDialogResize<cMsPlugInExportDlg>)
   END_MSG_MAP()

   BEGIN_DLGRESIZE_MAP(cMsPlugInExportDlg)
      DLGRESIZE_CONTROL(IDC_BROWSE, DLSZ_MOVE_X | DLSZ_MOVE_Y)
      DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
      DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
   END_DLGRESIZE_MAP()

   BEGIN_DDX_MAP(cMsPlugInExportDlg)
      DDX_TEXT(IDC_FILENAME, m_exportFileName);
      DDX_TEXT_LEN(IDC_FILENAME, m_exportFileName, MAX_PATH);
   END_DDX_MAP()

   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
   LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled);

private:
   CString m_exportFileName;
   BOOL m_bExportAnimations;
   CListViewCtrl m_animations;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MSPLUGINEXPORTDLG_H
