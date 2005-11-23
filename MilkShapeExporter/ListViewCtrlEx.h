/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_LISTVIEWCTRLEX_H
#define INCLUDED_LISTVIEWCTRLEX_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cListViewSubItemEditImpl
//

//class cListViewSubItemEditTraits : public CWinTraitsOR<ES_AUTOHSCROLL | WS_BORDER>
//{
//};
typedef CControlWinTraits cListViewSubItemEditTraits;

template <class T, class TBase = CEdit, class TWinTraits = cListViewSubItemEditTraits>
class ATL_NO_VTABLE cListViewSubItemEditImpl : public ATL::CWindowImpl<T, TBase, TWinTraits>
{
   typedef cListViewSubItemEditImpl<T, TBase, TWinTraits> ThisClass;

public:
   DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

   cListViewSubItemEditImpl()
   {
   }

   ~cListViewSubItemEditImpl()
   {
   }

   BEGIN_MSG_MAP_EX(ThisClass)
      DEFAULT_REFLECTION_HANDLER()
   END_MSG_MAP()
};

class cListViewSubItemEdit : public cListViewSubItemEditImpl<cListViewSubItemEdit>
{
public:
   DECLARE_WND_SUPERCLASS(_T("ListViewSubItemEdit"), GetWndClassName())
};

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cListViewCtrlExImplTraits
//

template <DWORD t_dwStyle, DWORD t_dwExStyle, DWORD t_dwExListViewStyle>
class cListViewCtrlExImplTraits
{
public:
   static DWORD GetWndStyle(DWORD dwStyle)
   {
      return (dwStyle == 0) ? t_dwStyle : dwStyle;
   }

   static DWORD GetWndExStyle(DWORD dwExStyle)
   {
      return (dwExStyle == 0) ? t_dwExStyle : dwExStyle;
   }

   static DWORD GetExtendedLVStyle()
   {
      return t_dwExListViewStyle;
   }
};

typedef cListViewCtrlExImplTraits<WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE, LVS_EX_FULLROWSELECT> cListViewCtrlExTraits;


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cListViewCtrlExImpl
//

template <class T, class TBase = CListViewCtrl, class TWinTraits = cListViewCtrlExTraits>
class ATL_NO_VTABLE cListViewCtrlExImpl : public ATL::CWindowImpl<T, TBase, TWinTraits>
{
   typedef cListViewCtrlExImpl<T, TBase, TWinTraits> ThisClass;

public:
   DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

// Attributes
   static DWORD GetExtendedLVStyle()
   {
      return TWinTraits::GetExtendedLVStyle();
   }

// Operations
   BOOL SubclassWindow(HWND hWnd)
   {
#if (_MSC_VER >= 1300)
      BOOL bRet = ATL::CWindowImplBaseT< TBase, TWinTraits>::SubclassWindow(hWnd);
#else //!(_MSC_VER >= 1300)
      typedef ATL::CWindowImplBaseT< TBase, TWinTraits> _baseClass;
      BOOL bRet = _baseClass::SubclassWindow(hWnd);
#endif //!(_MSC_VER >= 1300)
      if(bRet)
      {
         T* pT = static_cast<T*>(this);
         SetExtendedListViewStyle(pT->GetExtendedLVStyle());
      }
      return bRet;
   }

// Implementation
   BEGIN_MSG_MAP_EX(ThisClass)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      REFLECTED_NOTIFY_CODE_HANDLER_EX(LVN_BEGINLABELEDIT, OnBeginLabelEdit)
      REFLECTED_NOTIFY_CODE_HANDLER_EX(LVN_ENDLABELEDIT, OnEndLabelEdit)
      DEFAULT_REFLECTION_HANDLER()
   END_MSG_MAP()

   LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
   {
      // First let the ListView control initialize everything
      LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);
      if (lRet == 0)
      {
         T* pT = static_cast<T*>(this);
         SetExtendedListViewStyle(pT->GetExtendedLVStyle());
      }
      return lRet;
   }

   LRESULT OnBeginLabelEdit(LPNMHDR pnmh)
   {
      LV_DISPINFO *pLVDispInfo = (LV_DISPINFO*)pnmh;

      DWORD dwMsgPos = GetMessagePos();
      POINT ptMsg = { GET_X_LPARAM(dwMsgPos), GET_Y_LPARAM(dwMsgPos) };
      ScreenToClient(&ptMsg);

      CRect rect;
      GetItemRect(pLVDispInfo->item.iItem, &rect, LVIR_BOUNDS);

      int iSubItem = -1;

      for (int i = 0; true; i++)
      {
         LVCOLUMN col = { 0 };
         col.mask = LVCF_WIDTH;
         if (!GetColumn(i, &col))
         {
            break;
         }

         rect.right = rect.left + col.cx;

         if ((ptMsg.x >= rect.left) && (ptMsg.x <= rect.right))
         {
            iSubItem = i;
            break;
         }

         rect.left += col.cx;
      }

      if (iSubItem < 0)
      {
         // Sub-item unknown so prevent editing
         return 1;
      }

      CString text;
      GetItemText(pLVDispInfo->item.iItem, iSubItem, text);

      HWND hWndEdit = (HWND)SendMessage(LVM_GETEDITCONTROL);
      ::SetWindowText(hWndEdit, text);
//      m_subItemEdit.SubclassWindow(hWndEdit);
//      m_subItemEdit.SetWindowText(text);

      return 0;
   }

   LRESULT OnEndLabelEdit(LPNMHDR pnmh)
   {
      LV_DISPINFO *pLVDispInfo = (LV_DISPINFO*)pnmh;

      if (m_subItemEdit.m_hWnd != NULL)
      {
         m_subItemEdit.UnsubclassWindow();
      }

      return 0;
   }

private:
   cListViewSubItemEdit m_subItemEdit;
};

class cListViewCtrlEx : public cListViewCtrlExImpl<cListViewCtrlEx>
{
public:
   DECLARE_WND_SUPERCLASS(_T("ListViewCtrlEx"), GetWndClassName())
};

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_LISTVIEWCTRLEX_H
