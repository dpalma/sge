///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guicontext.h"

#include "sceneapi.h"

#include "keys.h"
#include "resmgr.h"
#include "readwriteapi.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIContext
//

BEGIN_CONSTRAINTS()
   AFTER_GUID(IID_IScene)
END_CONSTRAINTS()

///////////////////////////////////////

cGUIContext::cGUIContext()
 : cGlobalObject<IMPLEMENTSCP(IGUIContext, IGUIEventListener)>("GUIContext", CONSTRAINTS())
{
   UseGlobal(Scene);
   pScene->AddInputListener(kSL_InGameUI, &m_inputListener);
}

///////////////////////////////////////

cGUIContext::~cGUIContext()
{
   UseGlobal(Scene);
   pScene->RemoveInputListener(kSL_InGameUI, &m_inputListener);
}

///////////////////////////////////////

tResult cGUIContext::Init()
{
   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::Term()
{
   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::AddEventListener(IGUIEventListener * pListener)
{
   return Connect(pListener);
}

///////////////////////////////////////

tResult cGUIContext::RemoveEventListener(IGUIEventListener * pListener)
{
   return Disconnect(pListener);
}

///////////////////////////////////////

tResult cGUIContext::GetFocus(IGUIElement * * ppElement)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cGUIContext::SetFocus(IGUIElement * pElement)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cGUIContext::GetCapture(IGUIElement * * ppElement)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cGUIContext::SetCapture(IGUIElement * pElement)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cGUIContext::LoadFromResource(const char * psz)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cGUIContext::LoadFromString(const char * psz)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

bool cGUIContext::HandleInputEvent(const sInputEvent * pEvent)
{
   if (KeyIsMouse(pEvent->key))
   {
   }
   else
   {
   }

   return false;
}

///////////////////////////////////////

bool cGUIContext::cInputListener::OnInputEvent(const sInputEvent * pEvent)
{
   cGUIContext * pOuter = CTGetOuter(cGUIContext, m_inputListener);
   return false;
}

///////////////////////////////////////

void GUIContextCreate()
{
   cAutoIPtr<IGUIContext>(new cGUIContext);
}

///////////////////////////////////////////////////////////////////////////////
