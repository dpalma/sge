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

extern const char * Key2Name(long key); // from cmds.cpp

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
         (size.width - pComponent->GetSize().width) * 0.5f,
         (size.height - pComponent->GetSize().height) * 0.5f);
   }
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIManager
//

///////////////////////////////////////

cUIManager::cUIManager()
 : m_pLastMouseOver(NULL),
   m_pFocus(NULL),
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
   tContainerBase::Render(pRenderDevice);
   glPopAttrib();
}

///////////////////////////////////////

cUISize cUIManager::GetSize() const
{
   return UIGetRootContainerSize();
}

///////////////////////////////////////

bool cUIManager::OnEvent(const cUIEvent * pEvent)
{
   Assert(pEvent != NULL);
   if (pEvent->code == kEventDestroy)
   {
      if (IsEventPertinent(pEvent, m_pLastMouseOver))
         m_pLastMouseOver = NULL;
      if (IsEventPertinent(pEvent, m_pFocus))
         SetFocus(NULL);
   }
   return tContainerBase::OnEvent(pEvent);
}

///////////////////////////////////////

void cUIManager::SetFocus(cUIComponent * pNewFocus)
{
   if (pNewFocus != m_pFocus)
   {
      if (pNewFocus != NULL)
      {
         if (!pNewFocus->AcceptsFocus())
            return;

         cUIEvent event;
         event.code = kEventFocus;
         event.pSrc = m_pFocus;
         UIDispatchEvent(pNewFocus, &event);
         pNewFocus->SetInternalFlags(kUICF_Focussed, kUICF_Focussed);
      }

      if (m_pFocus != NULL)
      {
         cUIEvent event;
         event.code = kEventBlur;
         event.pSrc = pNewFocus;
         UIDispatchEvent(m_pFocus, &event);
         Assert(m_pFocus->TestInternalFlags(kUICF_Focussed));
         m_pFocus->SetInternalFlags(0, kUICF_Focussed);
      }

      m_pFocus = pNewFocus;
   }
}

///////////////////////////////////////

bool cUIManager::HandleInputEvent(const sInputEvent * pEvent)
{
   cUIEvent event;
   event.code = UIEventCode(pEvent->key, pEvent->down);
   event.mousePos = pEvent->point;
   event.keyCode = pEvent->key;
   Assert(event.code != kEventERROR);

   if (KeyIsMouse(pEvent->key))
   {
      event.pSrc = HitTest(pEvent->point);

      DebugMsgIfEx1(UIMgr, event.pSrc != NULL, "Hit UI element \"%s\"\n", event.pSrc->GetId());

      if (event.pSrc != NULL)
      {
         if (event.code == kEventMouseMove)
         {
            if (event.pSrc != m_pLastMouseOver)
            {
               if (m_pLastMouseOver != NULL)
               {
                  cUIEvent mouseLeaveEvent;
                  mouseLeaveEvent.code = kEventMouseLeave;
                  mouseLeaveEvent.pSrc = m_pLastMouseOver;
                  mouseLeaveEvent.mousePos = event.mousePos;
                  UIBubbleEvent(mouseLeaveEvent.pSrc, &mouseLeaveEvent);
               }

               m_pLastMouseOver = event.pSrc;
               event.code = kEventMouseEnter;
            }
         }
         else if (event.code == kEventMouseDown)
         {
            SetFocus(event.pSrc);
         }
         else if (event.code == kEventMouseUp)
         {
            if (m_pFocus == event.pSrc)
            {
               DebugMsgEx1(UIMgr, "Click element \"%s\"\n", event.pSrc->GetId());
               cUIEvent clickEvent;
               clickEvent.code = kEventClick;
               clickEvent.pSrc = event.pSrc;
               clickEvent.mousePos = event.mousePos;
               clickEvent.keyCode = event.keyCode;
               UIBubbleEvent(clickEvent.pSrc, &clickEvent);
            }
         }

         return UIBubbleEvent(event.pSrc, &event);
      }
   }
   else
   {
      event.pSrc = m_pFocus;

      DebugMsgIfEx1(UIMgr, event.pSrc != NULL, "Key event to UI element %s\n", event.pSrc->GetId());

      if (event.pSrc != NULL)
      {
         return UIBubbleEvent(event.pSrc, &event);
      }
   }

   return false;
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

#define GetOuter(Class, Member) ((Class *)((byte *)this - offsetof(Class, Member)))

bool cUIManager::cInputListener::OnInputEvent(const sInputEvent * pEvent)
{
   cUIManager * pUIManager = GetOuter(cUIManager, m_inputListener);
   return pUIManager->HandleInputEvent(pEvent);
}

///////////////////////////////////////////////////////////////////////////////
