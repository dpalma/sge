///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guicontext.h"
#include "guievent.h"
#include "guielementtools.h"
#include "guieventroutertem.h"

#include "keys.h"
#include "resourceapi.h"
#include "readwriteapi.h"

#include <tinyxml.h>
#include <GL/glew.h>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIContext
//

///////////////////////////////////////

BEGIN_CONSTRAINTS()
   AFTER_GUID(IID_IInput)
END_CONSTRAINTS()

///////////////////////////////////////

cGUIContext::cGUIContext()
 : tBaseClass("GUIContext", CONSTRAINTS()),
   m_inputListener(this),
   m_bNeedLayout(false)
#ifdef GUI_DEBUG
   , m_bShowDebugInfo(false)
   , m_debugInfoPlacement(0,0)
   , m_debugInfoTextColor(tGUIColor::White)
   , m_lastMousePos(0,0)
#endif
{
}

///////////////////////////////////////

cGUIContext::~cGUIContext()
{
}

///////////////////////////////////////

tResult cGUIContext::Init()
{
   UseGlobal(Input);
   pInput->SetGUIInputListener(&m_inputListener);
   GUILayoutManagerRegisterBuiltInTypes();
   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::Term()
{
   UseGlobal(Input);
   pInput->SetGUIInputListener(NULL);
   RemoveAllElements();
   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::AddElement(IGUIElement * pElement)
{
#ifndef NDEBUG
   {
      // Check that all sink interface pointers are valid
      tSinksIterator iter = BeginSinks();
      tSinksIterator end = EndSinks();
      for (; iter != end; iter++)
      {
         (*iter)->AddRef();
         (*iter)->Release();
      }
   }
#endif
   return tBaseClass::AddElement(pElement);
}

///////////////////////////////////////

tResult cGUIContext::RemoveElement(IGUIElement * pElement)
{
   tResult result = tBaseClass::RemoveElement(pElement);
#ifndef NDEBUG
   {
      // Check that all sink interface pointers are valid
      tSinksIterator iter = BeginSinks();
      tSinksIterator end = EndSinks();
      for (; iter != end; iter++)
      {
         (*iter)->AddRef();
         (*iter)->Release();
      }
   }
#endif
   return result;
}

///////////////////////////////////////

tResult cGUIContext::LoadFromResource(const char * psz, bool bVisible)
{
   tResKey rk(psz, kRC_TiXml);
   TiXmlDocument * pTiXmlDoc = NULL;
   UseGlobal(ResourceManager);
   if (pResourceManager->Load(rk, (void**)&pTiXmlDoc) == S_OK)
   {
      uint nCreated = LoadFromTiXmlDoc(pTiXmlDoc, bVisible);
      pResourceManager->Unload(rk);
      return (nCreated > 0) ? S_OK : S_FALSE;
   }
   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIContext::LoadFromString(const char * psz, bool bVisible)
{
   TiXmlDocument doc;
   doc.Parse(psz);

   if (doc.Error())
   {
      DebugMsg1("TiXml parse error: %s\n", doc.ErrorDesc());
      return E_FAIL;
   }

   uint nCreated = LoadFromTiXmlDoc(&doc, bVisible);
   return (nCreated > 0) ? S_OK : S_FALSE;
}

///////////////////////////////////////

uint cGUIContext::LoadFromTiXmlDoc(TiXmlDocument * pTiXmlDoc, bool bVisible)
{
   Assert(pTiXmlDoc != NULL);

   UseGlobal(GUIFactory);

   uint nElementsCreated = 0;
   TiXmlElement * pXmlElement;
   for (pXmlElement = pTiXmlDoc->FirstChildElement(); pXmlElement != NULL;
        pXmlElement = pXmlElement->NextSiblingElement())
   {
      if (pXmlElement->Type() == TiXmlNode::ELEMENT)
      {
         cAutoIPtr<IGUIElement> pGUIElement;
         if (pGUIFactory->CreateElement(pXmlElement, &pGUIElement) == S_OK)
         {
            pGUIElement->SetVisible(bVisible);
            if (AddElement(pGUIElement) == S_OK)
            {
               nElementsCreated++;
            }
         }
      }
   }

   m_bNeedLayout = (nElementsCreated > 0);

   return nElementsCreated;
}

///////////////////////////////////////

void cGUIContext::ClearGUI()
{
   RemoveAllElements();
   m_bNeedLayout = true;
}

///////////////////////////////////////

class cRenderElement
{
public:
   cRenderElement();
   tResult operator()(IGUIElement * pGUIElement);
};

cRenderElement::cRenderElement()
{
}

tResult cRenderElement::operator()(IGUIElement * pGUIElement)
{
   if (pGUIElement == NULL)
   {
      return E_POINTER;
   }

   if (!pGUIElement->IsVisible())
   {
      return S_FALSE;
   }

   cAutoIPtr<IGUIElementRenderer> pRenderer;
   if (pGUIElement->GetRenderer(&pRenderer) == S_OK)
   {
      if (pRenderer->Render(pGUIElement) == S_OK)
      {
         return S_OK;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIContext::RenderGUI()
{
   if (m_bNeedLayout)
   {
      m_bNeedLayout = false;

      int viewport[4];
      glGetIntegerv(GL_VIEWPORT, viewport);

      ForEachElement(cSizeAndPlaceElement(tGUIRect(0,0,viewport[2],viewport[3])));
   }

   glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
   glDisable(GL_DEPTH_TEST);
   ForEachElement(cRenderElement());
#ifdef GUI_DEBUG
   RenderDebugInfo();
#endif
   glPopAttrib();

   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::ShowDebugInfo(const tGUIPoint & placement, const tGUIColor & textColor)
{
#ifdef GUI_DEBUG
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
#ifdef GUI_DEBUG
   if (m_bShowDebugInfo)
   {
      m_bShowDebugInfo = false;
      return S_OK;
   }
#endif
   return S_FALSE;
}

///////////////////////////////////////

#ifdef GUI_DEBUG
static bool GUIElementType(IUnknown * pUnkElement, cStr * pType)
{
   static const struct
   {
      const GUID * pIID;
      const tChar * pszType;
   }
   guiElementTypes[] =
   {
      { &IID_IGUIButtonElement,     "Button" },
      { &IID_IGUIDialogElement,     "Dialog" },
      { &IID_IGUILabelElement,      "Label" },
      { &IID_IGUIPanelElement,      "Panel" },
      { &IID_IGUITextEditElement,   "TextEdit" },
      { &IID_IGUIContainerElement,  "Container" },
   };
   for (int i = 0; i < _countof(guiElementTypes); i++)
   {
      cAutoIPtr<IUnknown> pUnk;
      if (pUnkElement->QueryInterface(*guiElementTypes[i].pIID, (void**)&pUnk) == S_OK)
      {
         *pType = guiElementTypes[i].pszType;
         return true;
      }
   }
   return false;
}
#endif

///////////////////////////////////////

#ifdef GUI_DEBUG
void cGUIContext::RenderDebugInfo()
{
   if (!m_bShowDebugInfo)
   {
      return;
   }

   cAutoIPtr<IGUIFont> pFont;
   if (GUIFontGetDefault(&pFont) == S_OK)
   {
      tGUIRect rect(0,0,0,0);
      pFont->RenderText("Xy", -1, &rect, kRT_CalcRect, m_debugInfoTextColor);

      const int lineHeight = rect.GetHeight();

      rect = tGUIRect(Round(m_debugInfoPlacement.x), Round(m_debugInfoPlacement.y), 0, 0);

      cStr temp;
      temp.Format("Mouse: (%d, %d)", Round(m_lastMousePos.x), Round(m_lastMousePos.y));
      pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
      rect.Offset(0, lineHeight);

      std::list<IGUIElement*> hitElements;
      if (GetHitElements(m_lastMousePos, &hitElements) == S_OK)
      {
         std::list<IGUIElement*>::const_iterator iter = hitElements.begin();
         std::list<IGUIElement*>::const_iterator end = hitElements.end();
         for (int index = 0; iter != end; ++iter, ++index)
         {
            if (GUIElementType(*iter, &temp))
            {
               cStr type, temp;
               if (GUIElementType(*iter, &type))
               {
                  temp.Format("Element %d: %s", index, type.c_str());
               }
               else
               {
                  temp.Format("Element %d", index);
               }
               pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
               rect.Offset(0, lineHeight);
            }

//            if (index == 0)
            {
               rect.left += lineHeight;

               tGUISize size((*iter)->GetSize());
               temp.Format("Size: %d x %d", Round(size.width), Round(size.height));
               pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
               rect.Offset(0, lineHeight);

               tGUIPoint pos((*iter)->GetPosition());
               temp.Format("Position: (%d, %d)", Round(pos.x), Round(pos.y));
               pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
               rect.Offset(0, lineHeight);

               uint nParents = 0;
               tGUIPoint absPos(GUIElementAbsolutePosition(*iter, &nParents));
               temp.Format("Absolute Position: (%d, %d)", Round(absPos.x), Round(absPos.y));
               pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
               rect.Offset(0, lineHeight);

               temp.Format("# Parents: %d", nParents);
               pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
               rect.Offset(0, lineHeight);

               tGUIPoint relPoint(m_lastMousePos - absPos);
               Assert((*iter)->Contains(relPoint));
               temp.Format("Mouse (relative): (%d, %d)", Round(relPoint.x), Round(relPoint.y));
               pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
               rect.Offset(0, lineHeight);

               rect.left -= lineHeight;
            }
         }

         std::for_each(hitElements.begin(), hitElements.end(), CTInterfaceMethod(&IGUIElement::Release));
      }
   }
}
#endif

///////////////////////////////////////

#ifdef GUI_DEBUG
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

cGUIContext::cInputListener::cInputListener(cGUIContext * pOuter)
 : m_pOuter(pOuter)
{
}

///////////////////////////////////////

bool cGUIContext::cInputListener::OnInputEvent(const sInputEvent * pEvent)
{
   Assert(m_pOuter != NULL);
   return m_pOuter->HandleInputEvent(pEvent);
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
