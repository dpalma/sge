///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "uimgr.h"
#include "uievent.h"
#include "uirender.h"
#include "ggl.h"
#include "sceneapi.h"
#include "globalobj.h"
#include "keys.h"

#include "dbgalloc.h" // must be last header

bool IsEventPertinent(const cUIEvent *, const cUIComponent *); // from ui.cpp

LOG_DEFINE_CHANNEL(UIMgr);

///////////////////////////////////////////////////////////////////////////////

static void Center(cUIComponent * pComponent)
{
   Assert(pComponent != NULL);

   cUIComponent * pParent = pComponent->GetParent();

   if (pParent != NULL)
   {
      cUISize size = pParent->GetSize();

      pComponent->SetPos(
         (size.width - pComponent->GetSize().width) * 0.5,
         (size.height - pComponent->GetSize().height) * 0.5);
   }
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIManager
//

///////////////////////////////////////

cUIManager::cUIManager()
 : m_pLastMouseOver(NULL),
   m_pSceneEntity(SceneEntityCreate())
{
   UseGlobal(Scene);
   pScene->AddInputListener(kSL_InGameUI, &m_inputListener);
}

///////////////////////////////////////

cUIManager::~cUIManager()
{
   UseGlobal(Scene);
   pScene->RemoveInputListener(kSL_InGameUI, &m_inputListener);
}

///////////////////////////////////////

void cUIManager::ShowModalDialog(const char * pszXmlFile)
{
   cDialogParseHook hook;

   tUIComponentList components;
   if (UIParseFile(pszXmlFile, &components, &hook))
   {
      DebugMsgIf1(components.size() > 1,
         "More than one (%d) UI components parsed in ShowModalDialog\n",
         components.size());

      tUIComponentList::iterator iter;
      for (iter = components.begin(); iter != components.end(); iter++)
      {
         AddComponent(*iter);
         Center(*iter);
      }
   }
}

///////////////////////////////////////
// assumes renderer is already set up for orthographic (2D) projection and stuff

void cUIManager::Render(IRenderDevice * pRenderDevice)
{
   glPushAttrib(GL_ENABLE_BIT);
   glDisable(GL_DEPTH_TEST);
   tContainerBase::Render();
   glPopAttrib();
}

///////////////////////////////////////

cUISize cUIManager::GetSize() const
{
   return UIGetRootContainerSize();
}

///////////////////////////////////////

bool cUIManager::OnEvent(const cUIEvent * pEvent, tUIResult * pResult)
{
   Assert(pEvent != NULL);
   if (pEvent->code == kEventDestroy)
   {
      if (IsEventPertinent(pEvent, m_pLastMouseOver))
         m_pLastMouseOver = NULL;
   }
   return tContainerBase::OnEvent(pEvent, pResult);
}

///////////////////////////////////////

void cUIManager::PreBubbleEvent(cUIComponent * pEventTarget, cUIEvent * pEvent)
{
   Assert(pEventTarget != NULL);
   Assert(pEvent != NULL);

   if (pEvent->code == kEventMouseMove)
   {
      if (pEventTarget != m_pLastMouseOver)
      {
         if (m_pLastMouseOver != NULL)
         {
            cUIEvent mouseLeaveEvent;
            mouseLeaveEvent.code = kEventMouseLeave;
            mouseLeaveEvent.pSrc = m_pLastMouseOver;
            mouseLeaveEvent.mousePos = pEvent->mousePos;
            tUIResult result;
            UIBubbleEvent(m_pLastMouseOver, &mouseLeaveEvent, &result);
         }

         m_pLastMouseOver = pEventTarget;
         pEvent->code = kEventMouseEnter;
      }
   }
   else if (pEvent->code == kEventMouseDown)
   {
      SetFocus(pEventTarget);
   }
}

///////////////////////////////////////

cUIManager::cDialogParseHook::cDialogParseHook()
 : m_nDlgsSeen(0)
{
}

///////////////////////////////////////

cUIManager::cDialogParseHook::~cDialogParseHook()
{
}

///////////////////////////////////////
// Allow only the first dialog definition to be created. Used by ShowModalDialog().

eSkipResult cUIManager::cDialogParseHook::SkipElement(const char * pszElement)
{
   Assert(pszElement != NULL);
   if (strcmp(pszElement, "dialog") == 0)
   {
      if (m_nDlgsSeen++ == 0)
         return kNoSkip;
   }
   return kSkipEntire;
}

///////////////////////////////////////

#define GetOuter(Class, Member) ((Class *)((byte *)this - (byte *)&((Class *)NULL)->Member))

bool cUIManager::cInputListener::OnKeyEvent(long key, bool down, double time)
{
   cUIManager * pUIManager = GetOuter(cUIManager, m_inputListener);
   cUIComponent * pTarget = NULL;
   cUIEvent event;
   if (pUIManager->TranslateKeyEvent(key, down, time, &pTarget, &event))
   {
      pUIManager->PreBubbleEvent(pTarget, &event);

      tUIResult result;
      return UIBubbleEvent(pTarget, &event, &result);
   }

   return false;
}

bool cUIManager::cInputListener::OnInputEvent(const sInputEvent * pEvent)
{
   cUIManager * pUIManager = GetOuter(cUIManager, m_inputListener);
   if (KeyIsMouse(pEvent->key))
   {
      cUIComponent * pTarget = pUIManager->HitTest(pEvent->point);
      DebugMsgIfEx1(UIMgr, pTarget != NULL, "Hit UI element %s\n", pTarget->GetId());
   }
   return false;
}

///////////////////////////////////////////////////////////////////////////////
