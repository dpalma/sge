///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUICONTAINERBASE_H
#error ("guicontainerbasetem.h must be preceded by guicontainerbase.h")
#endif

#ifndef INCLUDED_GUICONTAINERBASETEM_H
#define INCLUDED_GUICONTAINERBASETEM_H

#include "guielementbasetem.h"
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
cGUIContainerBase<INTRFC>::cGUIContainerBase(IGUILayoutManager * pLayout)
 : m_pLayout(CTAddRef(pLayout))
{
}

///////////////////////////////////////

template <typename INTRFC>
cGUIContainerBase<INTRFC>::~cGUIContainerBase()
{
   // Since the container has been destroyed, set the parent of all children
   // to NULL as well as release references
   tGUIElementList::iterator iter;
   for (iter = m_children.begin(); iter != m_children.end(); iter++)
   {
      (*iter)->SetParent(NULL);
      (*iter)->Release();
   }
   m_children.clear();
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIContainerBase<INTRFC>::EnumChildren(IGUIElementEnum * * ppElements)
{
   return GUIElementEnumCreate(m_children, ppElements);
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

   tGUIElementList::iterator f = std::find_if(m_children.begin(), m_children.end(), cSameAs(pElement));
   if (f != m_children.end())
   {
      m_children.erase(f);
      pElement->SetParent(NULL);
      pElement->Release();
      return S_OK;
   }

   return S_FALSE;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIContainerBase<INTRFC>::HasElement(IGUIElement * pElement) const
{
   if (pElement == NULL)
   {
      return E_POINTER;
   }

   tGUIElementList::const_iterator f = std::find_if(m_children.begin(), m_children.end(), cSameAs(pElement));
   if (f != m_children.end())
   {
      return S_OK;
   }

   return S_FALSE;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIContainerBase<INTRFC>::GetElement(const tChar * pszId, IGUIElement * * ppElement) const
{
   if (pszId == NULL || ppElement == NULL)
   {
      return E_POINTER;
   }

   // TODO: construct a map to do this
   tGUIElementList::const_iterator iter;
   for (iter = m_children.begin(); iter != m_children.end(); iter++)
   {
      if (GUIElementIdMatch(*iter, pszId))
      {
         *ppElement = CTAddRef(*iter);
         return S_OK;
      }
   }

   return S_FALSE;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIContainerBase<INTRFC>::GetLayout(IGUILayoutManager * * ppLayout)
{
   return m_pLayout.GetPointer(ppLayout);
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIContainerBase<INTRFC>::SetLayout(IGUILayoutManager * pLayout)
{
   SafeRelease(m_pLayout);
   m_pLayout = CTAddRef(pLayout);
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

#include "undbgalloc.h"

#endif // !INCLUDED_GUICONTAINERBASETEM_H
