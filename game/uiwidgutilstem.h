///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_UIWIDGUTILSTEM_H
#define INCLUDED_UIWIDGUTILSTEM_H

#include "keys.h"
#include "script.h"
#include "inputapi.h"
#include "globalobj.h"

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
   UseGlobal(Input);
   Assert(pInput->Disconnect(this) != S_OK); // should have already been removed
}

///////////////////////////////////////

template <class T>
bool cUIDragSemantics<T>::FilterEvent(const cUIEvent * pEvent)
{
   T * pT = static_cast<T *>(this);

   if (pEvent->code == kEventMouseDown &&
       pEvent->keyCode == kMouseLeft &&
       pT->QueryStartDrag(pEvent->mousePos))
   {
      UseGlobal(Input);
      pInput->Connect(this);
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
            UseGlobal(Input);
            pInput->Disconnect(this);
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
bool cUIScriptEventHandler<T>::FilterEvent(const cUIEvent * pEvent)
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
