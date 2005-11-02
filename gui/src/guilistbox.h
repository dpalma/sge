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

class cGUIListBoxElement : public cComObject2<cGUIElementBase<IGUIListBoxElement>, &IID_IGUIListBoxElement,
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
   virtual tResult AddItem(const tChar * pszString, uint_ptr extra);
   virtual tResult RemoveItem(uint index);
   virtual tResult GetItemCount(uint * pItemCount);
   virtual tResult GetItem(uint index, cStr * pString, uint_ptr * pExtra);
   virtual tResult Sort();
   virtual tResult Clear();
   virtual tResult FindItem(const tChar * pszString, uint * pIndex);
   virtual tResult Select(uint startIndex, uint endIndex);
   virtual tResult SelectAll();
   virtual tResult Deselect(uint startIndex, uint endIndex);
   virtual tResult DeselectAll();
   virtual tResult GetSelected(uint * pIndices, uint nMaxIndices);
   virtual tResult GetRowCount(uint * pRowCount);
   virtual tResult SetRowCount(uint rowCount);
   virtual tResult GetScrollBar(eGUIScrollBarType scrollBarType, IGUIScrollBarElement * * ppScrollBar);

   // IScriptable methods
   virtual tResult Invoke(const char * pszMethodName,
                          int argc, const cScriptVar * argv,
                          int nMaxResults, cScriptVar * pResults);

private:
   typedef std::pair<cStr, uint_ptr> tListBoxItem;
   typedef std::vector<tListBoxItem> tListBoxItems;
   tListBoxItems m_items;
   uint m_rowCount;
   cAutoIPtr<IGUIScrollBarElement> m_pHScrollBar, m_pVScrollBar;
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIListBoxElementFactory
//

class cGUIListBoxElementFactory : public cComObject<IMPLEMENTS(IGUIElementFactory)>
{
public:
   virtual tResult CreateElement(const TiXmlElement * pXmlElement, IGUIElement * * ppElement);
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUILISTBOX_H
