///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIEVENTROUTERTEM_H
#define INCLUDED_GUIEVENTROUTERTEM_H

/// @file guieventroutertem.h
/// Contains the member function implementations of the cGUIEventRouter class.
/// This file is intended to be included only by cpp files, not other headers.

#include "gui/guiapi.h"

#include "guielementtools.h"
#include "guievent.h"

#include "tech/globalobj.h"

#include <algorithm>

#include "tech/dbgalloc.h" // must be last header

#ifdef _MSC_VER
#pragma once
#endif

LOG_EXTERN_CHANNEL(GUIEventRouter); // defined in guieventrouter.cpp

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cGUIEventRouter
//

///////////////////////////////////////

template <typename T, typename INTRFC>
cGUIEventRouter<T, INTRFC>::cGUIEventRouter()
{
}

///////////////////////////////////////

template <typename T, typename INTRFC>
cGUIEventRouter<T, INTRFC>::~cGUIEventRouter()
{
}

///////////////////////////////////////

template <typename T, typename INTRFC>
tResult cGUIEventRouter<T, INTRFC>::AddEventListener(IGUIEventListener * pListener)
{
   return tBaseClass::Connect(pListener);
}

///////////////////////////////////////

template <typename T, typename INTRFC>
tResult cGUIEventRouter<T, INTRFC>::RemoveEventListener(IGUIEventListener * pListener)
{
   return tBaseClass::Disconnect(pListener);
}

///////////////////////////////////////

template <typename T, typename INTRFC>
tResult cGUIEventRouter<T, INTRFC>::GetFocus(IGUIElement * * ppElement)
{
   Assert(!m_pFocus || m_pFocus->HasFocus());
   return m_pFocus.GetPointer(ppElement);
}

///////////////////////////////////////

template <typename T, typename INTRFC>
tResult cGUIEventRouter<T, INTRFC>::SetFocus(IGUIElement * pElement)
{
   if (!!m_pFocus)
   {
      m_pFocus->SetFocus(false);
   }
   SafeRelease(m_pFocus);
   m_pFocus = CTAddRef(pElement);
   if (pElement != NULL)
   {
      pElement->SetFocus(true);
   }
   return S_OK;
}

///////////////////////////////////////

template <typename T, typename INTRFC>
tResult cGUIEventRouter<T, INTRFC>::GetMouseOver(IGUIElement * * ppElement)
{
   return m_pMouseOver.GetPointer(ppElement);
}

///////////////////////////////////////

template <typename T, typename INTRFC>
tResult cGUIEventRouter<T, INTRFC>::SetMouseOver(IGUIElement * pElement)
{
   SafeRelease(m_pMouseOver);
   m_pMouseOver = CTAddRef(pElement);
   return S_OK;
}

///////////////////////////////////////

template <typename T, typename INTRFC>
tResult cGUIEventRouter<T, INTRFC>::GetDrag(IGUIElement * * ppElement)
{
   return m_pDrag.GetPointer(ppElement);
}

///////////////////////////////////////

template <typename T, typename INTRFC>
tResult cGUIEventRouter<T, INTRFC>::SetDrag(IGUIElement * pElement)
{
   SafeRelease(m_pDrag);
   m_pDrag = CTAddRef(pElement);
   return S_OK;
}

///////////////////////////////////////

template <typename T, typename INTRFC>
void cGUIEventRouter<T, INTRFC>::ElementRemoved(IGUIElement * pElement)
{
   if (pElement != NULL)
   {
      // If the focus, moused-over, or drag elements are descendants of 
      // the element being removed, release the pointers.
      if (!!m_pFocus && IsDescendant(pElement, m_pFocus))
      {
         SafeRelease(m_pFocus);
      }
      if (!!m_pMouseOver && IsDescendant(pElement, m_pMouseOver))
      {
         SafeRelease(m_pMouseOver);
      }
      if (!!m_pDrag && IsDescendant(pElement, m_pDrag))
      {
         SafeRelease(m_pDrag);
      }
   }
}

///////////////////////////////////////
// Similar to BubbleEvent but doesn't walk up the parent chain

template <typename T, typename INTRFC>
bool cGUIEventRouter<T, INTRFC>::DoEvent(IGUIEvent * pEvent)
{
   Assert(pEvent != NULL);

   pEvent->SetCancelBubble(true);
   return BubbleEvent(pEvent);
}

///////////////////////////////////////

template <typename T, typename INTRFC>
bool cGUIEventRouter<T, INTRFC>::BubbleEvent(IGUIEvent * pEvent)
{
   Assert(pEvent != NULL);

   cAutoIPtr<IGUIElement> pEventSrc;
   if (pEvent->GetSourceElement(&pEventSrc) == S_OK)
   {
      return BubbleEvent(pEventSrc, pEvent);
   }

   return false;
}

///////////////////////////////////////

template <typename T, typename INTRFC>
bool cGUIEventRouter<T, INTRFC>::BubbleEvent(IGUIElement * pStartElement, IGUIEvent * pEvent)
{
   Assert(pStartElement != NULL);
   Assert(pEvent != NULL);

   typename cConnectionPoint<INTRFC, IGUIEventListener>::tSinksIterator iter = tBaseClass::BeginSinks();
   typename cConnectionPoint<INTRFC, IGUIEventListener>::tSinksIterator end = tBaseClass::EndSinks();
   for (; iter != end; ++iter)
   {
      if ((*iter)->OnEvent(pEvent) != S_OK)
      {
         return true;
      }
   }

   cAutoIPtr<IGUIElement> pDispatchTo(CTAddRef(pStartElement));
   while (!!pDispatchTo)
   {
      if (pDispatchTo->OnEvent(pEvent) != S_OK)
      {
         return true;
      }

      if (pEvent->GetCancelBubble() == S_OK)
      {
         break;
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

   return false;
}

///////////////////////////////////////

template <typename T, typename INTRFC>
void cGUIEventRouter<T, INTRFC>::DoMouseEnterExit(const sInputEvent * pInputEvent,
                                                  IGUIElement * pMouseOver,
                                                  IGUIElement * pRestrictTo)
{
   if (pMouseOver != NULL)
   {
      bool bMouseOverSame = false;

      cAutoIPtr<IGUIElement> pOldMouseOver;
      if (GetMouseOver(&pOldMouseOver) == S_OK)
      {
         bMouseOverSame = CTIsSameObject(pMouseOver, pOldMouseOver);

         if (!bMouseOverSame)
         {
            SetMouseOver(NULL);
            pOldMouseOver->SetMouseOver(false);

            cAutoIPtr<IGUIEvent> pMouseLeaveEvent;
            if (GUIEventCreate(kGUIEventMouseLeave, pInputEvent->point, pInputEvent->key, pInputEvent->modifierKeys,
               pOldMouseOver, true, &pMouseLeaveEvent) == S_OK)
            {
               DoEvent(pMouseLeaveEvent);
            }
         }
      }

      if (!bMouseOverSame 
         && ((pRestrictTo == NULL) || (CTIsSameObject(pMouseOver, pRestrictTo))))
      {
         SetMouseOver(pMouseOver);
         pMouseOver->SetMouseOver(true);

         cAutoIPtr<IGUIEvent> pMouseEnterEvent;
         if (GUIEventCreate(kGUIEventMouseEnter, pInputEvent->point, pInputEvent->key, pInputEvent->modifierKeys,
            pMouseOver, true, &pMouseEnterEvent) == S_OK)
         {
            DoEvent(pMouseEnterEvent);
         }
      }
   }
   else
   {
      cAutoIPtr<IGUIElement> pOldMouseOver;
      if (GetMouseOver(&pOldMouseOver) == S_OK)
      {
         tGUIPoint absPos = GUIElementAbsolutePosition(pOldMouseOver);

         tGUIPoint relPoint(pInputEvent->point.x - absPos.x, pInputEvent->point.y - absPos.y); // TODO: ADDED_tScreenPoint

         SetMouseOver(NULL);
         pOldMouseOver->SetMouseOver(false);

         cAutoIPtr<IGUIEvent> pMouseLeaveEvent;
         if (GUIEventCreate(kGUIEventMouseLeave, pInputEvent->point, pInputEvent->key, pInputEvent->modifierKeys,
            pOldMouseOver, true, &pMouseLeaveEvent) == S_OK)
         {
            DoEvent(pMouseLeaveEvent);
         }
      }
   }
}

///////////////////////////////////////

template <typename T, typename INTRFC>
bool cGUIEventRouter<T, INTRFC>::HandleInputEvent(const sInputEvent * pInputEvent)
{
   DebugMsgIfEx5(GUIEventRouter, pInputEvent->key != kMouseMove,
      "InputEvent: key %d, down %d, point(%d, %d), time %f\n",
      pInputEvent->key, static_cast<int>(pInputEvent->down),
      pInputEvent->point.x, pInputEvent->point.y,
      pInputEvent->time);

   tGUIEventCode eventCode = GUIEventCode(pInputEvent->key, pInputEvent->down);
   if (eventCode == kGUIEventNone)
   {
      WarnMsg("Invalid event code\n");
      return false;
   }

   T * pT = static_cast<T*>(this);

   bool bEatInputEvent = false;

   if (KeyIsMouse(pInputEvent->key))
   {
      cAutoIPtr<IGUIElement> pMouseOver;
      if (pT->GetHitElement(pInputEvent->point, &pMouseOver) != S_OK)
      {
         Assert(!pMouseOver);
      }

      if (eventCode == kGUIEventMouseMove)
      {
         DoMouseEnterExit(pInputEvent, pMouseOver, NULL);
         return true;
      }

      cAutoIPtr<IGUIElement> pDrag;
      if (GetDrag(&pDrag) != S_OK)
      {
         Assert(!pDrag);
      }

      if (!!pMouseOver && pMouseOver->IsEnabled())
      {
         cAutoIPtr<IGUIEvent> pEvent;
         if (GUIEventCreate(eventCode, pInputEvent->point, 
            pInputEvent->key, pInputEvent->modifierKeys, pMouseOver, true,
            &pEvent) == S_OK)
         {
            bEatInputEvent = BubbleEvent(pEvent);
         }

         if (!bEatInputEvent)
         {
            if (eventCode == kGUIEventMouseDown)
            {
               SetFocus(pMouseOver);
               SetDrag(pMouseOver);
            }
            else if (eventCode == kGUIEventMouseUp)
            {
               SetDrag(NULL);

               // If moused-over same as dragging element
               if (CTIsSameObject(pMouseOver, pDrag))
               {
                  // Send click to moused-over/dragging element
                  cAutoIPtr<IGUIEvent> pClickEvent;
                  if (GUIEventCreate(kGUIEventClick, pInputEvent->point, 
                     pInputEvent->key, pInputEvent->modifierKeys, pMouseOver, true,
                     &pClickEvent) == S_OK)
                  {
                     BubbleEvent(pClickEvent);
                     bEatInputEvent = true;
                  }
               }
            }
         }
      }
   }
   else
   {
      cAutoIPtr<IGUIElement> pFocus;
      if (GetFocus(&pFocus) == S_OK)
      {
         Assert(!!pFocus);
         cAutoIPtr<IGUIEvent> pEvent;
         if (GUIEventCreate(eventCode, pInputEvent->point, 
            pInputEvent->key, pInputEvent->modifierKeys, pFocus, true,
            &pEvent) == S_OK)
         {
            bEatInputEvent = BubbleEvent(pEvent);
         }
      }
   }

   return bEatInputEvent;
}


///////////////////////////////////////////////////////////////////////////////

#include "tech/undbgalloc.h"

#endif // !INCLUDED_GUIEVENTROUTERTEM_H
