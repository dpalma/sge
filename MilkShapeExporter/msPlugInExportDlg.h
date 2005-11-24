/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MSPLUGINEXPORTDLG_H
#define INCLUDED_MSPLUGINEXPORTDLG_H

#include "resource.h"
#include "ListViewCtrlEx.h"

#include <atlddx.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct msModel msModel;

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMsPlugInExportDlg
//

class cMsPlugInExportDlg : public CDialogImpl<cMsPlugInExportDlg>,
                           public CDialogResize<cMsPlugInExportDlg>,
                           public CWinDataExchange<cMsPlugInExportDlg>
{
public:
   cMsPlugInExportDlg(msModel * pMsModel);
   ~cMsPlugInExportDlg();

   enum { IDD = IDD_EXPORT };

private:
   BEGIN_MSG_MAP_EX(cMsPlugInExportDlg)
      REFLECT_NOTIFICATIONS()
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_ID_HANDLER(IDOK, OnOK)
      COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
      COMMAND_ID_HANDLER(IDC_BROWSE, OnBrowse)
      COMMAND_ID_HANDLER(IDC_EXPORTANIMATIONS, OnExportAnimations)
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
      DDX_CONTROL(IDC_ANIMATIONS, m_animations);
   END_DDX_MAP()

   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
   LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnExportAnimations(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled);

   // Reflect only WM_NOTIFY messages
   LRESULT ReflectNotifications(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      if (uMsg == WM_NOTIFY)
      {
         HWND hWndChild = ((LPNMHDR)lParam)->hwndFrom;
         return ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
      }
      bHandled = FALSE;
      return 1;
   }

private:
   CString m_exportFileName;
   bool m_bExportAnimations;
   cListViewCtrlEx m_animations;
   msModel * m_pMsModel;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MSPLUGINEXPORTDLG_H
