///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guicontext.h"
#include "guievent.h"

#include "sceneapi.h"

#include "render.h"

#include "keys.h"
#include "resmgr.h"
#include "readwriteapi.h"

#include <tinyxml.h>

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
   SafeRelease(m_pFocus);
   SafeRelease(m_pCapture);
   SafeRelease(m_pMouseOver);
   std::for_each(m_elements.begin(), m_elements.end(), CTInterfaceMethodRef(&IGUIElement::Release));
   m_elements.clear();
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
   return m_pFocus.GetPointer(ppElement);
}

///////////////////////////////////////

tResult cGUIContext::SetFocus(IGUIElement * pElement)
{
   SafeRelease(m_pFocus);
   m_pFocus = CTAddRef(pElement);
   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::GetCapture(IGUIElement * * ppElement)
{
   return m_pCapture.GetPointer(ppElement);
}

///////////////////////////////////////

tResult cGUIContext::SetCapture(IGUIElement * pElement)
{
   SafeRelease(m_pCapture);
   m_pCapture = CTAddRef(pElement);
   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::GetMouseOver(IGUIElement * * ppElement)
{
   return m_pMouseOver.GetPointer(ppElement);
}

///////////////////////////////////////

tResult cGUIContext::SetMouseOver(IGUIElement * pElement)
{
   SafeRelease(m_pMouseOver);
   m_pMouseOver = CTAddRef(pElement);
   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::LoadFromResource(const char * psz)
{
   UseGlobal(ResourceManager);
   cAutoIPtr<IReader> pReader = pResourceManager->Find(psz);
   if (!pReader)
      return E_FAIL;

   pReader->Seek(0, kSO_End);
   int len = pReader->Tell();
   pReader->Seek(0, kSO_Set);

   char * pszContents = new char[len + 1];
   if (pszContents == NULL)
      return E_OUTOFMEMORY;

   if (pReader->Read(pszContents, len) != S_OK)
   {
      delete [] pszContents;
      return E_FAIL;
   }

   pszContents[len] = 0;

   tResult result = LoadFromString(pszContents);

   delete [] pszContents;

   return result;
}

///////////////////////////////////////

tResult cGUIContext::LoadFromString(const char * psz)
{
   TiXmlDocument doc;
   doc.Parse(psz);

   if (doc.Error())
   {
      DebugMsg1("TiXml parse error: %s\n", doc.ErrorDesc());
      return E_FAIL;
   }

   UseGlobal(GUIFactory);

   ulong nElementsCreated = 0;

   TiXmlElement * pXmlElement;
   for (pXmlElement = doc.FirstChildElement(); pXmlElement != NULL; pXmlElement = pXmlElement->NextSiblingElement())
   {
      if (pXmlElement->Type() == TiXmlNode::ELEMENT)
      {
         cAutoIPtr<IGUIElement> pGUIElement;
         if (pGUIFactory->CreateElement(pXmlElement->Value(), pXmlElement, &pGUIElement) == S_OK)
         {
            nElementsCreated++;
            m_elements.push_back(CTAddRef(pGUIElement));
         }
      }
   }

   return nElementsCreated;
}

///////////////////////////////////////

tResult cGUIContext::RenderGUI(IRenderDevice * pRenderDevice)
{
   Assert(pRenderDevice != NULL);

   tResult result = S_OK;

   pRenderDevice->SetRenderState(kRS_EnableDepthBuffer, FALSE);

   tGUIElementList::iterator iter;
   for (iter = m_elements.begin(); iter != m_elements.end(); iter++)
   {
      if ((*iter)->IsVisible())
      {
         cAutoIPtr<IGUIElementRenderer> pRenderer;
         if ((*iter)->GetRenderer(&pRenderer) == S_OK)
         {
            if ((result = pRenderer->Render(*iter, pRenderDevice)) != S_OK)
            {
               DebugMsg("WARNING: Error during GUI rendering\n");
               break;
            }
         }
      }
   }

   pRenderDevice->SetRenderState(kRS_EnableDepthBuffer, TRUE);

   return result;
}

///////////////////////////////////////

bool cGUIContext::BubbleEvent(IGUIEvent * pEvent)
{
   Assert(pEvent != NULL);

   cAutoIPtr<IGUIElement> pDispatchTo;
   if (pEvent->GetSourceElement(&pDispatchTo) == S_OK)
   {
      while (!!pDispatchTo)
      {
         if (pDispatchTo->OnEvent(pEvent))
         {
            return true;
         }

         if (pDispatchTo->GetParent(&pDispatchTo) != S_OK)
         {
            SafeRelease(pDispatchTo);
         }
      }
   }

   return false;
}

///////////////////////////////////////

bool cGUIContext::HandleInputEvent(const sInputEvent * pInputEvent)
{
   tGUIEventCode eventCode = GUIEventCode(pInputEvent->key, pInputEvent->down);
   if (eventCode == kGUIEventNone)
   {
      DebugMsg("WARNING: Invalid event code\n");
      return false;
   }

   if (KeyIsMouse(pInputEvent->key))
   {
      cAutoIPtr<IGUIElement> pElement;

      tGUIElementList::iterator iter;
      for (iter = m_elements.begin(); iter != m_elements.end(); iter++)
      {
         if ((*iter)->Contains(pInputEvent->point))
         {
            pElement = CTAddRef(*iter);
            break;
         }
      }

      if (!!pElement)
      {
         if (eventCode == kGUIEventMouseMove)
         {
            if (!CTIsSameObject(pElement, AccessMouseOver()))
            {
               cAutoIPtr<IGUIElement> pMouseOver;
               if (GetMouseOver(&pMouseOver) == S_OK)
               {
                  cAutoIPtr<IGUIEvent> pMouseLeaveEvent;
                  if (GUIEventCreate(kGUIEventMouseLeave, pInputEvent->point, pInputEvent->key, 
                     AccessMouseOver(), &pMouseLeaveEvent) == S_OK)
                  {
                     BubbleEvent(pMouseLeaveEvent);
                  }
               }

               SetMouseOver(pElement);
               eventCode = kGUIEventMouseEnter;
            }
         }
         else if (eventCode == kGUIEventMouseDown)
         {
            SetFocus(pElement);
         }
         else if (eventCode == kGUIEventMouseUp)
         {
            if (CTIsSameObject(pElement, AccessFocus()))
            {
               cAutoIPtr<IGUIEvent> pClickEvent;
               if (GUIEventCreate(kGUIEventClick, pInputEvent->point, pInputEvent->key, pElement, &pClickEvent) == S_OK)
               {
                  BubbleEvent(pClickEvent);
               }
            }
         }

         cAutoIPtr<IGUIEvent> pEvent;
         if (GUIEventCreate(eventCode, pInputEvent->point, pInputEvent->key, pElement, &pEvent) == S_OK)
         {
            return BubbleEvent(pEvent);
         }
      }
   }
   else
   {
      cAutoIPtr<IGUIElement> pElement;
      if (GetFocus(&pElement) == S_OK)
      {
         cAutoIPtr<IGUIEvent> pEvent;
         if (GUIEventCreate(eventCode, pInputEvent->point, pInputEvent->key, pElement, &pEvent) == S_OK)
         {
            return BubbleEvent(pEvent);
         }
      }
   }

   return false;
}

///////////////////////////////////////

bool cGUIContext::cInputListener::OnInputEvent(const sInputEvent * pEvent)
{
   cGUIContext * pOuter = CTGetOuter(cGUIContext, m_inputListener);
   return pOuter->HandleInputEvent(pEvent);
}

///////////////////////////////////////

void GUIContextCreate()
{
   cAutoIPtr<IGUIContext>(new cGUIContext);
}

///////////////////////////////////////////////////////////////////////////////
