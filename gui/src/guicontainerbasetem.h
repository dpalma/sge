///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUICONTAINERBASETEM_H
#define INCLUDED_GUICONTAINERBASETEM_H

#include "guielementbasetem.h"
#include "guielementtools.h"
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
 , m_bNeedLayout(false)
 , m_pInsets(NULL)
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

   delete m_pInsets;
   m_pInsets = NULL;
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIContainerBase<INTRFC>::SetSize(const tGUISize & size)
{
   cGUIElementBase<INTRFC>::SetSize(size);
   m_bNeedLayout = true;
   DoLayout();
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIContainerBase<INTRFC>::EnumChildren(IGUIElementEnum * * ppElements)
{
   return GUIElementEnumCreate(m_children, ppElements);
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIContainerBase<INTRFC>::SetClientArea(const tGUIRect & clientArea)
{
   tResult result = cGUIElementBase<INTRFC>::SetClientArea(clientArea);
   m_bNeedLayout = (result == S_OK);
   DoLayout();
   return result;
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

   m_bNeedLayout = true;

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
      m_bNeedLayout = true;
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
   m_bNeedLayout = true;
   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIContainerBase<INTRFC>::GetInsets(tGUIInsets * pInsets)
{
   if (pInsets == NULL)
   {
      return E_POINTER;
   }

   if (m_pInsets == NULL)
   {
      return S_FALSE;
   }

   *pInsets = *m_pInsets;

   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIContainerBase<INTRFC>::SetInsets(const tGUIInsets & insets)
{
   if (m_pInsets == NULL)
   {
      m_pInsets = new tGUIInsets;
      if (m_pInsets == NULL)
      {
         return E_OUTOFMEMORY;
      }
   }

   *m_pInsets = insets;
   m_bNeedLayout = true;
   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIContainerBase<INTRFC>::DoLayout()
{
   if (m_bNeedLayout)
   {
      cAutoIPtr<IGUILayoutManager> pLayout;
      if (GetLayout(&pLayout) == S_OK)
      {
         pLayout->Layout(this);
      }
      else
      {
         tGUIRect rect;
         if (GetClientArea(&rect) == S_OK)
         {
            tGUIInsets insets;
            if (GetInsets(&insets) == S_OK)
            {
               rect.left += insets.left;
               rect.top += insets.top;
               rect.right -= insets.right;
               rect.bottom -= insets.bottom;
            }
            ForEachElement(cSizeAndPlaceElement(rect));
         }
      }

      m_bNeedLayout = false;
   }
}

///////////////////////////////////////////////////////////////////////////////

#include "undbgalloc.h"

#endif // !INCLUDED_GUICONTAINERBASETEM_H
