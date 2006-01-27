///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUILISTBOX_H
#define INCLUDED_GUILISTBOX_H

#include "guielementbase.h"

#include "scriptapi.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIListBoxElement
//

class cGUIListBoxElement : public cComObject2<cGUIElementBase<IGUIListBoxElement>,
                                              &IID_IGUIListBoxElement,
                                              IMPLEMENTS(IScriptable)>
{
public:
   cGUIListBoxElement();
   ~cGUIListBoxElement();

   // Over-rides
   virtual tResult OnEvent(IGUIEvent * pEvent);
   virtual tResult EnumChildren(IGUIElementEnum * * ppChildren);
   virtual tResult SetClientArea(const tGUIRect & clientArea);

   // IGUIListBoxElement methods
   virtual tResult AddItem(const tGUIChar * pszString, uint_ptr extra);
   virtual tResult RemoveItem(uint index);
   virtual tResult GetItemCount(uint * pItemCount) const;
   virtual tResult GetItem(uint index, tGUIString * pString, uint_ptr * pExtra,
                           bool * pbIsSelected) const;
   virtual tResult Sort();
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
   virtual tResult GetScrollBar(eGUIScrollBarType scrollBarType,
                                IGUIScrollBarElement * * ppScrollBar);
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

   class cListBoxItem
   {
   public:
      cListBoxItem(const tGUIChar * pszText, uint_ptr data, bool bSelected = false)
         : m_text(pszText), m_data(data), m_bSelected(bSelected) {}
      cListBoxItem(const cListBoxItem & other)
         : m_text(other.m_text), m_data(other.m_data), m_bSelected(other.m_bSelected) {}
      const tGUIString & GetText() const { return m_text; }
      uint_ptr GetData() const { return m_data; }
      bool IsSelected() const { return m_bSelected; }
      void Select() { m_bSelected = true; }
      void Deselect() { m_bSelected = false; }
   private:
      tGUIString m_text;
      uint_ptr m_data;
      bool m_bSelected;
   };
   typedef cListBoxItem tListBoxItem;
   typedef std::vector<cListBoxItem> tListBoxItems;
   tListBoxItems m_items;
   uint m_rowCount;
   cAutoIPtr<IGUIScrollBarElement> m_pHScrollBar, m_pVScrollBar;
   uint m_itemHeight;
   tGUIString m_onSelChange;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUILISTBOX_H
