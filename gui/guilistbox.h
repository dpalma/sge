///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUILISTBOX_H
#define INCLUDED_GUILISTBOX_H

#include "guielementbase.h"

#include "script/scriptapi.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIListBoxItem
//

class cGUIListBoxItem
{
public:
   cGUIListBoxItem(const tGUIChar * pszText, uint_ptr data, bool bSelected = false);
   cGUIListBoxItem(const cGUIListBoxItem & other);

   const cGUIListBoxItem & operator =(const cGUIListBoxItem & other);

   const tGUIString & GetText() const;
   uint_ptr GetData() const;
   bool IsSelected() const;
   void Select();
   void Deselect();

private:
   tGUIString m_text;
   uint_ptr m_data;
   bool m_bSelected;
};

inline const tGUIString & cGUIListBoxItem::GetText() const { return m_text; }
inline uint_ptr cGUIListBoxItem::GetData() const { return m_data; }
inline bool cGUIListBoxItem::IsSelected() const { return m_bSelected; }
inline void cGUIListBoxItem::Select() { m_bSelected = true; }
inline void cGUIListBoxItem::Deselect() { m_bSelected = false; }


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cGUIScrollable
//

template <typename T, typename INTRFC = IGUIScrollable>
class cGUIScrollable : public INTRFC
{
public:
   tResult ComputeClientArea(IGUIElementRenderer * pRenderer, tGUIRect * pClientArea)
   {
      if (pRenderer == NULL || pClientArea == NULL)
      {
         return E_POINTER;
      }

      T * pT = static_cast<T*>(this);

      if (!!m_pVScrollBar && m_pVScrollBar->IsVisible())
      {
         tGUISize scrollBarSize(0,0);
         if (pRenderer->GetPreferredSize(m_pVScrollBar, pT->GetSize(), &scrollBarSize) == S_OK)
         {
            scrollBarSize.height = static_cast<tGUISizeType>(pClientArea->GetHeight());
            m_pVScrollBar->SetPosition(tGUIPoint(pClientArea->right - scrollBarSize.width, static_cast<float>(pClientArea->top)));
            m_pVScrollBar->SetSize(scrollBarSize);
            pClientArea->right -= FloatToInt(scrollBarSize.width);
            // Returning S_FALSE will signal the page layout code not to size or position the scrollbar
            return S_FALSE;
         }
      }

      return E_FAIL;
   }

   tResult GetVerticalScrollBar(IGUIScrollBarElement * * ppScrollBar)
   {
      return m_pVScrollBar.GetPointer(ppScrollBar);
   }

protected:
   cAutoIPtr<IGUIScrollBarElement> m_pVScrollBar;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIListBoxElement
//

class cGUIListBoxElement : public cComObject3<cGUIElementBase<IGUIListBoxElement>, &IID_IGUIListBoxElement,
                                              cGUIScrollable<cGUIListBoxElement>, &IID_IGUIScrollable,
                                              IMPLEMENTS(IScriptable)>
{
public:
   cGUIListBoxElement();
   ~cGUIListBoxElement();

   // Over-rides
   virtual tResult OnEvent(IGUIEvent * pEvent);

   // IGUIListBoxElement methods
   virtual tResult AddItem(const tGUIChar * pszString, uint_ptr extra);
   virtual tResult RemoveItem(uint index);
   virtual tResult GetItemCount(uint * pItemCount) const;
   virtual tResult GetItem(uint index, tGUIString * pString, uint_ptr * pExtra,
                           bool * pbIsSelected) const;
   virtual const tGUIChar * GetItemText(uint index) const;
   virtual bool IsItemSelected(uint index) const;
   virtual tResult Clear();
   virtual tResult FindItem(const tGUIChar * pszString, uint * pIndex) const;
   virtual tResult Select(uint startIndex, uint endIndex);
   virtual tResult SelectAll();
   virtual tResult Deselect(uint startIndex, uint endIndex);
   virtual tResult DeselectAll();
   virtual tResult GetSelectedCount(uint * pSelectedCount) const;
   virtual tResult GetSelected(uint * pIndices, uint nMaxIndices);
   virtual tResult GetRowCount(uint * pRowCount) const;
   virtual tResult SetRowCount(uint rowCount);
   virtual tResult GetItemHeight(uint * pItemHeight) const;
   virtual tResult SetItemHeight(uint itemHeight);
   virtual tResult GetOnSelChange(tGUIString * pOnSelChange) const;
   virtual tResult SetOnSelChange(const tGUIChar * pszOnSelChange);

   // IScriptable methods
   virtual tResult Invoke(const char * pszMethodName,
                          int argc, const tScriptVar * argv,
                          int nMaxResults, tScriptVar * pResults);

private:
   bool IsMultiSelect() const { return false; }
   void UpdateScrollInfo();

   typedef std::vector<cGUIListBoxItem> tListBoxItems;
   tListBoxItems m_items;
   uint m_rowCount;
   uint m_itemHeight;
   tGUIString m_onSelChange;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUILISTBOX_H
