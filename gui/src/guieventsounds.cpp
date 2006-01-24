///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guieventsounds.h"

#include "soundapi.h"

#include "multivar.h"
#include "resourceapi.h"

#include <tinyxml.h>

#include "dbgalloc.h" // must be last header


#undef PlaySound // TODO: tinyxml.h should not include windows.h

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIEventSounds
//

////////////////////////////////////////

BEGIN_CONSTRAINTS(cGUIEventSounds)
   AFTER_GUID(IID_IGUIContext)
   AFTER_GUID(IID_IScriptInterpreter)
END_CONSTRAINTS()

////////////////////////////////////////

cGUIEventSounds::cGUIEventSounds(const tChar * pszScriptName)
 : m_scriptName((pszScriptName != NULL) ? pszScriptName : _T(""))
{
}

////////////////////////////////////////

cGUIEventSounds::~cGUIEventSounds()
{
}

////////////////////////////////////////

tResult cGUIEventSounds::Init()
{
   UseGlobal(GUIContext);
   pGUIContext->AddEventListener(static_cast<IGUIEventListener*>(this));
   UseGlobal(ScriptInterpreter);
   pScriptInterpreter->AddNamedItem(GetScriptName(), static_cast<IScriptable*>(this));
   return S_OK;
}

////////////////////////////////////////

tResult cGUIEventSounds::Term()
{
   UseGlobal(ScriptInterpreter);
   pScriptInterpreter->RemoveNamedItem(GetScriptName());
   UseGlobal(GUIContext);
   pGUIContext->RemoveEventListener(static_cast<IGUIEventListener*>(this));
   return S_OK;
}

////////////////////////////////////////

tResult cGUIEventSounds::SetEventSound(tGUIEventCode eventCode, const tChar * pszSound)
{
   if (eventCode == kGUIEventNone)
   {
      return E_INVALIDARG;
   }
   if (pszSound == NULL)
   {
      return E_POINTER;
   }
   m_eventSoundMap[eventCode] = pszSound;
   return S_OK;
}

////////////////////////////////////////

tResult cGUIEventSounds::GetEventSound(tGUIEventCode eventCode, cStr * pSound) const
{
   if (eventCode == kGUIEventNone)
   {
      return E_INVALIDARG;
   }
   if (pSound == NULL)
   {
      return E_POINTER;
   }
   tGUIEventSoundMap::const_iterator f = m_eventSoundMap.find(eventCode);
   if (f == m_eventSoundMap.end())
   {
      return S_FALSE;
   }
   *pSound = f->second;
   return S_OK;
}

////////////////////////////////////////

tResult cGUIEventSounds::ClearEventSound(tGUIEventCode eventCode)
{
   if (eventCode == kGUIEventNone)
   {
      return E_INVALIDARG;
   }

   tGUIEventSoundMap::size_type nErased = m_eventSoundMap.erase(eventCode);
   return (nErased == 0) ? S_FALSE : S_OK;
}

////////////////////////////////////////

void cGUIEventSounds::ClearAll()
{
   m_eventSoundMap.clear();
}

////////////////////////////////////////

tResult cGUIEventSounds::OnEvent(IGUIEvent * pEvent)
{
   tGUIEventCode eventCode;
   if (pEvent != NULL && pEvent->GetEventCode(&eventCode) == S_OK)
   {
      tGUIEventSoundMap::const_iterator f = m_eventSoundMap.find(eventCode);
      if (f != m_eventSoundMap.end())
      {
         PlaySound(f->second.c_str());
      }
   }

   return S_OK;
}

////////////////////////////////////////

static const struct
{
   tGUIEventCode code;
   const tChar * pszName;
}
g_guiEventNames[] =
{
   //{ kGUIEventNone, "None" },
   { kGUIEventFocus,             "Focus" },
   { kGUIEventBlur,              "Blur" },
   { kGUIEventDestroy,           "Destroy" },
   { kGUIEventMouseMove,         "MouseMove" },
   { kGUIEventMouseEnter,        "MouseEnter" },
   { kGUIEventMouseLeave,        "MouseLeave" },
   { kGUIEventMouseUp,           "MouseUp" },
   { kGUIEventMouseDown,         "MouseDown" },
   { kGUIEventMouseWheelUp,      "MouseWheelUp" },
   { kGUIEventMouseWheelDown,    "MouseWheelDown" },
   { kGUIEventKeyUp,             "KeyUp" },
   { kGUIEventKeyDown,           "KeyDown" },
   { kGUIEventClick,             "Click" },
   { kGUIEventHover,             "Hover" },
   { kGUIEventDragStart,         "DragStart" },
   { kGUIEventDragEnd,           "DragEnd" },
   { kGUIEventDragMove,          "DragMove" },
   { kGUIEventDragOver,          "DragOver" },
   { kGUIEventDrop,              "Drop" },
};

tResult cGUIEventSounds::Invoke(const char * pszMethodName,
                                int argc, const tScriptVar * argv,
                                int nMaxResults, tScriptVar * pResults)
{
   if (pszMethodName == NULL || argv == NULL)
   {
      return E_POINTER;
   }

   if (_tcsicmp(pszMethodName, "SetEventSound") == 0)
   {
      if (argc != 2 || !argv[0].IsString() || !argv[1].IsString())
      {
         return E_INVALIDARG;
      }

      tGUIEventCode code = kGUIEventNone;
      for (int i = 0; i < _countof(g_guiEventNames); i++)
      {
         if (_tcsicmp(g_guiEventNames[i].pszName, argv[0].ToString()) == 0)
         {
            code = g_guiEventNames[i].code;
            break;
         }
      }

      if (code == kGUIEventNone)
      {
         //WarnMsg
         return E_FAIL;
      }

      if (SetEventSound(code, argv[1].ToString()) != S_OK)
      {
         return E_FAIL;
      }

      return S_OK;
   }

   return E_NOTIMPL;
}

////////////////////////////////////////

void cGUIEventSounds::PlaySound(const tChar * pszSound)
{
   tSoundId soundId;
   UseGlobal(ResourceManager);
   if (pResourceManager->Load(pszSound, kRT_WavSound, NULL, (void**)&soundId) == S_OK)
   {
      UseGlobal(SoundManager);
      pSoundManager->Play(soundId);
   }
}

////////////////////////////////////////

tResult GUIEventSoundsCreate(const tChar * pszScriptName)
{
   cAutoIPtr<IGUIEventSounds> pEventSounds(static_cast<IGUIEventSounds*>(new cGUIEventSounds(pszScriptName)));
   if (!pEventSounds)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IGUIEventSounds, pEventSounds);
}

///////////////////////////////////////////////////////////////////////////////
