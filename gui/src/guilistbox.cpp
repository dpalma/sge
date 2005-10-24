///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guilistbox.h"
#include "guielementbasetem.h"
#include "guielementtools.h"
#include "guistrings.h"

#include "globalobj.h"

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

void cGUIListBoxElement::SetPosition(const tGUIPoint & point)
{
   cGUIElementBase<IGUIListBoxElement>::SetPosition(point);

   const tGUISize size = GetSize();

   if (!!m_pHScrollBar)
   {
      const tGUISize scrollBarSize = m_pHScrollBar->GetSize();
      m_pHScrollBar->SetPosition(tGUIPoint(0, size.height - scrollBarSize.height));
   }

   if (!!m_pVScrollBar)
   {
      const tGUISize scrollBarSize = m_pVScrollBar->GetSize();
      m_pVScrollBar->SetPosition(tGUIPoint(size.width - scrollBarSize.width, 0));
   }
}

///////////////////////////////////////

void cGUIListBoxElement::SetSize(const tGUISize & size)
{
   cGUIElementBase<IGUIListBoxElement>::SetSize(size);

   cAutoIPtr<IGUIElementRenderer> pRenderer;
   if (GetRenderer(&pRenderer) == S_OK)
   {
      if (!!m_pHScrollBar)
      {
         tGUISize scrollBarSize = pRenderer->GetPreferredSize(m_pHScrollBar);
         scrollBarSize.width = size.width;
         m_pHScrollBar->SetSize(scrollBarSize);
      }

      if (!!m_pVScrollBar)
      {
         tGUISize scrollBarSize = pRenderer->GetPreferredSize(m_pVScrollBar);
         scrollBarSize.height = size.height;
         m_pVScrollBar->SetSize(scrollBarSize);
      }
   }
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

tResult cGUIListBoxElement::GetItemCount(uint * pItemCount)
{
   if (pItemCount == NULL)
   {
      return E_POINTER;
   }
   *pItemCount = m_items.size();
   return *pItemCount > 0 ? S_OK : S_FALSE;
}

////////////////////////////////////////

tResult cGUIListBoxElement::GetItem(uint index, cStr * pString, uint_ptr * pExtra)
{
   if (index >= m_items.size())
   {
      return E_INVALIDARG;
   }

   if (pString == NULL)
   {
      return E_POINTER;
   }

   *pString = m_items[index].first;
   if (pExtra != NULL)
   {
      *pExtra = m_items[index].second;
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

tResult cGUIListBoxElement::FindItem(const tChar * pszString, uint * pIndex)
{
   if (pszString == NULL || pIndex == NULL)
   {
      return E_POINTER;
   }

   tListBoxItems::iterator iter = m_items.begin();
   for (uint index = 0; iter != m_items.end(); iter++, index++)
   {
      if (iter->first.compare(pszString) == 0)
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
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cGUIListBoxElement::SelectAll()
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cGUIListBoxElement::Deselect(uint startIndex, uint endIndex)
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cGUIListBoxElement::DeselectAll()
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cGUIListBoxElement::GetSelected(uint * pIndices, uint nMaxIndices)
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cGUIListBoxElement::GetRowCount(uint * pRowCount)
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


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIListBoxElementFactory
//

AUTOREGISTER_GUIELEMENTFACTORY(listbox, cGUIListBoxElementFactory);

tResult cGUIListBoxElementFactory::CreateElement(const TiXmlElement * pXmlElement, 
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

         for (TiXmlElement * pXmlChild = pXmlElement->FirstChildElement(); 
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
