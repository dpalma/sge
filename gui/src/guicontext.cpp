///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guicontext.h"
#include "guievent.h"
#include "guielementtools.h"
#include "guieventroutertem.h"
#include "scriptvar.h"
#include "sys.h"

#include "keys.h"
#include "resourceapi.h"
#include "readwriteapi.h"

#include <tinyxml.h>
#include <GL/glew.h>

#include <vector>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(GUIContext);

#define LocalMsg(msg)                  DebugMsgEx(GUIContext,(msg))
#define LocalMsg1(msg,a1)              DebugMsgEx1(GUIContext,(msg),(a1))
#define LocalMsg2(msg,a1,a2)           DebugMsgEx2(GUIContext,(msg),(a1),(a2))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(GUIContext,(cond),(msg))
#define LocalMsgIf1(cond,msg,a1)       DebugMsgIfEx1(GUIContext,(cond),(msg),(a1))
#define LocalMsgIf2(cond,msg,a1,a2)    DebugMsgIfEx2(GUIContext,(cond),(msg),(a1),(a2))

///////////////////////////////////////////////////////////////////////////////

static tResult LoadElements(TiXmlDocument * pTiXmlDoc, std::vector<IGUIElement*> * pElements)
{
   if (pTiXmlDoc == NULL || pElements == NULL)
   {
      return E_POINTER;
   }

   UseGlobal(GUIFactory);

   uint nCreated = 0;
   TiXmlElement * pXmlElement;
   for (pXmlElement = pTiXmlDoc->FirstChildElement(); pXmlElement != NULL;
        pXmlElement = pXmlElement->NextSiblingElement())
   {
      if (pXmlElement->Type() == TiXmlNode::ELEMENT)
      {
         cAutoIPtr<IGUIElement> pGuiElement;
         if (pGUIFactory->CreateElement(pXmlElement, &pGuiElement) == S_OK)
         {
            pElements->push_back(CTAddRef(pGuiElement));
            nCreated++;
         }
      }
   }

   return nCreated;
}

///////////////////////////////////////////////////////////////////////////////

static tResult LoadElements(const char * psz, std::vector<IGUIElement*> * pElements)
{
   if (psz == NULL || pElements == NULL)
   {
      return E_POINTER;
   }

   TiXmlDocument doc;
   doc.Parse(psz);

   if (doc.Error())
   {
      ErrorMsg1("TiXml parse error: %s\n", doc.ErrorDesc());
      return E_FAIL;
   }

   uint nCreated = LoadElements(&doc, pElements);
   if (nCreated > 0)
   {
      return S_OK;
   }
   else
   {
      tResKey rk(psz, kRC_TiXml);
      TiXmlDocument * pTiXmlDoc = NULL;
      UseGlobal(ResourceManager);
      if (pResourceManager->Load(rk, (void**)&pTiXmlDoc) == S_OK)
      {
         nCreated = LoadElements(pTiXmlDoc, pElements);
      }
   }

   return (nCreated > 0) ? S_OK : S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

static bool g_bExitModalLoop = false;

static bool GUIModalLoopFrameHandler()
{
   if (g_bExitModalLoop)
   {
      return false;
   }
   return true;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIModalLoopEventListener
//

enum eGUIModalDialogResult
{
   kGUIModalDlgError,
   kGUIModalDlgOK,
   kGUIModalDlgCancel
};

////////////////////////////////////////

class cGUIModalLoopEventListener : public cComObject<IMPLEMENTS(IGUIEventListener)>
{
public:
   cGUIModalLoopEventListener(eGUIModalDialogResult * pResult);
   ~cGUIModalLoopEventListener();

   virtual tResult OnEvent(IGUIEvent * pEvent);

private:
   eGUIModalDialogResult * m_pResult;
};

////////////////////////////////////////

cGUIModalLoopEventListener::cGUIModalLoopEventListener(eGUIModalDialogResult * pResult)
 : m_pResult(pResult)
{
}

////////////////////////////////////////

cGUIModalLoopEventListener::~cGUIModalLoopEventListener()
{
}

////////////////////////////////////////

tResult cGUIModalLoopEventListener::OnEvent(IGUIEvent * pEvent)
{
   bool bEatEvent = false;

   tGUIEventCode eventCode;
   Verify(pEvent->GetEventCode(&eventCode) == S_OK);

   long keyCode;
   Verify(pEvent->GetKeyCode(&keyCode) == S_OK);

   switch (eventCode)
   {
      case kGUIEventKeyUp:
      {
         if (keyCode == kEnter)
         {
            *m_pResult = kGUIModalDlgOK;
            g_bExitModalLoop = true;
            bEatEvent = true;
         }
         break;
      }
      case kGUIEventKeyDown:
      {
         if (keyCode == kEscape)
         {
            *m_pResult = kGUIModalDlgCancel;
            g_bExitModalLoop = true;
            bEatEvent = true;
         }
         break;
      }
      case kGUIEventClick:
      {
         cAutoIPtr<IGUIElement> pSrcElement;
         if (pEvent->GetSourceElement(&pSrcElement) == S_OK)
         {
            cAutoIPtr<IGUIButtonElement> pButtonElement;
            if (pSrcElement->QueryInterface(IID_IGUIButtonElement, (void**)&pButtonElement) == S_OK)
            {
               if (GUIElementIdMatch(pButtonElement, "ok"))
               {
                  *m_pResult = kGUIModalDlgOK;
                  g_bExitModalLoop = true;
                  bEatEvent = true;
               }
               else if (GUIElementIdMatch(pButtonElement, "cancel"))
               {
                  *m_pResult = kGUIModalDlgCancel;
                  g_bExitModalLoop = true;
                  bEatEvent = true;
               }
            }
         }
         break;
      }
   }

   return bEatEvent ? S_FALSE : S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIContext
//

///////////////////////////////////////

BEGIN_CONSTRAINTS(cGUIContext)
   AFTER_GUID(IID_IInput)
   AFTER_GUID(IID_IScriptInterpreter)
END_CONSTRAINTS()

///////////////////////////////////////

cGUIContext::cGUIContext()
 : m_inputListener(this),
   m_nElementsLastLayout(0),
   m_bNeedsLayout(false),
   m_bShowingModalDialog(false)
#ifdef GUI_DEBUG
   , m_bShowDebugInfo(false)
   , m_debugInfoPlacement(0,0)
   , m_debugInfoTextColor(tGUIColor::White)
   , m_lastMousePos(0,0)
#endif
{
   RegisterGlobalObject(IID_IGUIContext, static_cast<IGlobalObject*>(this));
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
   GUILayoutRegisterBuiltInTypes();
   UseGlobal(ScriptInterpreter);
   pScriptInterpreter->AddNamedItem(GetName(), static_cast<IScriptable*>(this));
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

tResult cGUIContext::Invoke(const char * pszMethodName,
                            int argc, const cScriptVar * argv,
                            int nMaxResults, cScriptVar * pResults)
{
   if (pszMethodName == NULL)
   {
      return E_POINTER;
   }

   typedef tResult (cGUIContext::*tInvokeMethod)(int argc, const cScriptVar * argv,
                                                 int nMaxResults, cScriptVar * pResults);

   static const struct
   {
      const tChar * pszMethodName;
      tInvokeMethod pfnMethod;
   }
   invokeMethods[] =
   {
      { "ShowModalDialog",    &cGUIContext::InvokeShowModalDialog },
      { "Clear",              &cGUIContext::InvokeClear },
      { "Load",               &cGUIContext::InvokeLoad },
      { "ToggleDebugInfo",    &cGUIContext::InvokeToggleDebugInfo },
   };

   for (int i = 0; i < _countof(invokeMethods); i++)
   {
      if (strcmp(invokeMethods[i].pszMethodName, pszMethodName) == 0)
      {
         return (this->*invokeMethods[i].pfnMethod)(argc, argv, nMaxResults, pResults);
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIContext::InvokeShowModalDialog(int argc, const cScriptVar * argv,
                                           int nMaxResults, cScriptVar * pResults)
{
   Assert(nMaxResults >= 1);

   if (argc != 1 || !argv[0].IsString())
   {
      return E_INVALIDARG;
   }

   tResult result = ShowModalDialog(argv[0]);
   if (result == S_OK)
   {
      *pResults = cScriptVar(true);
      result = 1; // # of return values
   }
   else if (result == S_FALSE)
   {
      *pResults = cScriptVar::Nil;
      result = 1; // # of return values
   }
   return result;
}

///////////////////////////////////////

tResult cGUIContext::InvokeClear(int argc, const cScriptVar * argv,
                                 int nMaxResults, cScriptVar * pResults)
{
   if (argc != 0)
   {
      return E_INVALIDARG;
   }
   ClearGUI();
   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::InvokeLoad(int argc, const cScriptVar * argv,
                                int nMaxResults, cScriptVar * pResults)
{
   if (argc == 1 && argv[0].IsString())
   {
      if (LoadElements(argv[0], true) == S_OK)
      {
         LocalMsg1("Loading GUI definitions from %s\n", argv[0].psz);
      }
   }
   else if (argc == 2 && argv[0].IsString() && argv[1].IsNumber())
   {
      bool bVisible = (argv[1].ToInt() != 0);
      if (LoadElements(argv[0], bVisible) == S_OK)
      {
         LocalMsg1("Loading GUI definitions from %s\n", argv[0].psz);
      }
   }
   else
   {
      return E_INVALIDARG;
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::InvokeToggleDebugInfo(int argc, const cScriptVar * argv,
                                           int nMaxResults, cScriptVar * pResults)
{
   tGUIPoint placement(0,0);
   tGUIColor color(tGUIColor::White);

   if (argc == 2 
      && argv[0].IsNumber() 
      && argv[1].IsNumber())
   {
      placement = tGUIPoint(argv[0], argv[1]);
   }
   else if (argc == 3 
      && argv[0].IsNumber() 
      && argv[1].IsNumber()
      && argv[2].IsString())
   {
      placement = tGUIPoint(argv[0], argv[1]);
      GUIStyleParseColor(argv[2], &color);
   }
   else if (argc == 5 
      && argv[0].IsNumber() 
      && argv[1].IsNumber()
      && argv[2].IsNumber() 
      && argv[3].IsNumber()
      && argv[4].IsNumber())
   {
      placement = tGUIPoint(argv[0], argv[1]);
      color = tGUIColor(argv[2], argv[3], argv[4]);
   }

   if (ShowDebugInfo(placement, color) == S_FALSE)
   {
      HideDebugInfo();
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::ShowModalDialog(const tChar * pszDialog)
{
   if (pszDialog == NULL)
   {
      return E_POINTER;
   }

   // Only one at a time supported now
   if (m_bShowingModalDialog)
   {
      WarnMsg("Attempt to show multiple confirm dialogs\n");
      return E_FAIL;
   }

   tResult result = E_FAIL;

   std::vector<IGUIElement*> elements;
   if (::LoadElements(pszDialog, &elements) == S_OK && elements.size() == 1
      && SUCCEEDED(CheckModalDialog(elements.front())))
   {
      m_bShowingModalDialog = true;

      AddElement(elements.front());

      eGUIModalDialogResult modalDialogResult = kGUIModalDlgError;
      cGUIModalLoopEventListener listener(&modalDialogResult);
      AddEventListener(&listener);

      g_bExitModalLoop = false;

      // This function won't return until the modal loop is ended by
      // some user action (Enter, Esc, OK button click, etc.)
      SysEventLoop(GUIModalLoopFrameHandler, NULL);

      if (modalDialogResult == kGUIModalDlgOK)
      {
         result = S_OK;
      }
      else if (modalDialogResult == kGUIModalDlgCancel)
      {
         result = S_FALSE;
      }

      RemoveEventListener(&listener);

      RemoveElement(elements.front());

      m_bShowingModalDialog = false;
   }

   std::for_each(elements.begin(), elements.end(), CTInterfaceMethod(&IGUIElement::Release));

   return result;
}

///////////////////////////////////////

tResult cGUIContext::LoadElements(const char * pszXmlStringOrFile, bool bVisible)
{
   if (pszXmlStringOrFile == NULL)
   {
      return E_POINTER;
   }

   std::vector<IGUIElement*> elements;
   if (::LoadElements(pszXmlStringOrFile, &elements) == S_OK)
   {
      std::vector<IGUIElement*>::iterator iter = elements.begin();
      std::vector<IGUIElement*>::iterator end = elements.end();
      for (; iter != end; iter++)
      {
         (*iter)->SetVisible(bVisible);
         AddElement(*iter);
         (*iter)->Release();
      }

      elements.clear();

      return S_OK;
   }

   return S_FALSE;
}

///////////////////////////////////////

void cGUIContext::ClearGUI()
{
   RemoveAllElements();
   m_bNeedsLayout = true;
}

///////////////////////////////////////

class cRenderElement
{
   cRenderElement(const cRenderElement &);
   void operator =(const cRenderElement &);

public:
   cRenderElement(void * pReserved);
   ~cRenderElement();

   tResult operator()(IGUIElement * pGUIElement);

private:
   cAutoIPtr<IGUIRenderDevice> m_pRenderDevice;
};

cRenderElement::cRenderElement(void * pReserved)
{
}

cRenderElement::~cRenderElement()
{
}

tResult cRenderElement::operator()(IGUIElement * pElement)
{
   if (pElement == NULL)
   {
      return E_POINTER;
   }

   if (!pElement->IsVisible())
   {
      return S_FALSE;
   }

   cAutoIPtr<IGUIElementRenderer> pRenderer;
   if (pElement->GetRenderer(&pRenderer) == S_OK)
   {
      if (pRenderer->Render(pElement, m_pRenderDevice) == S_OK)
      {
         return S_OK;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIContext::RenderGUI()
{
   // Wait as long as possible to create the GUI rendering device to
   // ensure there is a GL context.
   if (!m_pRenderDevice)
   {
      if (FAILED(GUIRenderDeviceCreateGL(&m_pRenderDevice)))
      {
         return E_FAIL;
      }
   }

   uint nElements = GetElementCount();
   if ((nElements != m_nElementsLastLayout) || m_bNeedsLayout)
   {
      m_nElementsLastLayout = nElements;
      m_bNeedsLayout = false;

      int viewport[4];
      glGetIntegerv(GL_VIEWPORT, viewport);

      ForEachElement(cSizeAndPlaceElement(tGUIRect(0,0,viewport[2],viewport[3])));
   }

   glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
   glDisable(GL_DEPTH_TEST);

   tGUIElementList::iterator iter = BeginElements();
   tGUIElementList::iterator end = EndElements();
   for (; iter != end; ++iter)
   {
      if ((*iter)->IsVisible())
      {
         cAutoIPtr<IGUIElementRenderer> pRenderer;
         if ((*iter)->GetRenderer(&pRenderer) == S_OK)
         {
            if (pRenderer->Render(*iter, m_pRenderDevice) != S_OK)
            {
               ErrorMsg("A GUI element failed to render properly\n");
            }
         }
      }
   }

#ifdef GUI_DEBUG
   RenderDebugInfo();
#endif

   m_pRenderDevice->FlushQueue();

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

tResult cGUIContext::CheckModalDialog(IGUIElement * pElement)
{
   if (pElement == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IGUIDialogElement> pDialog;
   if (pElement->QueryInterface(IID_IGUIDialogElement, (void**)&pDialog) != S_OK)
   {
      return E_INVALIDARG;
   }

   // TODO: this won't find buttons inside nested containers, like a panel inside a dialog
   if (CheckChild(pDialog, "ok", IID_IGUIButtonElement) != S_OK
      && CheckChild(pDialog, "cancel", IID_IGUIButtonElement) != S_OK)
   {
      WarnMsg("Dialog box has no \"ok\" nor \"cancel\" button\n");
      return S_FALSE;
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::CheckChild(IGUIContainerElement * pContainer, const tChar * pszId, REFGUID iid)
{
   if (pContainer == NULL || pszId == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IGUIElement> pElement;
   if (pContainer->GetElement(pszId, &pElement) != S_OK)
   {
      return E_FAIL;
   }

   cAutoIPtr<IUnknown> pUnknown;
   if (pElement->QueryInterface(iid, (void**)&pUnknown) != S_OK)
   {
      return E_FAIL;
   }

   return S_OK;
}

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
