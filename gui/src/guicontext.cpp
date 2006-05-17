///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guicontext.h"
#include "guievent.h"
#include "guielementenum.h"
#include "guipage.h"
#include "guistyleapi.h"

#include "sys.h"
#include "engineapi.h"

#include "keys.h"
#include "multivar.h"
#include "resourceapi.h"

#include "guieventroutertem.h"

#include <tinyxml.h>

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

static bool g_bExitModalLoop = false;
tSysFrameFn g_pfnOuterFrameHandler = NULL;

static tResult GUIModalLoopFrameHandler()
{
   if (g_bExitModalLoop)
   {
      g_pfnOuterFrameHandler = NULL;
      return S_FALSE;
   }
   return (g_pfnOuterFrameHandler != NULL) ? (*g_pfnOuterFrameHandler)() : S_OK;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIModalLoopEventListener
//

////////////////////////////////////////

class cGUIModalLoopEventListener : public cComObject<IMPLEMENTS(IGUIEventListener)>
{
public:
   cGUIModalLoopEventListener(tResult * pResult);
   ~cGUIModalLoopEventListener();

   virtual tResult OnEvent(IGUIEvent * pEvent);

private:
   tResult * m_pResult;
};

////////////////////////////////////////

cGUIModalLoopEventListener::cGUIModalLoopEventListener(tResult * pResult)
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
            *m_pResult = S_OK;
            g_bExitModalLoop = true;
            bEatEvent = true;
         }
         break;
      }
      case kGUIEventKeyDown:
      {
         if (keyCode == kEscape)
         {
            *m_pResult = S_FALSE;
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
                  *m_pResult = S_OK;
                  g_bExitModalLoop = true;
                  bEatEvent = true;
               }
               else if (GUIElementIdMatch(pButtonElement, "cancel"))
               {
                  *m_pResult = S_FALSE;
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

static tResult LoadPage(const tChar * pszPage, cGUIPage * * ppPage)
{
   if (pszPage == NULL || ppPage == NULL)
   {
      return E_POINTER;
   }

   TiXmlBase::SetCondenseWhiteSpace(false);

   TiXmlDocument doc;
   TiXmlDocument * pTiXmlDoc = &doc;

   doc.Parse(pszPage);
   int errorId = doc.ErrorId();

   if (errorId != TiXmlBase::TIXML_NO_ERROR && errorId != TiXmlBase::TIXML_ERROR_DOCUMENT_EMPTY)
   {
      ErrorMsg1("TiXml parse error: %s\n", doc.ErrorDesc());
      return E_FAIL;
   }
   else if (errorId == TiXmlBase::TIXML_ERROR_DOCUMENT_EMPTY)
   {
      UseGlobal(ResourceManager);
      if (pResourceManager->Load(pszPage, kRT_TiXml, NULL, (void**)&pTiXmlDoc) != S_OK)
      {
         ErrorMsg("Error loading TiXml document as a resource\n");
         return E_FAIL;
      }
   }

   return cGUIPage::Create(pTiXmlDoc, ppPage);
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

cGUIContext::cGUIContext(const tChar * pszScriptName)
 : m_inputListener(this)
 , m_bShowingModalDialog(false)
 , m_scriptName((pszScriptName != NULL) ? pszScriptName : _T(""))
#ifdef GUI_DEBUG
 , m_bShowDebugInfo(false)
 , m_debugInfoPlacement(0,0)
 , m_debugInfoTextColor(GUIStandardColors::White)
 , m_lastMousePos(0,0)
#endif
{
}

///////////////////////////////////////

cGUIContext::~cGUIContext()
{
   for (int i = 0; i < _countof(m_pagePlanes); i++)
   {
      Assert(m_pagePlanes[i].empty());
   }
}

///////////////////////////////////////

tResult cGUIContext::Init()
{
   UseGlobal(Input);
   pInput->AddInputListener(&m_inputListener, kILP_GUI);

   GUILayoutRegisterBuiltInTypes();

   UseGlobal(ScriptInterpreter);
   pScriptInterpreter->AddNamedItem(m_scriptName.empty() ? GetName() : m_scriptName.c_str(),
      static_cast<IScriptable*>(this));

   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::Term()
{
   UseGlobal(Input);
   pInput->RemoveInputListener(&m_inputListener);

   for (int i = 0; i < _countof(m_pagePlanes); i++)
   {
      tGUIPageList::iterator iter = m_pagePlanes[i].begin();
      for (; iter != m_pagePlanes[i].end(); iter++)
      {
         delete *iter;
      }
      m_pagePlanes[i].clear();
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::Invoke(const char * pszMethodName,
                            int argc, const tScriptVar * argv,
                            int nMaxResults, tScriptVar * pResults)
{
   if (pszMethodName == NULL)
   {
      return E_POINTER;
   }

   typedef tResult (cGUIContext::*tInvokeMethod)(int argc, const tScriptVar * argv,
                                                 int nMaxResults, tScriptVar * pResults);

   static const struct
   {
      const tChar * pszMethodName;
      tInvokeMethod pfnMethod;
   }
   invokeMethods[] =
   {
      { "ShowModalDialog",    &cGUIContext::InvokeShowModalDialog },
      { "PushPage",           &cGUIContext::InvokePushPage },
      { "PopPage",            &cGUIContext::InvokePopPage },
      { "ToggleDebugInfo",    &cGUIContext::InvokeToggleDebugInfo },
      { "GetElement",         &cGUIContext::InvokeGetElement },
      { "AddOverlay",         &cGUIContext::InvokeAddOverlay },
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

tResult cGUIContext::InvokeShowModalDialog(int argc, const tScriptVar * argv,
                                           int nMaxResults, tScriptVar * pResults)
{
   Assert(nMaxResults >= 1);

   if (argc != 1 || !argv[0].IsString())
   {
      return E_INVALIDARG;
   }

   tResult result = ShowModalDialog(argv[0]);
   if (result == S_OK)
   {
      *pResults = tScriptVar(true);
      result = 1; // # of return values
   }
   else if (result == S_FALSE)
   {
      *pResults = tScriptVar();
      result = 1; // # of return values
   }
   return result;
}

///////////////////////////////////////

tResult cGUIContext::InvokePushPage(int argc, const tScriptVar * argv,
                                    int nMaxResults, tScriptVar * pResults)
{
   if (argc == 1 && argv[0].IsString())
   {
      if (PushPage(argv[0]) == S_OK)
      {
         LocalMsg1("Loading GUI definitions from %s\n", argv[0].ToString());
         return S_OK;
      }
   }
   else
   {
      return E_INVALIDARG;
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIContext::InvokePopPage(int argc, const tScriptVar * argv,
                                   int nMaxResults, tScriptVar * pResults)
{
   if (argc != 0)
   {
      return E_INVALIDARG;
   }

   if (PopPage() == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIContext::InvokeToggleDebugInfo(int argc, const tScriptVar * argv,
                                           int nMaxResults, tScriptVar * pResults)
{
   tGUIPoint placement(0,0);
   cAutoIPtr<IGUIStyle> pStyle;

   if (argc == 2 
      && IsNumber(argv[0]) 
      && IsNumber(argv[1]))
   {
      placement = tGUIPoint(argv[0], argv[1]);
   }
   else if (argc == 3 
      && IsNumber(argv[0]) 
      && IsNumber(argv[1])
      && argv[2].IsString())
   {
      placement = tGUIPoint(argv[0], argv[1]);
      if (GUIStyleParse(argv[2], -1, &pStyle) != S_OK)
      {
         SafeRelease(pStyle);
      }
   }
   else
   {
      return E_FAIL;
   }

   if (ShowDebugInfo(placement, pStyle) == S_FALSE)
   {
      HideDebugInfo();
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::InvokeGetElement(int argc, const tScriptVar * argv,
                                      int nMaxResults, tScriptVar * pResults)
{
   if (argc != 1 || !argv[0].IsString())
   {
      return E_INVALIDARG;
   }

   cAutoIPtr<IGUIElement> pElement;
   if (GetElementById(argv[0], &pElement) == S_OK)
   {
      cAutoIPtr<IScriptable> pScriptable;
      if (pElement->QueryInterface(IID_IScriptable, (void**)&pScriptable) == S_OK)
      {
         pResults[0] = pScriptable;
         return 1;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIContext::InvokeAddOverlay(int argc, const tScriptVar * argv,
                                      int nMaxResults, tScriptVar * pResults)
{
   if (argc == 1 && argv[0].IsString())
   {
      if (AddOverlayPage(argv[0]) == S_OK)
      {
         LocalMsg1("Adding overlay page \"%s\"\n", argv[0].ToString());
         return S_OK;
      }
   }
   else
   {
      return E_INVALIDARG;
   }

   return E_FAIL;
}

///////////////////////////////////////

class cBoolSetter
{
public:
   cBoolSetter(bool * pBool, bool initialValue = true) : m_pBool(pBool)
   {
      Assert(pBool != NULL);
      *m_pBool = initialValue;
   }
   ~cBoolSetter()
   {
      *m_pBool = !*m_pBool;
   }
private:
   bool * m_pBool;
};

tResult cGUIContext::ShowModalDialog(const tChar * pszDialog)
{
   if (pszDialog == NULL)
   {
      return E_POINTER;
   }

   // Only one at a time supported now
   if (m_bShowingModalDialog)
   {
      ErrorMsg("Attempt to show multiple modal dialogs\n");
      return E_FAIL;
   }

   tResult result = E_FAIL;

   cGUIPage * pPage = NULL;
   if (LoadPage(pszDialog, &pPage) == S_OK)
   {
      if (!pPage->IsModalDialogPage())
      {
         return E_FAIL;
      }

      PushPage(kDialogs, pPage);

      cBoolSetter boolSetter(&m_bShowingModalDialog, true);

      cGUIModalLoopEventListener listener(&result);
      AddEventListener(&listener);

      g_bExitModalLoop = false;
      g_pfnOuterFrameHandler = SysGetFrameCallback();

      // This function won't return until the modal loop is ended by
      // some user action (Enter, Esc, OK button click, etc.)
      SysEventLoop(GUIModalLoopFrameHandler);

      RemoveEventListener(&listener);

      PopPage(kDialogs);
   }

   return result;
}

///////////////////////////////////////

tResult cGUIContext::PushPage(eGUIPagePlane plane, cGUIPage * pPage)
{
   if (pPage == NULL)
   {
      return E_POINTER;
   }
   m_pagePlanes[plane].push_back(pPage);
   SetFocus(NULL);
   SetMouseOver(NULL);
   SetDrag(NULL);
   pPage->Activate();
   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::PopPage(eGUIPagePlane plane)
{
   if (m_pagePlanes[plane].empty())
   {
      return E_FAIL;
   }

   cGUIPage * pLastPage = m_pagePlanes[plane].back();
   m_pagePlanes[plane].pop_back();
   pLastPage->Deactivate();
   delete pLastPage, pLastPage = NULL;

   SetFocus(NULL);
   SetMouseOver(NULL);
   SetDrag(NULL);

   if (!m_pagePlanes[plane].empty())
   {
      cGUIPage * pNewPage = m_pagePlanes[plane].back();
      if (pNewPage != NULL)
      {
         pNewPage->Activate();
      }
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::PushPage(const tChar * pszPage)
{
   cGUIPage * pPage = NULL;
   if (LoadPage(pszPage, &pPage) == S_OK)
   {
      return PushPage(kPages, pPage);
   }
   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIContext::PopPage()
{
   return PopPage(kPages);
}

///////////////////////////////////////

tResult cGUIContext::AddOverlayPage(const tGUIChar * pszPage)
{
   cGUIPage * pPage = NULL;
   if (LoadPage(pszPage, &pPage) == S_OK)
   {
      return PushPage(kOverlays, pPage);
   }
   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIContext::GetElementById(const tChar * pszId, IGUIElement * * ppElement)
{
   if (pszId == NULL || ppElement == NULL)
   {
      return E_POINTER;
   }

   cGUIPage * pPage = GetCurrentPage();
   if (pPage != NULL)
   {
      return pPage->GetElement(pszId, ppElement);
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIContext::GetOverlayElement(const tGUIChar * pszId, IGUIElement * * ppElement)
{
   if (pszId == NULL || ppElement == NULL)
   {
      return E_POINTER;
   }
   tGUIPageList::iterator iter = m_pagePlanes[kOverlays].begin();
   for (; iter != m_pagePlanes[kOverlays].end(); iter++)
   {
      if ((*iter)->GetElement(pszId, ppElement) == S_OK)
      {
         return S_OK;
      }
   }
   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIContext::RequestLayout(IGUIElement * pRequester)
{
   cGUIPage * pPage = GetCurrentPage();
   if (pPage != NULL)
   {
      pPage->RequestLayout(pRequester);
      return S_OK;
   }
   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIContext::RenderGUI()
{
   cAutoIPtr<IGUIRenderDeviceContext> pRenderDeviceContext;
   if (GetRenderDeviceContext(&pRenderDeviceContext) != S_OK)
   {
      return E_FAIL;
   }

   int width, height;
   if (SysGetWindowSize(&width, &height) != S_OK)
   {
      return E_FAIL;
   }

   pRenderDeviceContext->Begin2D(width, height);

   tGUIPageList renderPages;

   // Add top-most page
   if (!m_pagePlanes[kPages].empty())
   {
      renderPages.push_back(m_pagePlanes[kPages].back());
   }

   // Add all dialog and overlay pages
   for (int i = kDialogs; i < _countof(m_pagePlanes); i++)
   {
      tGUIPageList::iterator iter = m_pagePlanes[i].begin();
      for (; iter != m_pagePlanes[i].end(); iter++)
      {
         renderPages.push_back(*iter);
      }
   }

   {
      tGUIPageList::iterator iter = renderPages.begin();
      for (; iter != renderPages.end(); iter++)
      {
         (*iter)->UpdateLayout(tGUIRect(0,0,width,height));
         (*iter)->Render(static_cast<IGUIRenderDevice*>(pRenderDeviceContext));
      }
   }

#ifdef GUI_DEBUG
   RenderDebugInfo();
#endif

   pRenderDeviceContext->End2D();

   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::GetRenderDeviceContext(IGUIRenderDeviceContext * * ppRenderDeviceContext)
{
   return m_pRenderDeviceContext.GetPointer(ppRenderDeviceContext);
}

///////////////////////////////////////

tResult cGUIContext::SetRenderDeviceContext(IGUIRenderDeviceContext * pRenderDeviceContext)
{
   SafeRelease(m_pRenderDeviceContext);
   m_pRenderDeviceContext = CTAddRef(pRenderDeviceContext);
   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::GetDefaultFont(IGUIFont * * ppFont)
{
   if (!m_pDefaultFont)
   {
      cGUIFontDesc fontDesc;
      if (GUIFontDescDefault(&fontDesc) == S_OK)
      {
         GUIFontCreate(fontDesc, NULL, &m_pDefaultFont);
      }
   }

   return m_pDefaultFont.GetPointer(ppFont);
}

///////////////////////////////////////

tResult cGUIContext::ShowDebugInfo(const tGUIPoint & placement, IGUIStyle * pStyle)
{
#ifdef GUI_DEBUG
   if (!m_bShowDebugInfo)
   {
      m_debugInfoPlacement = placement;

      if (pStyle != NULL)
      {
         pStyle->GetForegroundColor(&m_debugInfoTextColor);

         cGUIFontDesc debugFontDesc;
         if (GUIStyleFontDesc(pStyle, &debugFontDesc) == S_OK)
         {
            SafeRelease(m_pDebugFont);
            GUIFontCreate(debugFontDesc, NULL, &m_pDebugFont);
         }
      }

      m_bShowDebugInfo = true;
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

tResult cGUIContext::GetHitElement(const tScreenPoint & point, IGUIElement * * ppElement) const
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   const cGUIPage * pPage = GetCurrentPage();
   if (pPage != NULL)
   {
      std::list<IGUIElement*> hitElements;
      if (pPage->GetHitElements(point, &hitElements) == S_OK)
      {
         Assert(!hitElements.empty());
         *ppElement = CTAddRef(hitElements.front());
         std::for_each(hitElements.begin(), hitElements.end(), CTInterfaceMethod(&IGUIElement::Release));
         return S_OK;
      }
   }

   return S_FALSE;
}

///////////////////////////////////////

cGUIPage * cGUIContext::GetCurrentPage()
{
   if (!m_pagePlanes[kDialogs].empty())
   {
      return m_pagePlanes[kDialogs].back();
   }
   else if (!m_pagePlanes[kPages].empty())
   {
      return m_pagePlanes[kPages].back();
   }
   else
   {
      return NULL;
   }
}

///////////////////////////////////////

const cGUIPage * cGUIContext::GetCurrentPage() const
{
   if (!m_pagePlanes[kDialogs].empty())
   {
      return m_pagePlanes[kDialogs].back();
   }
   else if (!m_pagePlanes[kPages].empty())
   {
      return m_pagePlanes[kPages].back();
   }
   else
   {
      return NULL;
   }
}

///////////////////////////////////////

#ifdef GUI_DEBUG
tResult cGUIContext::GetDebugFont(IGUIFont * * ppFont)
{
   if (!!m_pDebugFont)
   {
      return m_pDebugFont.GetPointer(ppFont);
   }
   else
   {
      return GetDefaultFont(ppFont);
   }
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

   cGUIPage * pPage = GetCurrentPage();
   if (pPage == NULL)
   {
      return;
   }

   cAutoIPtr<IGUIFont> pFont;
   if (GetDebugFont(&pFont) == S_OK)
   {
      tGUIRect rect(0,0,0,0);
      pFont->RenderText("Xy", -1, &rect, kRT_CalcRect, m_debugInfoTextColor);

      const int lineHeight = rect.GetHeight();

      rect = tGUIRect(FloatToInt(m_debugInfoPlacement.x), FloatToInt(m_debugInfoPlacement.y), 0, 0);

      cStr temp;
      Sprintf(&temp, "Mouse: (%d, %d)", m_lastMousePos.x, m_lastMousePos.y);
      pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
      rect.Offset(0, lineHeight);

      tGUIElementList hitElements;
      if (pPage->GetHitElements(m_lastMousePos, &hitElements) == S_OK)
      {
         tGUIElementList::reverse_iterator iter = hitElements.rbegin();
         for (int index = 0; iter != hitElements.rend(); iter++, index++)
         {
            tGUIString type(GUIElementType(*iter)), temp;
            Sprintf(&temp, "Element %d: %s", index, type.c_str());
            pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
            rect.Offset(0, lineHeight);

            rect.left += lineHeight;

            const tGUISize size((*iter)->GetSize());
            Sprintf(&temp, "Size: %d x %d", FloatToInt(size.width), FloatToInt(size.height));
            pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
            rect.Offset(0, lineHeight);

            const tGUIPoint pos((*iter)->GetPosition());
            Sprintf(&temp, "Position: (%d, %d)", FloatToInt(pos.x), FloatToInt(pos.y));
            pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
            rect.Offset(0, lineHeight);

            uint nParents = 0;
            tGUIPoint absPos(GUIElementAbsolutePosition(*iter, &nParents));
            Sprintf(&temp, "Absolute Position: (%d, %d)", FloatToInt(absPos.x), FloatToInt(absPos.y));
            pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
            rect.Offset(0, lineHeight);

            Sprintf(&temp, "# Parents: %d", nParents);
            pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
            rect.Offset(0, lineHeight);

            const tGUIPoint relPoint(m_lastMousePos.x - absPos.x, m_lastMousePos.y - absPos.y); // TODO: ADDED_tScreenPoint
            Assert((*iter)->Contains(relPoint));
            Sprintf(&temp, "Mouse (relative): (%d, %d)", FloatToInt(relPoint.x), FloatToInt(relPoint.y));
            pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
            rect.Offset(0, lineHeight);

            rect.left -= lineHeight;
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

   return cGUIEventRouter<cGUIContext, IGUIContext>::HandleInputEvent(pEvent);
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

tResult GUIContextCreate(const tChar * pszScriptName)
{
   cAutoIPtr<IGUIContext> p(static_cast<IGUIContext*>(new cGUIContext(pszScriptName)));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IGUIContext, p);
}

///////////////////////////////////////////////////////////////////////////////
