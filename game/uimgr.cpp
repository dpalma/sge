///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ui.h"
#include "uimgr.h"
#include "uievent.h"
#include "uirender.h"
#include "uiparse.h"
#include "keys.h"
#include "input.h"

#include "dbgalloc.h" // must be last header

bool IsEventPertinent(const cUIEvent *, const cUIComponent *); // from ui.cpp

// {A1EC19A0-E2DC-4652-BF61-C848DB00DC6A}
const GUID IID_IUIManager = 
{ 0xa1ec19a0, 0xe2dc, 0x4652, { 0xbf, 0x61, 0xc8, 0x48, 0xdb, 0x0, 0xdc, 0x6a } };

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

class cUIManager : public cComObject<IMPLEMENTS(IUIManager)>,
                   private cUIContainer,
                   public cComObject<cDefaultInputListener, &IID_IInputListener>
{
   typedef cUIContainer tContainerBase;

public:
   cUIManager();
   virtual ~cUIManager();

   ////////////////////////////////////
   // IUIManager methods

   virtual void ShowModalDialog(const char * pszXmlFile);

   virtual void Render();

   ////////////////////////////////////
   // cUIContainer/cUIComponent over-rides

   virtual cUISize GetSize() const;
   virtual bool OnEvent(const cUIEvent * pEvent, tUIResult * pResult);

   ////////////////////////////////////
   // cDefaultInputListener methods

   virtual bool OnKeyEvent(long key, bool down, double time);

private:
   class cDialogParseHook : public cUIParseHook
   {
   public:
      cDialogParseHook();
      virtual ~cDialogParseHook();
   
      virtual eSkipResult SkipElement(const char * pszElement);

   private:
      int m_nDlgsSeen;
   };

   void PreBubbleEvent(cUIComponent * pEventTarget, cUIEvent * pEvent);

   cUIComponent * m_pLastMouseOver;
};

///////////////////////////////////////

cUIManager::cUIManager()
 : m_pLastMouseOver(NULL)
{
   // if this happens too early (e.g., at static initialization time)
   // it will likely cause problems
   InputAddListener(this);
}

///////////////////////////////////////

cUIManager::~cUIManager()
{
   InputRemoveListener(this);
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

void cUIManager::Render()
{
   tContainerBase::Render();
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

bool cUIManager::OnKeyEvent(long key, bool down, double time)
{
   cUIComponent * pTarget = NULL;
   cUIEvent event;
   if (TranslateKeyEvent(key, down, time, &pTarget, &event))
   {
      PreBubbleEvent(pTarget, &event);

      tUIResult result;
      return UIBubbleEvent(pTarget, &event, &result);
   }

   return false;
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

IUIManager * UIManagerCreate()
{
   return static_cast<IUIManager *>(new cUIManager);
}

///////////////////////////////////////////////////////////////////////////////
