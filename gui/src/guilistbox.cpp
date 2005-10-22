///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guilistbox.h"
#include "guielementbasetem.h"
#include "guielementtools.h"
#include "guistrings.h"

#include "globalobj.h"

#include <tinyxml.h>

#include "dbgalloc.h" // must be last header


static const uint kInsaneRowCount = 1000;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIListBoxElement
//

////////////////////////////////////////

cGUIListBoxElement::cGUIListBoxElement()
 : m_rowCount(1)
{
}

////////////////////////////////////////

cGUIListBoxElement::~cGUIListBoxElement()
{
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
