///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guicontext.h"
#include "guievent.h"
#include "guielementtools.h"
#include "guieventroutertem.h"

#include "sceneapi.h"

#include "render.h"

#ifdef _DEBUG
#include "font.h"
#endif

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
#ifdef _DEBUG
   , m_bShowDebugInfo(false)
   , m_debugInfoPlacement(0,0)
   , m_debugInfoTextColor(tGUIColor::White)
   , m_lastMousePos(0,0)
#endif
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

tResult cGUIContext::AddElement(IGUIElement * pElement)
{
   return tBaseClass::AddElement(pElement);
}

///////////////////////////////////////

tResult cGUIContext::RemoveElement(IGUIElement * pElement)
{
   return tBaseClass::RemoveElement(pElement);
}

///////////////////////////////////////

tResult cGUIContext::GetElements(IGUIElementEnum * * ppElements)
{
   return tBaseClass::GetElements(ppElements);
}

///////////////////////////////////////

tResult cGUIContext::HasElement(IGUIElement * pElement) const
{
   return tBaseClass::HasElement(pElement);
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

      cSizeAndPlaceElement sizeAndPlaceFunctor(tGUIRect(0,0,vpWidth,vpHeight));
      ForEachElement(sizeAndPlaceFunctor);
   }

   pRenderDevice->SetRenderState(kRS_EnableDepthBuffer, FALSE);

   sRenderElement renderElementFunctor;
   renderElementFunctor.m_pRenderDevice = CTAddRef(pRenderDevice);
   ForEachElement(renderElementFunctor);

   pRenderDevice->SetRenderState(kRS_EnableDepthBuffer, TRUE);

#ifdef _DEBUG
   RenderDebugInfo(pRenderDevice);
#endif

   return result;
}

///////////////////////////////////////

tResult cGUIContext::ShowDebugInfo(const tGUIPoint & placement, const tGUIColor & textColor)
{
#ifdef _DEBUG
   if (!m_bShowDebugInfo)
   {
      m_bShowDebugInfo = true;
      m_debugInfoPlacement = placement;
      m_debugInfoTextColor = textColor;
      return S_OK;
   }
#endif
   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIContext::HideDebugInfo()
{
#ifdef _DEBUG
   if (m_bShowDebugInfo)
   {
      m_bShowDebugInfo = false;
      return S_OK;
   }
#endif
   return S_FALSE;
}

///////////////////////////////////////

#ifdef _DEBUG
static void DescribeElement(IGUIElement * pElement, char * psz, uint maxLength)
{
   Assert(pElement != NULL);
   cAutoIPtr<IGUIButtonElement> pButton;
   cAutoIPtr<IGUIPanelElement> pPanel;
   cAutoIPtr<IGUILabelElement> pLabel;
   cAutoIPtr<IGUIDialogElement> pDialog;
   const char * pszId = pElement->GetId();
   if (pElement->QueryInterface(IID_IGUIButtonElement, (void**)&pButton) == S_OK)
   {
      snprintf(psz, maxLength, "Button \"%s\"", pButton->GetText());
   }
   else if (pElement->QueryInterface(IID_IGUIPanelElement, (void**)&pPanel) == S_OK)
   {
      snprintf(psz, maxLength, "Panel '%s'", strlen(pszId) > 0 ? pszId : "<no id>");
   }
   else if (pElement->QueryInterface(IID_IGUILabelElement, (void**)&pLabel) == S_OK)
   {
      tGUIString text;
      if (pLabel->GetText(&text) == S_OK)
      {
         snprintf(psz, maxLength, "Label \"%s\"", text.c_str());
      }
      else
      {
         snprintf(psz, maxLength, "Label '%s'", strlen(pszId) > 0 ? pszId : "<no id>");
      }
   }
   else if (pElement->QueryInterface(IID_IGUIDialogElement, (void**)&pDialog) == S_OK)
   {
      tGUIString title;
      if (pDialog->GetTitle(&title) == S_OK)
      {
         snprintf(psz, maxLength, "Dialog \"%s\"", title.c_str());
      }
      else
      {
         snprintf(psz, maxLength, "Dialog '%s'", strlen(pszId) > 0 ? pszId : "<no id>");
      }
   }
   else
   {
      snprintf(psz, maxLength, "Element '%s'", strlen(pszId) > 0 ? pszId : "<no id>");
   }
}
#endif

///////////////////////////////////////

#ifdef _DEBUG
template <typename CTYPE, const int SIZE>
class cTextBuffer
{
public:
   cTextBuffer() : m_length(0)
   {
      m_buffer[0] = 0;
   }

   const CTYPE * GetBuffer() const
   {
      return m_buffer;
   }

   CTYPE * NextPointer()
   {
      m_length = strlen(m_buffer);
      if (m_length > 0)
      {
         strncat(m_buffer, "\n", _countof(m_buffer));
         m_length += 1;
      }
      return m_buffer + m_length;
   }

   uint MaxLength() const
   {
      return SIZE - m_length;
   }

   void NullTerminate()
   {
      m_buffer[SIZE - 1] = 0;
   }

private:
   uint m_length;
   CTYPE m_buffer[SIZE];
};

void cGUIContext::RenderDebugInfo(IRenderDevice * pRenderDevice)
{
   if (!m_bShowDebugInfo)
   {
      return;
   }

   cAutoIPtr<IRenderFont> pFont;
   UseGlobal(GUIRenderingTools);
   if (pGUIRenderingTools->GetDefaultFont(&pFont) == S_OK)
   {
      cTextBuffer<char, 200> text;

      snprintf(text.NextPointer(), text.MaxLength(), "Mouse: (%d, %d)", 
         Round(m_lastMousePos.x), Round(m_lastMousePos.y));

      cAutoIPtr<IGUIElement> pHitElement;
      if (GetHitElement(m_lastMousePos, &pHitElement) == S_OK)
      {
         DescribeElement(pHitElement, text.NextPointer(), text.MaxLength());

         tGUIPoint pos = pHitElement->GetPosition();
         snprintf(text.NextPointer(), text.MaxLength(), "Position: (%d, %d)", 
            Round(pos.x), Round(pos.y));

         tGUIPoint absPos = GUIElementAbsolutePosition(pHitElement);
         snprintf(text.NextPointer(), text.MaxLength(), "Absolute Position: (%d, %d)",
            Round(absPos.x), Round(absPos.y));

         tGUIPoint relPoint(m_lastMousePos - absPos);
         bool bContainsResult = pHitElement->Contains(relPoint);
         snprintf(text.NextPointer(), text.MaxLength(), "Contains() returns %s",
            bContainsResult ? "true" : "false");

         snprintf(text.NextPointer(), text.MaxLength(), "Mouse (relative): (%d, %d)",
            Round(relPoint.x), Round(relPoint.y));

         text.NullTerminate();
      }

      tGUIRect rect(m_debugInfoPlacement.x, m_debugInfoPlacement.y, 0, 0);
      pFont->DrawText(text.GetBuffer(), -1, kDT_NoClip, &rect, m_debugInfoTextColor);
   }
}
#endif

///////////////////////////////////////

#ifdef _DEBUG
bool cGUIContext::HandleInputEvent(const sInputEvent * pEvent)
{
   Assert(pEvent != NULL);

   if (pEvent->key == kMouseMove)
   {
      m_lastMousePos = pEvent->point;
   }

   return cGUIEventRouter<IGUIContext>::HandleInputEvent(pEvent);
}
#endif

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
