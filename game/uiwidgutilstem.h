///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_UIWIDGUTILSTEM_H
#define INCLUDED_UIWIDGUTILSTEM_H

#include "script.h"

#ifdef _MSC_VER
#pragma once
#endif

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
