/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EXPORTPREVIEWDLG_H)
#define INCLUDED_EXPORTPREVIEWDLG_H

#include "resource.h"       // main symbols

#include <atlctrls.h>
#include <atlddx.h>
#include <atlframe.h>

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class cExporter;

namespace WTL { class CHyperLink; }

/////////////////////////////////////////////////////////////////////////////

class cTreeCtrl : public CWindowImpl<cTreeCtrl, WTL::CTreeViewCtrl>
{
   DECLARE_EMPTY_MSG_MAP()
};

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cExportPreviewDlg
//

class cExportPreviewDlg : public CDialogImpl<cExportPreviewDlg>
                        , public WTL::CWinDataExchange<cExportPreviewDlg>
                        , public WTL::CDialogResize<cExportPreviewDlg>
{
public:
	enum { IDD = IDD_PREVIEW };

   BEGIN_MSG_MAP(cExportPreviewDlg)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
      COMMAND_ID_HANDLER(IDOK, OnCloseCommand)
      COMMAND_ID_HANDLER(IDCANCEL, OnCloseCommand)
      CHAIN_MSG_MAP(WTL::CDialogResize<cExportPreviewDlg>)
   END_MSG_MAP()

   LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL &);
   LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL &);
   LRESULT OnCloseCommand(WORD, WORD, HWND, BOOL &);

   BEGIN_DDX_MAP(cExportPreviewDlg)
      DDX_CONTROL(IDC_MODEL_PREVIEW_TREE, m_modelPreviewTree)
   END_DDX_MAP()

   BEGIN_DLGRESIZE_MAP(cExportPreviewDlg)
      DLGRESIZE_CONTROL(IDC_MODEL_PREVIEW_TREE, DLSZ_SIZE_X | DLSZ_SIZE_Y)
      DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
      DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
   END_DLGRESIZE_MAP()

private:
   std::vector<WTL::CHyperLink*> m_hyperLinks;
   cTreeCtrl m_modelPreviewTree;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EXPORTPREVIEWDLG_H)
