///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUICONTAINERBASETEM_H
#define INCLUDED_GUICONTAINERBASETEM_H

#include "guiapi.h"

#include "guielementenum.h"

#include "dbgalloc.h" // must be last header

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cGUIContainerBase
//

///////////////////////////////////////

template <typename INTRFC>
cGUIContainerBase<INTRFC>::cGUIContainerBase()
{
}

///////////////////////////////////////

template <typename INTRFC>
cGUIContainerBase<INTRFC>::~cGUIContainerBase()
{
   std::for_each(m_children.begin(), m_children.end(), CTInterfaceMethod(&IGUIElement::Release));
   m_children.clear();
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIContainerBase<INTRFC>::AddElement(IGUIElement * pElement)
{
   if (pElement == NULL)
   {
      return E_POINTER;
   }

   if (HasElement(pElement) == S_OK)
   {
      return S_FALSE;
   }

   // Important to add the given element to the child list before setting
   // the parent pointer. Setting the parent will also add the element to the
   // new parent. The HasElement() call above prevents the infinite circle of
   // AddElement() calling SetParent() calling AddElement() etc.
   m_children.push_back(CTAddRef(pElement));

   pElement->SetParent(this);

   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIContainerBase<INTRFC>::RemoveElement(IGUIElement * pElement)
{
   if (pElement == NULL)
   {
      return E_POINTER;
   }

   tGUIElementList::iterator iter;
   for (iter = m_children.begin(); iter != m_children.end(); iter++)
   {
      if (CTIsSameObject(*iter, pElement))
      {
         m_children.erase(iter);
         pElement->Release();
         return S_OK;
      }
   }

   return S_FALSE;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIContainerBase<INTRFC>::GetElements(IGUIElementEnum * * ppElements)
{
   return GUIElementEnumCreate(m_children, ppElements);
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIContainerBase<INTRFC>::HasElement(IGUIElement * pElement) const
{
   if (pElement == NULL)
   {
      return E_POINTER;
   }

   tGUIElementList::const_iterator iter;
   for (iter = m_children.begin(); iter != m_children.end(); iter++)
   {
      if (CTIsSameObject(*iter, pElement))
      {
         return S_OK;
      }
   }

   return S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////

#include "undbgalloc.h"

#endif // !INCLUDED_GUICONTAINERBASETEM_H
