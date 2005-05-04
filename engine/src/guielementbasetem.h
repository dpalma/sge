///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIELEMENTBASETEM_H
#define INCLUDED_GUIELEMENTBASETEM_H

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
 : m_id(""),
   m_bFocus(false),
   m_bMouseOver(false),
   m_bVisible(true),
   m_bEnabled(true),
   m_pParent(NULL),
   m_position(0,0),
   m_size(0,0)
{
}

///////////////////////////////////////

template <typename INTRFC>
const char * cGUIElementBase<INTRFC>::GetId() const
{
   return m_id.c_str();
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIElementBase<INTRFC>::SetId(const char * pszId)
{
   m_id = pszId;
}

///////////////////////////////////////

template <typename INTRFC>
bool cGUIElementBase<INTRFC>::HasFocus() const
{
   return m_bFocus;
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIElementBase<INTRFC>::SetFocus(bool bFocus)
{
   m_bFocus = bFocus;
}

///////////////////////////////////////

template <typename INTRFC>
bool cGUIElementBase<INTRFC>::IsMouseOver() const
{
   return m_bMouseOver;
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIElementBase<INTRFC>::SetMouseOver(bool bMouseOver)
{
   m_bMouseOver = bMouseOver;
}

///////////////////////////////////////

template <typename INTRFC>
bool cGUIElementBase<INTRFC>::IsVisible() const
{
   return m_bVisible;
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIElementBase<INTRFC>::SetVisible(bool bVisible)
{
   m_bVisible = bVisible;
}

///////////////////////////////////////

template <typename INTRFC>
bool cGUIElementBase<INTRFC>::IsEnabled() const
{
   return m_bEnabled;
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIElementBase<INTRFC>::SetEnabled(bool bEnabled)
{
   m_bEnabled = bEnabled;
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
