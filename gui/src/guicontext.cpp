///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guicontext.h"
#include "guievent.h"
#include "guielementtools.h"
#include "guieventroutertem.h"

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

struct sSizeAndPlaceElement
{
   sSizeAndPlaceElement(uint width, uint height)
     : m_contextField(0,0,width,height) {}

   void operator()(IGUIElement * pGUIElement)
   {
      GUISizeElement(m_contextField, pGUIElement);
      GUIPlaceElement(m_contextField, pGUIElement);
   }

   const tGUIRect m_contextField;
};

///////////////////////////////////////

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

///////////////////////////////////////

tResult cGUIContext::RenderGUI(IRenderDevice * pRenderDevice)
{
   Assert(pRenderDevice != NULL);

   tResult result = S_OK;

   if (m_bNeedLayout)
   {
      m_bNeedLayout = false;

      uint vpWidth, vpHeight;
      Verify(pRenderDevice->GetViewportSize(&vpWidth, &vpHeight) == S_OK);

      sSizeAndPlaceElement sizeAndPlaceFunctor(vpWidth, vpHeight);
      ForEachElement(sizeAndPlaceFunctor);
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
