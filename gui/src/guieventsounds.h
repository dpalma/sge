///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIEVENTSOUNDS_H
#define INCLUDED_GUIEVENTSOUNDS_H

#include "gui/guiapi.h"

#include "script/scriptapi.h"

#include "tech/globalobjdef.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIEventSounds
//

class cGUIEventSounds : public cComObject4<IMPLEMENTS(IGUIEventSounds),
                                           IMPLEMENTS(IGUIEventListener),
                                           IMPLEMENTS(IScriptable),
                                           IMPLEMENTS(IGlobalObject)>
{
public:
   cGUIEventSounds(const tChar * pszScriptName);
   ~cGUIEventSounds();

   DECLARE_NAME(GUIEventSounds)
   DECLARE_CONSTRAINTS()

   // IGlobalObject methods
   virtual tResult Init();
   virtual tResult Term();

   virtual tResult SetEventSound(tGUIEventCode eventCode, const tChar * pszSound);
   virtual tResult GetEventSound(tGUIEventCode eventCode, cStr * pSound) const;
   virtual tResult ClearEventSound(tGUIEventCode eventCode);
   virtual void ClearAll();

   // IGUIEventListener methods
   virtual tResult OnEvent(IGUIEvent * pEvent);

   // IScriptable methods
   virtual tResult Invoke(const char * pszMethodName,
                          int argc, const tScriptVar * argv,
                          int nMaxResults, tScriptVar * pResults);

private:
   inline const tChar * GetScriptName() const
   {
      return m_scriptName.empty() ? GetName() : m_scriptName.c_str();
   }

   void PlaySound(const tChar * pszSound);

   cStr m_scriptName;

   typedef std::map<tGUIEventCode, cStr> tGUIEventSoundMap;
   tGUIEventSoundMap m_eventSoundMap;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIEVENTSOUNDS_H
