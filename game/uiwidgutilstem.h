///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_UIWIDGUTILSTEM_H
#define INCLUDED_UIWIDGUTILSTEM_H

#include "keys.h"
#include "script.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cUIDragSemantics
//

///////////////////////////////////////

template <class T>
cUIDragSemantics<T>::cUIDragSemantics()
 : m_bDragging(false)
{
}

///////////////////////////////////////

template <class T>
cUIDragSemantics<T>::~cUIDragSemantics()
{
   Assert(!InputRemoveListener(this)); // should have already been removed
}

///////////////////////////////////////

template <class T>
bool cUIDragSemantics<T>::FilterEvent(const cUIEvent * pEvent, tUIResult * pResult)
{
   T * pT = static_cast<T *>(this);

   if (pEvent->code == kEventMouseDown &&
       pEvent->keyCode == kMouseLeft &&
       pT->QueryStartDrag(pEvent->mousePos))
   {
      InputAddListener(this);
      m_bDragging = true;
      return true;
   }

   return false;
}

///////////////////////////////////////

template <class T>
inline bool cUIDragSemantics<T>::QueryStartDrag(const cUIPoint & mousePos)
{
   T * pT = static_cast<T *>(this);

   return pT->GetScreenRect().PtInside(mousePos);
}

///////////////////////////////////////

template <class T>
void cUIDragSemantics<T>::UpdateDrag(const cUIPoint & mousePos)
{
}

///////////////////////////////////////

template <class T>
void cUIDragSemantics<T>::EndDrag(const cUIPoint & mousePos)
{
}

///////////////////////////////////////

template <class T>
inline bool cUIDragSemantics<T>::IsDragging() const
{
   return m_bDragging;
}

///////////////////////////////////////

template <class T>
bool cUIDragSemantics<T>::OnKeyEvent(long key, bool down, double time)
{
   T * pT = static_cast<T *>(this);

   cUIPoint mousePos = UIGetMousePos();

   if (key == kMouseLeft)
   {
      if (IsDragging())
      {
         if (!down)
         {
            pT->EndDrag(mousePos);
            InputRemoveListener(this);
            m_bDragging = false;
         }
         return true;
      }
   }
   else if (key == kMouseMove)
   {
      if (IsDragging())
         pT->UpdateDrag(mousePos);
   }

   // intercept all messages while dragging
   if (IsDragging())
      return true;

   return false;
}


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cUIClickSemantics
//

///////////////////////////////////////

template <class T>
void cUIClickSemantics<T>::EndDrag(const cUIPoint & mousePos)
{
   T * pT = static_cast<T *>(this);
   if (pT->GetScreenRect().PtInside(mousePos))
   {
      cUIEvent clickEvent;
      clickEvent.code = kEventClick;
      clickEvent.pSrc = pT;
      clickEvent.mousePos = mousePos;
      clickEvent.keyCode = 0;
      tUIResult result;
      UIBubbleEvent(pT, &clickEvent, &result);
   }
}

///////////////////////////////////////

template <class T>
bool cUIClickSemantics<T>::IsMouseOver() const
{
   const T * pT = static_cast<const T *>(this);
   return pT->GetScreenRect().PtInside(UIGetMousePos());
}

///////////////////////////////////////

template <class T>
bool cUIClickSemantics<T>::IsPressed()
{
   return IsDragging();
}


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cUIScriptEventHandler
//

///////////////////////////////////////

template <class T>
void cUIScriptEventHandler<T>::SetEventHandler(eUIEventCode code, const char * pszScriptCode)
{
   Assert(pszScriptCode != NULL);
   m_scriptEventHandlers.erase(code);
   m_scriptEventHandlers.insert(std::make_pair(code, pszScriptCode));
}

///////////////////////////////////////

template <class T>
bool cUIScriptEventHandler<T>::FilterEvent(const cUIEvent * pEvent, tUIResult * pResult)
{
   return CallEventHandler(pEvent->code);
}

///////////////////////////////////////

template <class T>
bool cUIScriptEventHandler<T>::CallEventHandler(eUIEventCode code)
{
   tEventHandlerMap::iterator iter = m_scriptEventHandlers.find(code);
   if (iter != m_scriptEventHandlers.end())
   {
      return ScriptExecString(iter->second.c_str());
   }
   return false;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_UIWIDGUTILSTEM_H
