///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIEVENTROUTERTEM_H
#define INCLUDED_GUIEVENTROUTERTEM_H

#include "guiapi.h"

#include "inputapi.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cGUIEventRouter
//

///////////////////////////////////////

template <typename INTRFC>
cGUIEventRouter<INTRFC>::cGUIEventRouter()
{
}

///////////////////////////////////////

template <typename INTRFC>
cGUIEventRouter<INTRFC>::~cGUIEventRouter()
{
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::AddEventListener(IGUIEventListener * pListener)
{
   return Connect(pListener);
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::RemoveEventListener(IGUIEventListener * pListener)
{
   return Disconnect(pListener);
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::GetFocus(IGUIElement * * ppElement)
{
   return m_pFocus.GetPointer(ppElement);
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::SetFocus(IGUIElement * pElement)
{
   SafeRelease(m_pFocus);
   m_pFocus = CTAddRef(pElement);
   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::GetCapture(IGUIElement * * ppElement)
{
   return m_pCapture.GetPointer(ppElement);
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::SetCapture(IGUIElement * pElement)
{
   SafeRelease(m_pCapture);
   m_pCapture = CTAddRef(pElement);
   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::GetMouseOver(IGUIElement * * ppElement)
{
   return m_pMouseOver.GetPointer(ppElement);
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::SetMouseOver(IGUIElement * pElement)
{
   SafeRelease(m_pMouseOver);
   m_pMouseOver = CTAddRef(pElement);
   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::AddElement(IGUIElement * pElement)
{
   if (pElement == NULL)
   {
      return E_POINTER;
   }
   m_elements.push_back(CTAddRef(pElement));
   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIEventRouter<INTRFC>::RemoveAllElements()
{
   SafeRelease(m_pFocus);
   SafeRelease(m_pCapture);
   SafeRelease(m_pMouseOver);
   std::for_each(m_elements.begin(), m_elements.end(), CTInterfaceMethodRef(&IGUIElement::Release));
   m_elements.clear();
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::GetHitElement(const tGUIPoint & point, IGUIElement * * ppElement) const
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   tGUIElementList::const_iterator iter;
   for (iter = m_elements.begin(); iter != m_elements.end(); iter++)
   {
      if ((*iter)->Contains(point))
      {
         *ppElement = CTAddRef(*iter);
         return S_OK;
      }
   }

   return S_FALSE;
}

///////////////////////////////////////

template <typename INTRFC>
bool cGUIEventRouter<INTRFC>::BubbleEvent(IGUIEvent * pEvent)
{
   Assert(pEvent != NULL);

   tSinks::iterator iter;
   for (iter = AccessSinks().begin(); iter != AccessSinks().end(); iter++)
   {
      if ((*iter)->OnEvent(pEvent) != S_OK)
      {
         return true;
      }
   }

   cAutoIPtr<IGUIElement> pDispatchTo;
   if (pEvent->GetSourceElement(&pDispatchTo) == S_OK)
   {
      while (!!pDispatchTo)
      {
         if (pDispatchTo->OnEvent(pEvent))
         {
            return true;
         }

         cAutoIPtr<IGUIElement> pNext;
         if (pDispatchTo->GetParent(&pNext) != S_OK)
         {
            SafeRelease(pDispatchTo);
         }
         else
         {
            pDispatchTo = pNext;
         }
      }
   }

   return false;
}

///////////////////////////////////////

template <typename INTRFC>
bool cGUIEventRouter<INTRFC>::GetEventTarget(const sInputEvent * pInputEvent, 
                                             IGUIElement * * ppElement)
{
   Assert(pInputEvent != NULL);
   Assert(ppElement != NULL);

   if (GetCapture(ppElement) == S_OK)
   {
      return true;
   }
   else if (KeyIsMouse(pInputEvent->key))
   {
      return GetHitElement(pInputEvent->point, ppElement) == S_OK;
   }
   else if (GetFocus(ppElement) == S_OK)
   {
      return true;
   }

   return false;
}

///////////////////////////////////////

template <typename INTRFC>
bool cGUIEventRouter<INTRFC>::HandleInputEvent(const sInputEvent * pInputEvent)
{
   tGUIEventCode eventCode = GUIEventCode(pInputEvent->key, pInputEvent->down);
   if (eventCode == kGUIEventNone)
   {
      DebugMsg("WARNING: Invalid event code\n");
      return false;
   }

   if (eventCode == kGUIEventMouseMove)
   {
      cAutoIPtr<IGUIElement> pMouseOver;
      if ((GetMouseOver(&pMouseOver) == S_OK) && !pMouseOver->Contains(pInputEvent->point))
      {
         SetMouseOver(NULL);
         cAutoIPtr<IGUIEvent> pMouseLeaveEvent;
         if (GUIEventCreate(kGUIEventMouseLeave, pInputEvent->point, pInputEvent->key, 
            pMouseOver, &pMouseLeaveEvent) == S_OK)
         {
            BubbleEvent(pMouseLeaveEvent);
         }
      }

      cAutoIPtr<IGUIElement> pHit;
      if ((GetHitElement(pInputEvent->point, &pHit) == S_OK)
         && (AccessMouseOver() == NULL))
      {
         SetMouseOver(pHit);
         cAutoIPtr<IGUIEvent> pMouseEnterEvent;
         if (GUIEventCreate(kGUIEventMouseEnter, pInputEvent->point, pInputEvent->key, 
            pHit, &pMouseEnterEvent) == S_OK)
         {
            BubbleEvent(pMouseEnterEvent);
         }
      }
   }

   cAutoIPtr<IGUIElement> pElement;
   if (GetEventTarget(pInputEvent, &pElement))
   {
      if (eventCode == kGUIEventMouseDown)
      {
         SetFocus(pElement);
      }
      else if (eventCode == kGUIEventMouseUp)
      {
         if (pElement->Contains(pInputEvent->point))
         {
            // TODO: Doing this here, the click event will occur before the mouse up event
            // Not sure if that is the right thing
            cAutoIPtr<IGUIEvent> pClickEvent;
            if (GUIEventCreate(kGUIEventClick, pInputEvent->point, pInputEvent->key, 
               pElement, &pClickEvent) == S_OK)
            {
               BubbleEvent(pClickEvent);
            }
         }
      }

      cAutoIPtr<IGUIEvent> pEvent;
      if (GUIEventCreate(eventCode, pInputEvent->point, pInputEvent->key, pElement, &pEvent) == S_OK)
      {
         return BubbleEvent(pEvent);
      }
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIEVENTROUTERTEM_H
