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

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cGUIEventRouter
//

///////////////////////////////////////

template <typename INTRFC>
cGUIEventRouter<INTRFC>::cGUIEventRouter()
{
}

///////////////////////////////////////

template <typename INTRFC>
cGUIEventRouter<INTRFC>::~cGUIEventRouter()
{
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::AddEventListener(IGUIEventListener * pListener)
{
   return Connect(pListener);
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::RemoveEventListener(IGUIEventListener * pListener)
{
   return Disconnect(pListener);
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::GetFocus(IGUIElement * * ppElement)
{
   return m_pFocus.GetPointer(ppElement);
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::SetFocus(IGUIElement * pElement)
{
   SafeRelease(m_pFocus);
   m_pFocus = CTAddRef(pElement);
   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::GetCapture(IGUIElement * * ppElement)
{
   return m_pCapture.GetPointer(ppElement);
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::SetCapture(IGUIElement * pElement)
{
   SafeRelease(m_pCapture);
   m_pCapture = CTAddRef(pElement);
   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::GetMouseOver(IGUIElement * * ppElement)
{
   return m_pMouseOver.GetPointer(ppElement);
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::SetMouseOver(IGUIElement * pElement)
{
   SafeRelease(m_pMouseOver);
   m_pMouseOver = CTAddRef(pElement);
   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::AddElement(IGUIElement * pElement)
{
   if (pElement == NULL)
   {
      return E_POINTER;
   }
   m_elements.push_back(CTAddRef(pElement));
   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIEventRouter<INTRFC>::RemoveAllElements()
{
   SafeRelease(m_pFocus);
   SafeRelease(m_pCapture);
   SafeRelease(m_pMouseOver);
   std::for_each(m_elements.begin(), m_elements.end(), CTInterfaceMethodRef(&IGUIElement::Release));
   m_elements.clear();
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::GetHitElement(const tGUIPoint & point, IGUIElement * * ppElement) const
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   tGUIElementList::const_iterator iter;
   for (iter = m_elements.begin(); iter != m_elements.end(); iter++)
   {
      if ((*iter)->Contains(point))
      {
         *ppElement = CTAddRef(*iter);
         return S_OK;
      }
   }

   return S_FALSE;
}

///////////////////////////////////////

template <typename INTRFC>
bool cGUIEventRouter<INTRFC>::BubbleEvent(IGUIEvent * pEvent)
{
   Assert(pEvent != NULL);

   tSinks::iterator iter;
   for (iter = AccessSinks().begin(); iter != AccessSinks().end(); iter++)
   {
      if ((*iter)->OnEvent(pEvent) != S_OK)
      {
         return true;
      }
   }

   cAutoIPtr<IGUIElement> pDispatchTo;
   if (pEvent->GetSourceElement(&pDispatchTo) == S_OK)
   {
      while (!!pDispatchTo)
      {
         if (pDispatchTo->OnEvent(pEvent))
         {
            return true;
         }

         cAutoIPtr<IGUIElement> pNext;
         if (pDispatchTo->GetParent(&pNext) != S_OK)
         {
            SafeRelease(pDispatchTo);
         }
         else
         {
            pDispatchTo = pNext;
         }
      }
   }

   return false;
}

///////////////////////////////////////

template <typename INTRFC>
bool cGUIEventRouter<INTRFC>::GetEventTarget(const sInputEvent * pInputEvent, 
                                             IGUIElement * * ppElement)
{
   Assert(pInputEvent != NULL);
   Assert(ppElement != NULL);

   if (GetCapture(ppElement) == S_OK)
   {
      return true;
   }
   else if (KeyIsMouse(pInputEvent->key))
   {
      return GetHitElement(pInputEvent->point, ppElement) == S_OK;
   }
   else if (GetFocus(ppElement) == S_OK)
   {
      return true;
   }

   return false;
}

///////////////////////////////////////

template <typename INTRFC>
bool cGUIEventRouter<INTRFC>::HandleInputEvent(const sInputEvent * pInputEvent)
{
   tGUIEventCode eventCode = GUIEventCode(pInputEvent->key, pInputEvent->down);
   if (eventCode == kGUIEventNone)
   {
      DebugMsg("WARNING: Invalid event code\n");
      return false;
   }

   if (eventCode == kGUIEventMouseMove)
   {
      cAutoIPtr<IGUIElement> pMouseOver;
      if ((GetMouseOver(&pMouseOver) == S_OK) && !pMouseOver->Contains(pInputEvent->point))
      {
         SetMouseOver(NULL);
         cAutoIPtr<IGUIEvent> pMouseLeaveEvent;
         if (GUIEventCreate(kGUIEventMouseLeave, pInputEvent->point, pInputEvent->key, 
            pMouseOver, &pMouseLeaveEvent) == S_OK)
         {
            BubbleEvent(pMouseLeaveEvent);
         }
      }

      cAutoIPtr<IGUIElement> pHit;
      if ((GetHitElement(pInputEvent->point, &pHit) == S_OK)
         && (AccessMouseOver() == NULL))
      {
         SetMouseOver(pHit);
         cAutoIPtr<IGUIEvent> pMouseEnterEvent;
         if (GUIEventCreate(kGUIEventMouseEnter, pInputEvent->point, pInputEvent->key, 
            pHit, &pMouseEnterEvent) == S_OK)
         {
            BubbleEvent(pMouseEnterEvent);
         }
      }
   }

   cAutoIPtr<IGUIElement> pElement;
   if (GetEventTarget(pInputEvent, &pElement))
   {
      if (eventCode == kGUIEventMouseDown)
      {
         SetFocus(pElement);
      }
      else if (eventCode == kGUIEventMouseUp)
      {
         if (pElement->Contains(pInputEvent->point))
         {
            // TODO: Doing this here, the click event will occur before the mouse up event
            // Not sure if that is the right thing
            cAutoIPtr<IGUIEvent> pClickEvent;
            if (GUIEventCreate(kGUIEventClick, pInputEvent->point, pInputEvent->key, 
               pElement, &pClickEvent) == S_OK)
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

   return false;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIContext
//

BEGIN_CONSTRAINTS()
   AFTER_GUID(IID_IScene)
END_CONSTRAINTS()

///////////////////////////////////////

cGUIContext::cGUIContext()
 : tBaseClass("GUIContext", CONSTRAINTS()),
   m_bNeedLayout(false)
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
   RemoveAllElements();
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
            AddElement(pGUIElement);
         }
      }
   }

   m_bNeedLayout = (nElementsCreated > 0);

   return nElementsCreated;
}

///////////////////////////////////////

// TODO HACK: this is boneheaded simple and temporary
struct sNaivelySetPreferredSize
{
   void operator()(IGUIElement * pGUIElement)
   {
      cAutoIPtr<IGUIElementRenderer> pRenderer;
      if (pGUIElement->GetRenderer(&pRenderer) == S_OK)
      {
         pGUIElement->SetSize(pRenderer->GetPreferredSize(pGUIElement));
      }
   }
};

struct sRenderElement
{
   sRenderElement() : m_bError(false) {}

   void operator()(IGUIElement * pGUIElement)
   {
      if (pGUIElement->IsVisible())
      {
         cAutoIPtr<IGUIElementRenderer> pRenderer;
         if (pGUIElement->GetRenderer(&pRenderer) == S_OK)
         {
            if (pRenderer->Render(pGUIElement, m_pRenderDevice) != S_OK)
            {
               DebugMsg("WARNING: Error during GUI rendering\n");
               m_bError = true;
            }
         }
      }
   }

   cAutoIPtr<IRenderDevice> m_pRenderDevice;
   bool m_bError;
};

tResult cGUIContext::RenderGUI(IRenderDevice * pRenderDevice)
{
   Assert(pRenderDevice != NULL);

   tResult result = S_OK;

   uint vpWidth, vpHeight;
   Verify(pRenderDevice->GetViewportSize(&vpWidth, &vpHeight) == S_OK);

   if (m_bNeedLayout)
   {
      m_bNeedLayout = false;

      ForEachElement(sNaivelySetPreferredSize());
   }

   pRenderDevice->SetRenderState(kRS_EnableDepthBuffer, FALSE);

   sRenderElement renderElementFunctor;
   renderElementFunctor.m_pRenderDevice = CTAddRef(pRenderDevice);
   ForEachElement(renderElementFunctor);

   pRenderDevice->SetRenderState(kRS_EnableDepthBuffer, TRUE);

   return result;
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

#ifdef HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
//
// CLASS cGUITestableEventRouter
//

class cGUITestableEventRouter : public cComObject<cGUIEventRouter<IGUIEventRouter>, &IID_IGUIEventRouter>
{
public:
   cGUITestableEventRouter();

   void FireInputEvent(long key, bool down, tVec2 point);

private:
   ulong m_time;
};

///////////////////////////////////////

cGUITestableEventRouter::cGUITestableEventRouter()
 : m_time(0)
{
}

///////////////////////////////////////

void cGUITestableEventRouter::FireInputEvent(long key, bool down, tVec2 point)
{
   sInputEvent event;
   event.key = key;
   event.down = down;
   event.point = point;
   event.time = m_time++;
   HandleInputEvent(&event);
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS cGUITestEventListener
//

class cGUITestEventListener : public cComObject<IMPLEMENTS(IGUIEventListener)>
{
public:
   virtual tResult OnEvent(IGUIEvent * pEvent);
};

///////////////////////////////////////

tResult cGUITestEventListener::OnEvent(IGUIEvent * pEvent)
{
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

class cGUIEventRouterTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cGUIEventRouterTests);
   CPPUNIT_TEST_SUITE_END();

private:
   virtual void setUp();
   virtual void tearDown();

   cAutoIPtr<cGUITestableEventRouter> m_pEventRouter;
   cAutoIPtr<cGUITestEventListener> m_pEventListener;
};

///////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cGUIEventRouterTests);

///////////////////////////////////////

void cGUIEventRouterTests::setUp()
{
   CPPUNIT_ASSERT(!m_pEventRouter);
   m_pEventRouter = new cGUITestableEventRouter;

   CPPUNIT_ASSERT(!m_pEventListener);
   m_pEventListener = new cGUITestEventListener;
   m_pEventRouter->AddEventListener(m_pEventListener);
}

///////////////////////////////////////

void cGUIEventRouterTests::tearDown()
{
   if (m_pEventRouter && m_pEventListener)
   {
      m_pEventRouter->RemoveEventListener(m_pEventListener);
   }

   SafeRelease(m_pEventRouter);
   SafeRelease(m_pEventListener);
}

///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
