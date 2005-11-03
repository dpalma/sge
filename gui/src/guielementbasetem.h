///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIELEMENTBASETEM_H
#define INCLUDED_GUIELEMENTBASETEM_H

#include "globalobj.h"

#include "dbgalloc.h" // must be last header

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
{
}

///////////////////////////////////////

template <typename INTRFC>
cGUIElementBase<INTRFC>::~cGUIElementBase()
{
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
void cGUIElementBase<INTRFC>::SetId(const char * pszId)
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
bool cGUIElementBase<INTRFC>::HasFocus() const
{
   return ((m_flags & kFlags_Focus) != 0);
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIElementBase<INTRFC>::SetFocus(bool bFocus)
{
   if (bFocus)
   {
      m_flags |= kFlags_Focus;
   }
   else
   {
      m_flags &= ~kFlags_Focus;
   }
}

///////////////////////////////////////

template <typename INTRFC>
bool cGUIElementBase<INTRFC>::IsMouseOver() const
{
   return ((m_flags & kFlags_MouseOver) != 0);
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIElementBase<INTRFC>::SetMouseOver(bool bMouseOver)
{
   if (bMouseOver)
   {
      m_flags |= kFlags_MouseOver;
   }
   else
   {
      m_flags &= ~kFlags_MouseOver;
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
bool cGUIElementBase<INTRFC>::Contains(const tGUIPoint & point) const
{
   tGUISize size = GetSize();
   return tRectf(0,0,size.width,size.height).PtInside(point.x,point.y);
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIElementBase<INTRFC>::OnEvent(IGUIEvent * pEvent)
{
   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIElementBase<INTRFC>::GetRendererClass(tGUIString * pRendererClass)
{
   if (pRendererClass == NULL)
   {
      return E_POINTER;
   }

   if (!m_rendererClass.empty())
   {
      *pRendererClass = m_rendererClass;
      return S_OK;
   }

   cAutoIPtr<IGUIElement> pParent;
   if (GetParent(&pParent) == S_OK
      && pParent->GetRendererClass(pRendererClass) == S_OK)
   {
      return S_OK;
   }

   return S_FALSE;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIElementBase<INTRFC>::SetRendererClass(const tChar * pszRendererClass)
{
   if (pszRendererClass == NULL && !m_rendererClass.empty())
   {
      SafeRelease(m_pRenderer);
      m_rendererClass.erase();
      return S_OK;
   }
   else if (m_rendererClass.compare(pszRendererClass) != 0)
   {
      SafeRelease(m_pRenderer);
      m_rendererClass = pszRendererClass;
      return S_OK;
   }
   return S_FALSE;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIElementBase<INTRFC>::GetRenderer(IGUIElementRenderer * * ppRenderer)
{
   if (!m_pRenderer)
   {
      tGUIString rendererClass;
      if (GetRendererClass(&rendererClass) == S_OK)
      {
         UseGlobal(GUIFactories);
         pGUIFactories->CreateRenderer(rendererClass.c_str(), &m_pRenderer);
      }
   }
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
tResult cGUIElementBase<INTRFC>::EnumChildren(IGUIElementEnum * * ppElements)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIElementBase<INTRFC>::GetClientArea(tGUIRect * pClientArea)
{
   if (pClientArea == NULL)
   {
      return E_POINTER;
   }
   if (m_flags & kFlags_HasClientArea)
   {
      *pClientArea = m_clientArea;
   }
   else
   {
      *pClientArea = tGUIRect(0,0,Round(m_size.width),Round(m_size.height));
   }
   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIElementBase<INTRFC>::SetClientArea(const tGUIRect & clientArea)
{
   m_flags |= kFlags_HasClientArea;
   m_clientArea = clientArea;
   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
tGUIPoint cGUIElementBase<INTRFC>::GetAbsolutePosition() const
{
   tGUIPoint absolutePosition = GetPosition();

   cAutoIPtr<IGUIElement> pParent;
   if (const_cast<cGUIElementBase<INTRFC> *>(this)->GetParent(&pParent) == S_OK)
   {
      while (!!pParent)
      {
         absolutePosition += pParent->GetPosition();

         cAutoIPtr<IGUIElement> pNext;
         if (pParent->GetParent(&pNext) != S_OK)
         {
            SafeRelease(pParent);
         }
         else
         {
            pParent = pNext;
         }
      }
   }

   return absolutePosition;
}

///////////////////////////////////////////////////////////////////////////////

#include "undbgalloc.h"

#endif // !INCLUDED_GUIELEMENTBASETEM_H
