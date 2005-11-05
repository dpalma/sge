///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guilistbox.h"
#include "guielementbasetem.h"
#include "guielementtools.h"
#include "guistrings.h"

#include "scriptvar.h"

#include "globalobj.h"

#include <algorithm>
#include <list>
#include <tinyxml.h>

#include "dbgalloc.h" // must be last header


static const uint kInsaneRowCount = 1000;

extern tResult GUIScrollBarElementCreate(eGUIScrollBarType scrollBarType, IGUIScrollBarElement * * ppScrollBar);

typedef std::list<IGUIElement *> tGUIElementList;
extern tResult GUIElementEnumCreate(const tGUIElementList & elements, IGUIElementEnum * * ppEnum);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIListBoxElement
//

////////////////////////////////////////

cGUIListBoxElement::cGUIListBoxElement()
 : m_rowCount(1)
 , m_itemHeight(0)
{
   GUIScrollBarElementCreate(kGUIScrollBarHorizontal, &m_pHScrollBar);
   if (!!m_pHScrollBar)
   {
      m_pHScrollBar->SetParent(this);
   }
   GUIScrollBarElementCreate(kGUIScrollBarVertical, &m_pVScrollBar);
   if (!!m_pVScrollBar)
   {
      m_pVScrollBar->SetParent(this);
   }
}

////////////////////////////////////////

cGUIListBoxElement::~cGUIListBoxElement()
{
}

///////////////////////////////////////

tResult cGUIListBoxElement::SetClientArea(const tGUIRect & clientArea)
{
   tGUIRect modifiedClientArea(clientArea);

   cAutoIPtr<IGUIElementRenderer> pRenderer;
   if (GetRenderer(&pRenderer) == S_OK)
   {
      tGUISize hScrollBarSize(0,0), vScrollBarSize(0,0);

      if (!!m_pHScrollBar)
      {
         pRenderer->GetPreferredSize(m_pHScrollBar, &hScrollBarSize);
      }

      if (!!m_pVScrollBar)
      {
         pRenderer->GetPreferredSize(m_pVScrollBar, &vScrollBarSize);
      }

      if (!!m_pHScrollBar)
      {
         hScrollBarSize.width = static_cast<tGUISizeType>(clientArea.GetWidth());
         if (!!m_pVScrollBar)
         {
            hScrollBarSize.width -= vScrollBarSize.width;
         }
         m_pHScrollBar->SetPosition(tGUIPoint(static_cast<float>(clientArea.left), clientArea.bottom - hScrollBarSize.height));
         m_pHScrollBar->SetSize(hScrollBarSize);
         modifiedClientArea.bottom -= Round(hScrollBarSize.height);
      }

      if (!!m_pVScrollBar)
      {
         vScrollBarSize.height = static_cast<tGUISizeType>(clientArea.GetHeight());
         if (!!m_pHScrollBar)
         {
            vScrollBarSize.height -= hScrollBarSize.height;
         }
         m_pVScrollBar->SetPosition(tGUIPoint(clientArea.right - vScrollBarSize.width, static_cast<float>(clientArea.top)));
         m_pVScrollBar->SetSize(vScrollBarSize);
         modifiedClientArea.right -= Round(vScrollBarSize.width);
      }
   }

   return cGUIElementBase<IGUIListBoxElement>::SetClientArea(modifiedClientArea);
}

///////////////////////////////////////

tResult cGUIListBoxElement::OnEvent(IGUIEvent * pEvent)
{
   Assert(pEvent != NULL);

   tResult result = S_OK; // allow event processing to continue

   tGUIEventCode eventCode;
   Verify(pEvent->GetEventCode(&eventCode) == S_OK);

   tGUIPoint point;
   Verify(pEvent->GetMousePosition(&point) == S_OK);

   tGUIPoint pos = GetAbsolutePosition();

   if (eventCode == kGUIEventClick)
   {
      if (m_itemHeight > 0)
      {
         uint index = Round(point.y - pos.y) / m_itemHeight;
         Select(index, index);
         Verify(pEvent->SetCancelBubble(true) == S_OK);
      }
   }

   return result;
}

////////////////////////////////////////

tResult cGUIListBoxElement::EnumChildren(IGUIElementEnum * * ppChildren)
{
   tGUIElementList children;
   children.push_back(m_pHScrollBar);
   children.push_back(m_pVScrollBar);
   return GUIElementEnumCreate(children, ppChildren);
}

////////////////////////////////////////

tResult cGUIListBoxElement::AddItem(const tChar * pszString, uint_ptr extra)
{
   if (pszString == NULL)
   {
      return E_POINTER;
   }
   m_items.push_back(tListBoxItem(pszString, extra));
   return S_OK;
}

////////////////////////////////////////

tResult cGUIListBoxElement::RemoveItem(uint index)
{
   if (index >= m_items.size())
   {
      return E_INVALIDARG;
   }
   m_items.erase(m_items.begin() + index);
   return S_OK;
}

////////////////////////////////////////

tResult cGUIListBoxElement::GetItemCount(uint * pItemCount) const
{
   if (pItemCount == NULL)
   {
      return E_POINTER;
   }
   *pItemCount = m_items.size();
   return *pItemCount > 0 ? S_OK : S_FALSE;
}

////////////////////////////////////////

tResult cGUIListBoxElement::GetItem(uint index, cStr * pString,
                                    uint_ptr * pExtra, bool * pbIsSelected) const
{
   if (index >= m_items.size())
   {
      return E_INVALIDARG;
   }

   if (pString == NULL)
   {
      return E_POINTER;
   }

   *pString = m_items[index].GetText();
   if (pExtra != NULL)
   {
      *pExtra = m_items[index].GetData();
   }
   if (pbIsSelected != NULL)
   {
      *pbIsSelected = m_items[index].IsSelected();
   }
   return S_OK;
}

////////////////////////////////////////

tResult cGUIListBoxElement::Sort()
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cGUIListBoxElement::Clear()
{
   m_items.clear();
   return S_OK;
}

////////////////////////////////////////

tResult cGUIListBoxElement::FindItem(const tChar * pszString, uint * pIndex) const
{
   if (pszString == NULL || pIndex == NULL)
   {
      return E_POINTER;
   }

   tListBoxItems::const_iterator iter = m_items.begin();
   for (uint index = 0; iter != m_items.end(); iter++, index++)
   {
      if (iter->GetText().compare(pszString) == 0)
      {
         *pIndex = index;
         return S_OK;
      }
   }

   return S_FALSE;
}

////////////////////////////////////////

tResult cGUIListBoxElement::Select(uint startIndex, uint endIndex)
{
   if (startIndex >= m_items.size() || startIndex > endIndex)
   {
      return E_INVALIDARG;
   }

   if (!IsMultiSelect())
   {
      if (startIndex != endIndex)
      {
         WarnMsg2("Invalid selection indices for single-select listbox: %d, %d\n", startIndex, endIndex);
         return E_INVALIDARG;
      }

      tListBoxItems::iterator iter = m_items.begin();
      for (uint index = 0; iter != m_items.end(); iter++, index++)
      {
         if (index == startIndex)
         {
            iter->Select();
         }
         else
         {
            iter->Deselect();
         }
      }
   }
   else
   {
      if (endIndex >= m_items.size())
      {
         endIndex = m_items.size() - 1;
      }

      for (uint i = startIndex; i <= endIndex; i++)
      {
         m_items[i].Select();
      }
   }

   return S_OK;
}

////////////////////////////////////////

tResult cGUIListBoxElement::SelectAll()
{
   if (!IsMultiSelect())
   {
      return E_FAIL;
   }

#if _MSC_VER <= 1200
   tListBoxItems::iterator iter = m_items.begin();
   for (; iter != m_items.end(); iter++)
   {
      iter->Select();
   }
#else
   std::for_each(m_items.begin(), m_items.end(), std::mem_fun_ref(&cListBoxItem::Select));
#endif
   return S_OK;
}

////////////////////////////////////////

tResult cGUIListBoxElement::Deselect(uint startIndex, uint endIndex)
{
   if (startIndex >= m_items.size())
   {
      return E_INVALIDARG;
   }

   if (endIndex >= m_items.size())
   {
      endIndex = m_items.size() - 1;
   }

   for (uint i = startIndex; i <= endIndex; i++)
   {
      m_items[i].Deselect();
   }

   return S_OK;
}

////////////////////////////////////////

tResult cGUIListBoxElement::DeselectAll()
{
#if _MSC_VER <= 1200
   tListBoxItems::iterator iter = m_items.begin();
   for (; iter != m_items.end(); iter++)
   {
      iter->Deselect();
   }
#else
   std::for_each(m_items.begin(), m_items.end(), std::mem_fun_ref(&cListBoxItem::Deselect));
#endif
   return S_OK;
}

////////////////////////////////////////

tResult cGUIListBoxElement::GetSelectedCount(uint * pSelectedCount) const
{
   if (pSelectedCount == NULL)
   {
      return E_POINTER;
   }
#if _MSC_VER <= 1200
   *pSelectedCount = 0;
   tListBoxItems::const_iterator iter = m_items.begin();
   for (; iter != m_items.end(); iter++)
   {
      if (iter->IsSelected())
      {
         *pSelectedCount += 1;
      }
   }
#else
   *pSelectedCount = std::count_if(m_items.begin(), m_items.end(), std::mem_fun_ref(&cListBoxItem::IsSelected));
#endif
   if (!IsMultiSelect())
   {
      Assert(*pSelectedCount == 1);
   }
   return S_OK;
}

////////////////////////////////////////

tResult cGUIListBoxElement::GetSelected(uint * pIndices, uint nMaxIndices)
{
   if (pIndices == NULL)
   {
      return E_POINTER;
   }
   if (nMaxIndices == 0)
   {
      return E_INVALIDARG;
   }
   uint nSelected = 0;
   tListBoxItems::const_iterator iter = m_items.begin();
   for (uint index = 0; iter != m_items.end(); iter++, index++)
   {
      if (iter->IsSelected())
      {
         if (nSelected < nMaxIndices)
         {
            pIndices[nSelected++] = index;
         }
         else
         {
            break;
         }
      }
   }
   if (!IsMultiSelect())
   {
      Assert(nSelected == 1);
   }
   return S_OK;
}

////////////////////////////////////////

tResult cGUIListBoxElement::GetRowCount(uint * pRowCount) const
{
   if (pRowCount == NULL)
   {
      return E_POINTER;
   }
   *pRowCount = m_rowCount;
   return *pRowCount > 0 ? S_OK : S_FALSE;
}

////////////////////////////////////////

tResult cGUIListBoxElement::SetRowCount(uint rowCount)
{
   if (rowCount >= kInsaneRowCount)
   {
      WarnMsg1("Insane row count given to listbox element: %d\n", rowCount);
      return E_INVALIDARG;
   }
   m_rowCount = rowCount;
   return S_OK;
}

////////////////////////////////////////

tResult cGUIListBoxElement::GetScrollBar(eGUIScrollBarType scrollBarType,
                                         IGUIScrollBarElement * * ppScrollBar)
{
   if (ppScrollBar == NULL)
   {
      return E_POINTER;
   }
   if (scrollBarType == kGUIScrollBarHorizontal)
   {
      return m_pHScrollBar.GetPointer(ppScrollBar);
   }
   else if (scrollBarType == kGUIScrollBarVertical)
   {
      return m_pVScrollBar.GetPointer(ppScrollBar);
   }
   else
   {
      return E_INVALIDARG;
   }
   return E_FAIL;
}

////////////////////////////////////////

tResult cGUIListBoxElement::GetItemHeight(uint * pItemHeight) const
{
   if (pItemHeight == NULL)
   {
      return E_POINTER;
   }
   *pItemHeight = m_itemHeight;
   return (m_itemHeight != 0) ? S_OK : S_FALSE;
}

////////////////////////////////////////

tResult cGUIListBoxElement::SetItemHeight(uint itemHeight)
{
   m_itemHeight = itemHeight;
   return S_OK;
}

////////////////////////////////////////

tResult cGUIListBoxElement::Invoke(const char * pszMethodName,
                                   int argc, const cScriptVar * argv,
                                   int nMaxResults, cScriptVar * pResults)
{
   if (pszMethodName == NULL)
   {
      return E_POINTER;
   }

   if (strcmp(pszMethodName, "AddItem") == 0)
   {
      if (argc == 1 && argv[0].IsString())
      {
         if (AddItem(argv[0], 0) == S_OK)
         {
            return S_OK;
         }
      }
      else if (argc == 2 && argv[0].IsString() && argv[1].IsNumber())
      {
         if (AddItem(argv[0], argv[1]) == S_OK)
         {
            return S_OK;
         }
      }
      else
      {
         return E_INVALIDARG;
      }
   }
   else if (strcmp(pszMethodName, "GetSelected") == 0)
   {
      if (argc != 0)
      {
         return E_INVALIDARG;
      }

      uint nSelected = 0;
      if (GetSelectedCount(&nSelected) == S_OK)
      {
         nSelected = Min(nSelected, static_cast<uint>(nMaxResults));
         uint * pIndices = reinterpret_cast<uint *>(alloca(nSelected * sizeof(uint)));
         if (GetSelected(pIndices, nSelected) == S_OK)
         {
            for (uint i = 0; i < nSelected; i++)
            {
               pResults[i] = pIndices[i];
            }
            return nSelected;
         }
      }
   }
   else if (strcmp(pszMethodName, "GetItem") == 0)
   {
      if (argc != 1 || !argv[0].IsNumber())
      {
         return E_INVALIDARG;
      }

      tGUIString itemText;
      uint_ptr itemData = 0;
      if (GetItem(argv[0], &itemText, &itemData, NULL) == S_OK)
      {
         if (nMaxResults >= 2)
         {
            pResults[0] = itemText.c_str();
            pResults[1] = itemData;
            return 2;
         }
         else if (nMaxResults >= 1)
         {
            pResults[0] = itemText.c_str();
            return 1;
         }
      }
   }

   return E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIListBoxElementFactory
//

AUTOREGISTER_GUIELEMENTFACTORY(listbox, cGUIListBoxElementFactory);

tResult cGUIListBoxElementFactory::CreateElement(const TiXmlElement * pXmlElement,
                                                 IGUIElement * pParent,
                                                 IGUIElement * * ppElement)
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   if (pXmlElement != NULL)
   {
      if (strcmp(pXmlElement->Value(), kElementListBox) == 0)
      {
         cAutoIPtr<IGUIListBoxElement> pListBox = static_cast<IGUIListBoxElement *>(new cGUIListBoxElement);
         if (!pListBox)
         {
            return E_OUTOFMEMORY;
         }

         GUIElementStandardAttributes(pXmlElement, pListBox);

         int rows;
         if (pXmlElement->QueryIntAttribute(kAttribRows, &rows) == TIXML_SUCCESS)
         {
            pListBox->SetRowCount(rows);
         }

         for (const TiXmlElement * pXmlChild = pXmlElement->FirstChildElement(); 
            pXmlChild != NULL; pXmlChild = pXmlChild->NextSiblingElement())
         {
            if (pXmlChild->Type() == TiXmlNode::ELEMENT
               && stricmp(pXmlChild->Value(), "item") == 0)
            {
               const char * pszText = pXmlChild->Attribute(kAttribText);
               if (pszText != NULL)
               {
                  pListBox->AddItem(pszText, 0);
               }
            }
         }

         *ppElement = CTAddRef(pListBox);
         return S_OK;
      }
   }
   else
   {
      *ppElement = static_cast<IGUIListBoxElement *>(new cGUIListBoxElement);
      return (*ppElement != NULL) ? S_OK : E_FAIL;
   }

   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////
