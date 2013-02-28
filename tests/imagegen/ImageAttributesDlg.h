/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_IMAGEATTRIBUTESDLG_H)
#define INCLUDED_IMAGEATTRIBUTESDLG_H

#include <atlctrls.h>
#include <atlddx.h>

#include "resource.h"       // main symbols

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cImageAttributesDlg
//

class cImageAttributesDlg : public CDialogImpl<cImageAttributesDlg>
                          , public WTL::CWinDataExchange<cImageAttributesDlg>
{
public:
   enum { IDD = IDD_IMAGE_ATTRIBUTES };

   cImageAttributesDlg();

   BEGIN_MSG_MAP(cImageGammaDlg)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_ID_HANDLER(IDOK, OnOK)
      COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
   END_MSG_MAP()

   LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
   LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
   LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

   template <typename Type>
   BOOL DDX_ComboBoxIndexByData(UINT nID, Type & data, BOOL bSave, BOOL bValidate = FALSE)
   {
      BOOL bSuccess = TRUE;

      if(bSave)
      {
         int iSel = SendDlgItemMessage(nID, CB_GETCURSEL, 0, 0);
         if (iSel != CB_ERR)
         {
            data = SendDlgItemMessage(nID, CB_GETITEMDATA, iSel, 0);
         }
         else
         {
            bSuccess = FALSE;
         }
      }
      else
      {
         int nItems = SendDlgItemMessage(nID, CB_GETCOUNT, 0, 0);
         for (int i = 0; i < nItems; ++i)
         {
            DWORD_PTR itemData = SendDlgItemMessage(nID, CB_GETITEMDATA, i, 0);
            if (itemData == data)
            {
               SendDlgItemMessage(nID, CB_SETCURSEL, i, 0);
               break;
            }
         }
      }

      if(!bSuccess)
      {
         OnDataExchangeError(nID, bSave);
      }
      else if(bSave && bValidate)   // validation
      {
         // No validation
      }
      return bSuccess;
   }

   BEGIN_DDX_MAP(cImageAttributesDlg)
      DDX_CONTROL_HANDLE(IDC_PIXEL_FORMAT, m_pixelFormatComboBox)
      if (nCtlID == (UINT)-1 || nCtlID == IDC_PIXEL_FORMAT)
      {
         if (!DDX_ComboBoxIndexByData(IDC_PIXEL_FORMAT, m_pixelFormat, bSaveAndValidate))
            return FALSE;
      }
      DDX_INT_RANGE(IDC_WIDTH, m_width, 1, 1024)
      DDX_INT_RANGE(IDC_HEIGHT, m_height, 1, 1024)
   END_DDX_MAP()

   WTL::CComboBox m_pixelFormatComboBox;
   int m_pixelFormat;
   int m_width;
   int m_height;
};

#endif // !defined(INCLUDED_IMAGEATTRIBUTESDLG_H)
