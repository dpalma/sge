///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIELEMENTBASE_H
#error ("guielementbasetem.h must be preceded by guielementbase.h")
#endif

#ifndef INCLUDED_GUIELEMENTBASETEM_H
#define INCLUDED_GUIELEMENTBASETEM_H

#include "gui/guiapi.h"
#include "gui/guistyleapi.h"

#include "tech/globalobj.h"

#include "tech/dbgalloc.h" // must be last header

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cGUIElementBase
//

///////////////////////////////////////

template <typename INTRFC>
cGUIElementBase<INTRFC>::cGUIElementBase()
 : m_flags(kFlags_Visible)
 , m_pParent(NULL)
 , m_position(0,0)
 , m_size(0,0)
 , m_pClientArea(NULL)
{
}

///////////////////////////////////////

template <typename INTRFC>
cGUIElementBase<INTRFC>::~cGUIElementBase()
{
   delete m_pClientArea, m_pClientArea = NULL;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIElementBase<INTRFC>::GetId(tGUIString * pId) const
{
   if (pId == NULL)
   {
      return E_POINTER;
   }
   if (m_id.empty())
   {
      return S_FALSE;
   }
   *pId = m_id;
   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIElementBase<INTRFC>::SetId(const tGUIChar * pszId)
{
   if (pszId != NULL)
   {
      m_id = pszId;
   }
   else
   {
      m_id.erase();
   }
}

///////////////////////////////////////

template <typename INTRFC>
bool cGUIElementBase<INTRFC>::IsVisible() const
{
   return ((m_flags & kFlags_Visible) != 0);
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIElementBase<INTRFC>::SetVisible(bool bVisible)
{
   if (bVisible)
   {
      m_flags |= kFlags_Visible;
   }
   else
   {
      m_flags &= ~kFlags_Visible;
   }
}

///////////////////////////////////////

template <typename INTRFC>
bool cGUIElementBase<INTRFC>::IsEnabled() const
{
   return ((m_flags & kFlags_Disabled) == 0);
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIElementBase<INTRFC>::SetEnabled(bool bEnabled)
{
   if (bEnabled)
   {
      m_flags &= ~kFlags_Disabled;
   }
   else
   {
      m_flags |= kFlags_Disabled;
   }
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIElementBase<INTRFC>::GetParent(IGUIElement * * ppParent)
{
   if (ppParent == NULL)
   {
      return E_POINTER;
   }
   if (m_pParent == NULL)
   {
      return S_FALSE;
   }
   *ppParent = CTAddRef(m_pParent);
   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIElementBase<INTRFC>::SetParent(IGUIElement * pParent)
{
   if (pParent != NULL)
   {
      cAutoIPtr<IGUIContainerElement> pContainer;
      if (pParent->QueryInterface(IID_IGUIContainerElement, (void**)&pContainer) == S_OK)
      {
         pContainer->AddElement(this);
      }
   }
   // Elements don't AddRef() the parent pointer because if the parent is
   // destroyed the element is likely to be destroyed as well. Plus, container
   // elements hold references to their children so to AddRef() the parent 
   // too would cause a circular reference.
   m_pParent = pParent;
   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
tGUIPoint cGUIElementBase<INTRFC>::GetPosition() const
{
   return m_position;
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIElementBase<INTRFC>::SetPosition(const tGUIPoint & point)
{
   m_position = point;
}

///////////////////////////////////////

template <typename INTRFC>
tGUISize cGUIElementBase<INTRFC>::GetSize() const
{
   return m_size;
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIElementBase<INTRFC>::SetSize(const tGUISize & size)
{
   m_size = size;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIElementBase<INTRFC>::OnEvent(IGUIEvent * pEvent)
{
   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIElementBase<INTRFC>::GetRenderer(IGUIElementRenderer * * ppRenderer)
{
   return m_pRenderer.GetPointer(ppRenderer);
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIElementBase<INTRFC>::SetRenderer(IGUIElementRenderer * pRenderer)
{
   SafeRelease(m_pRenderer);
   m_pRenderer = CTAddRef(pRenderer);
   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIElementBase<INTRFC>::GetStyle(IGUIStyle * * ppStyle)
{
   return m_pStyle.GetPointer(ppStyle);
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIElementBase<INTRFC>::SetStyle(IGUIStyle * pStyle)
{
   SafeRelease(m_pStyle);
   m_pStyle = CTAddRef(pStyle);
   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIElementBase<INTRFC>::GetClientArea(tGUIRect * pClientArea)
{
   if (pClientArea == NULL)
   {
      return E_POINTER;
   }
   if (m_pClientArea != NULL)
   {
      *pClientArea = *m_pClientArea;
      return S_OK;
   }
   else
   {
      *pClientArea = tGUIRect(0,0,FloatToInt(m_size.width),FloatToInt(m_size.height));
      return S_FALSE;
   }
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIElementBase<INTRFC>::SetClientArea(const tGUIRect & clientArea)
{
   if (m_pClientArea == NULL)
   {
      m_pClientArea = new tGUIRect(clientArea);
      if (m_pClientArea == NULL)
      {
         return E_OUTOFMEMORY;
      }
   }
   *m_pClientArea = clientArea;
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

#include "tech/undbgalloc.h"

#endif // !INCLUDED_GUIELEMENTBASETEM_H
