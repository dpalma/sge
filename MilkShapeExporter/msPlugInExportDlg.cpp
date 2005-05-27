/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "msPlugInExportDlg.h"

#include <atldlgs.h>

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMsPlugInExportDlg
//

////////////////////////////////////////

cMsPlugInExportDlg::cMsPlugInExportDlg()
 : m_bExportAnimations(FALSE),
   m_exportFileName(_T(""))
{
}

////////////////////////////////////////

cMsPlugInExportDlg::~cMsPlugInExportDlg()
{
}


/////////////////////////////////////////////////////////////////////////////
// cMsPlugInExportDlg message handlers

static bool ChooseExportFileName(CString * pFileName)
{
   CString defaultExt;
   Verify(defaultExt.LoadString(IDS_EXPORT_DEFAULT_EXTENSION));

   CString filter;
   Verify(filter.LoadString(IDS_EXPORT_FILTER));

   CFileDialog dlg(FALSE, defaultExt, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter);
   if (dlg.DoModal() == IDOK)
   {
      *pFileName = dlg.m_szFileName;
      return true;
   }

   return false;
}


LRESULT cMsPlugInExportDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
{
   DlgResize_Init();

   // Update the dialog controls from the member variables
   // after populating drop-lists, etc.
   DoDataExchange(FALSE);

   CenterWindow(GetParent());

   return TRUE;  // return TRUE unless you set the focus to a control
                 // EXCEPTION: OCX Property Pages should return FALSE
}


LRESULT cMsPlugInExportDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
   EndDialog(IDOK);
   return 0;
}


LRESULT cMsPlugInExportDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & bHandled)
{
   return 0;
}

LRESULT cMsPlugInExportDlg::OnBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled)
{
   if (ChooseExportFileName(&m_exportFileName))
   {
      DoDataExchange(FALSE);
   }
   return 0;
}
