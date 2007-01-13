/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_ABOUTDLG_H)
#define INCLUDED_ABOUTDLG_H

#include "resource.h"       // main symbols

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace WTL { class CHyperLink; }

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: CAboutDlg
//

class CAboutDlg : public CDialogImpl<CAboutDlg>
{
public:
	enum { IDD = IDD_ABOUTBOX };

   BEGIN_MSG_MAP(CAboutDlg)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
      COMMAND_ID_HANDLER(IDOK, OnCloseCommand)
      COMMAND_ID_HANDLER(IDCANCEL, OnCloseCommand)
   END_MSG_MAP()

   LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL &);
   LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL &);
   LRESULT OnCloseCommand(WORD, WORD, HWND, BOOL &);

private:
   std::vector<WTL::CHyperLink*> m_hyperLinks;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_ABOUTDLG_H)
