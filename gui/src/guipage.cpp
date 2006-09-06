///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guipage.h"
#include "guielementenum.h"
#include "guielementtools.h"
#include "guipagelayout.h"
#include "guistrings.h"
#include "guistyleapi.h"

#include "scriptapi.h"

#include "globalobj.h"

#include <tinyxml.h>

#include <stack>
#include <queue>

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

static void CreateElements(const TiXmlNode * pTiXmlNode, IGUIElement * pParent,
                           void (*pfnCallback)(IGUIElement *, IGUIElement *, void *),
                           void * pCallbackData)
{
   if (pTiXmlNode == NULL)
   {
      return;
   }

   UseGlobal(GUIFactory);

   cAutoIPtr<IGUIContainerElement> pContainer;
   if (pParent != NULL)
   {
      pParent->QueryInterface(IID_IGUIContainerElement, (void**)&pContainer);
   }

   for (const TiXmlElement * pXmlElement = pTiXmlNode->FirstChildElement();
      pXmlElement != NULL; pXmlElement = pXmlElement->NextSiblingElement())
   {
      if (pXmlElement->Type() != TiXmlNode::ELEMENT)
      {
         continue;
      }

      cAutoIPtr<IGUIElement> pElement;
      if (pGUIFactory->CreateElement(pXmlElement, pParent, &pElement) == S_OK)
      {
         if (pfnCallback != NULL)
         {
            (*pfnCallback)(pElement, pParent, pCallbackData);
         }
         if (!!pContainer && (pContainer->AddElement(pElement) != S_OK))
         {
            WarnMsg("Error creating child element\n");
         }
         CreateElements(pXmlElement, pElement, pfnCallback, pCallbackData);
      }
   }
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIPageCreateFactoryListener
//

class cGUIPageCreateFactoryListener : public cComObject<IMPLEMENTS(IGUIFactoryListener)>
{
public:
   cGUIPageCreateFactoryListener();
   ~cGUIPageCreateFactoryListener();

   virtual tResult PreCreateElement(const TiXmlElement * pXmlElement, IGUIElement * pParent);
   virtual void OnCreateElement(const TiXmlElement * pXmlElement, IGUIElement * pParent, IGUIElement * pElement);

private:
   std::list<IGUIStyleElement*> m_styleElements;
};

///////////////////////////////////////

cGUIPageCreateFactoryListener::cGUIPageCreateFactoryListener()
{
}

///////////////////////////////////////

cGUIPageCreateFactoryListener::~cGUIPageCreateFactoryListener()
{
   std::for_each(m_styleElements.begin(), m_styleElements.end(), CTInterfaceMethod(&IGUIStyleElement::Release));
   m_styleElements.clear();
}

///////////////////////////////////////

tResult cGUIPageCreateFactoryListener::PreCreateElement(const TiXmlElement * pXmlElement,
                                                        IGUIElement * pParent)
{
   return S_OK;
}

///////////////////////////////////////

void cGUIPageCreateFactoryListener::OnCreateElement(const TiXmlElement * pXmlElement,
                                                    IGUIElement * pParent,
                                                    IGUIElement * pElement)
{
   if (pElement == NULL)
   {
      return;
   }

   cAutoIPtr<IGUIStyleElement> pStyle;
   if (pElement->QueryInterface(IID_IGUIStyleElement, (void**)&pStyle) == S_OK)
   {
      m_styleElements.push_back(CTAddRef(pStyle));
   }
   else
   {
      cAutoIPtr<IGUIStyle> pClassStyle, pInlineStyle;

      if (!m_styleElements.empty())
      {
         // TODO: how to handle multiple style sheets on the same page?
         cAutoIPtr<IGUIStyleSheet> pStyleSheet;
         if (m_styleElements.front()->GetStyleSheet(&pStyleSheet) == S_OK)
         {
            pStyleSheet->GetStyle(pXmlElement->Value(),
               pXmlElement->Attribute(kAttribStyleClass), &pClassStyle);
         }
      }

      {
         const char * pszStyleAttrib = pXmlElement->Attribute(kAttribStyle);
         if (pszStyleAttrib != NULL)
         {
            GUIStyleParseInline(pszStyleAttrib, -1, pClassStyle, &pInlineStyle);
         }
      }

      if (!!pInlineStyle)
      {
         pElement->SetStyle(pInlineStyle);
      }
      else
      {
         pElement->SetStyle(pClassStyle);
      }
   }
}


///////////////////////////////////////////////////////////////////////////////

struct sRenderLoopStackElement
{
   sRenderLoopStackElement(IGUIElement * pElement_, IGUIElementRenderer * pRenderer_, tGUIPoint base_)
    : pElement(pElement_)
    , pRenderer(pRenderer_)
    , base(base_)
   {
   }

   IGUIElement * pElement;
   IGUIElementRenderer * pRenderer;
   tGUIPoint base;
};

typedef std::stack<sRenderLoopStackElement> tRenderLoopStack;

template <typename ITERATOR, typename FUNCTOR, typename DATA>
void GUIElementRenderLoop(ITERATOR begin, ITERATOR end, FUNCTOR f, DATA d)
{
   tRenderLoopStack s;

   for (ITERATOR iter = begin; iter != end; iter++)
   {
      cAutoIPtr<IGUIElement> pElement(CTAddRef(*iter));
      if (pElement->IsVisible())
      {
         cAutoIPtr<IGUIElementRenderer> pRenderer;
         if (pElement->GetRenderer(&pRenderer) == S_OK)
         {
            s.push(sRenderLoopStackElement(CTAddRef(pElement), CTAddRef(pRenderer), tGUIPoint(0,0)));
         }
      }
   }

   while (!s.empty())
   {
      const sRenderLoopStackElement & t = s.top();
      cAutoIPtr<IGUIElement> pElement(t.pElement);
      cAutoIPtr<IGUIElementRenderer> pRenderer(t.pRenderer);
      tGUIPoint position(pElement->GetPosition() + t.base);
      s.pop();

      tResult result = f(pElement, pRenderer, position, d);
      if (result == S_FALSE)
      {
         continue;
      }
      else if (FAILED(result))
      {
         while (!s.empty())
         {
            SafeRelease(s.top().pElement);
            SafeRelease(s.top().pRenderer);
            s.pop();
         }
         break;
      }

      cAutoIPtr<IGUIElementEnum> pEnum;
      if (pElement->EnumChildren(&pEnum) == S_OK)
      {
         IGUIElement * pChildren[32];
         ulong count = 0;
         while (SUCCEEDED(pEnum->Next(_countof(pChildren), &pChildren[0], &count)) && (count > 0))
         {
            for (ulong i = 0; i < count; i++)
            {
               if (pChildren[i]->IsVisible())
               {
                  cAutoIPtr<IGUIElementRenderer> pChildRenderer;
                  if (pChildren[i]->GetRenderer(&pChildRenderer) != S_OK)
                  {
                     pChildRenderer = pRenderer; // Copying smart pointers--no AddRef
                  }
                  s.push(sRenderLoopStackElement(pChildren[i], CTAddRef(pChildRenderer), position));
               }
               else
               {
                  SafeRelease(pChildren[i]);
               }
            }
            count = 0;
         }
      }
   }
}


///////////////////////////////////////////////////////////////////////////////

template <typename F>
static tResult GetElementHelper(IGUIElement * pParent, F f, IGUIElement * * ppElement)
{
   if (f(pParent))
   {
      *ppElement = CTAddRef(pParent);
      return S_OK;
   }
   else
   {
      cAutoIPtr<IGUIElementEnum> pEnum;
      if (pParent->EnumChildren(&pEnum) == S_OK)
      {
         IGUIElement * pChildren[32];
         ulong count = 0;

         while (SUCCEEDED((pEnum->Next(_countof(pChildren), &pChildren[0], &count))) && (count > 0))
         {
            for (ulong i = 0; i < count; i++)
            {
               if (GetElementHelper(pChildren[i], f, ppElement) == S_OK)
               {
                  for (; i < count; i++)
                  {
                     SafeRelease(pChildren[i]);
                  }
                  return S_OK;
               }

               SafeRelease(pChildren[i]);
            }

            count = 0;
         }
      }
   }

   return S_FALSE;
}

class cIdMatch
{
public:
   cIdMatch(const tChar * pszId) : m_id(pszId ? pszId : "") {}
   bool operator()(IGUIElement * pElement) { return GUIElementIdMatch(pElement, m_id.c_str()); }
private:
   cStr m_id;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIPage
//

///////////////////////////////////////

cGUIPage::cGUIPage(const tGUIElementList * pElements)
{
   if (pElements != NULL)
   {
      m_elements.resize(pElements->size());
      std::copy(pElements->begin(), pElements->end(), m_elements.begin());
      std::for_each(m_elements.begin(), m_elements.end(), CTInterfaceMethod(&IGUIElement::AddRef));

      RequestLayout(NULL, kGUILayoutDefault);
   }
}

///////////////////////////////////////

cGUIPage::~cGUIPage()
{
   ClearElements();
   ClearLayoutRequests();
}

///////////////////////////////////////

static void GUIPageCreateElementsCallback(IGUIElement * pElement, IGUIElement * pParent, void * pData)
{
   if (pData == NULL)
   {
      return;
   }

   if (pElement != NULL)
   {
      // Add only top-level elements to the list
      if (pParent == NULL)
      {
         tGUIElementList * pElements = (tGUIElementList*)pData;
         pElements->push_back(CTAddRef(pElement));
      }
   }
}

///////////////////////////////////////

tResult cGUIPage::Create(const TiXmlDocument * pXmlDoc, cGUIPage * * ppPage)
{
   if (pXmlDoc == NULL || ppPage == NULL)
   {
      return E_POINTER;
   }

   const TiXmlNode * pTopLevel = pXmlDoc->FirstChildElement();

   if (pTopLevel == NULL)
   {
      return E_FAIL;
   }

   if (strcmp(pTopLevel->Value(), kElementPage) != 0)
   {
      ErrorMsg("Top-level element should be a <page>\n");
      return E_FAIL;
   }

   cAutoIPtr<IGUIFactoryListener> pFL(static_cast<IGUIFactoryListener*>(new cGUIPageCreateFactoryListener));
   if (!pFL)
   {
      return E_OUTOFMEMORY;
   }

   cGUIPage * pPage = new cGUIPage(NULL);
   if (pPage == NULL)
   {
      return E_OUTOFMEMORY;
   }

   UseGlobal(GUIFactory);
   pGUIFactory->AddFactoryListener(pFL);

   CreateElements(pTopLevel, NULL, GUIPageCreateElementsCallback, &pPage->m_elements);

   pGUIFactory->RemoveFactoryListener(pFL);

   if (pPage->m_elements.empty())
   {
      delete pPage;
      return E_FAIL;
   }

   pPage->RequestLayout(NULL, kGUILayoutDefault);

   *ppPage = pPage;
   return S_OK;
}

///////////////////////////////////////

void cGUIPage::Activate()
{
   RunScripts();
}

///////////////////////////////////////

void cGUIPage::Deactivate()
{
}

///////////////////////////////////////

bool cGUIPage::IsModalDialogPage() const
{
   if (!m_elements.empty())
   {
      uint nDialogs = 0;

      tGUIElementList::const_iterator iter = m_elements.begin();
      for (; iter != m_elements.end(); iter++)
      {
         cAutoIPtr<IGUIDialogElement> pDlg;
         if ((*iter)->QueryInterface(IID_IGUIDialogElement, (void**)&pDlg) == S_OK)
         {
            nDialogs++;
            cAutoIPtr<IGUIElement> pOk, pCancel;
            if (GetElementHelper(pDlg, cIdMatch("ok"), &pOk) != S_OK
               || GetElementHelper(pDlg, cIdMatch("cancel"), &pCancel) != S_OK)
            {
               WarnMsg("Dialog box has no \"ok\" nor \"cancel\" button\n");
            }
         }
      }

      WarnMsgIf(nDialogs > 1, "Multiple dialogs defined in one XML file\n");

      return (nDialogs > 0);
   }

   return false;
}

///////////////////////////////////////

static tResult GUIGetElement(const tGUIElementList & elements, const tChar * pszId, IGUIElement * * ppElement)
{
   if (pszId == NULL || ppElement == NULL)
   {
      return E_POINTER;
   }
   tGUIElementList::const_iterator iter = elements.begin();
   for (; iter != elements.end(); iter++)
   {
      if (GetElementHelper(*iter, cIdMatch(pszId), ppElement) == S_OK)
      {
         return S_OK;
      }
   }
   return S_FALSE;
}

tResult cGUIPage::GetElement(const tChar * pszId, IGUIElement * * ppElement)
{
   return GUIGetElement(m_elements, pszId, ppElement);
}

///////////////////////////////////////

void cGUIPage::RequestLayout(IGUIElement * pRequester, uint options)
{
   // TODO: Eliminate redundant requests
   m_layoutRequests.push_back(std::make_pair(CTAddRef(pRequester), options));
}

///////////////////////////////////////

void cGUIPage::UpdateLayout(const tGUIRect & rect)
{
   static const int kUpdateLayoutInsanity = 16;
   int nLayoutUpdates = 0;
   while (!m_layoutRequests.empty() && (nLayoutUpdates < kUpdateLayoutInsanity))
   {
      cAutoIPtr<IGUIElement> pRequester(m_layoutRequests.front().first);
      uint options = m_layoutRequests.front().second;
      m_layoutRequests.pop_front();

      cGUIPageLayout pageLayout(rect, pRequester, options);
      GUIElementRenderLoop(m_elements.rbegin(), m_elements.rend(), pageLayout, static_cast<void*>(NULL));

      ++nLayoutUpdates;
   }
}

///////////////////////////////////////

static tResult DoRender(IGUIElement * pElement, IGUIElementRenderer * pRenderer, const tGUIPoint & position, IRender2D * pRender2D)
{
   //{
   //   tGUIPoint ap(GUIElementAbsolutePosition(pElement, NULL));
   //   Assert(AlmostEqual(ap.x, position.x));
   //   Assert(AlmostEqual(ap.y, position.y));
   //}
   if (FAILED(pRenderer->Render(pElement, position, pRender2D)))
   {
      ErrorMsg1("A GUI element of type \"%s\" failed to render\n", GUIElementType(pElement).c_str());
   }
   return S_OK;
}

void cGUIPage::Render(IRender2D * pRender2D)
{
   if (pRender2D == NULL)
   {
      return;
   }

   GUIElementRenderLoop(m_elements.rbegin(), m_elements.rend(), DoRender, pRender2D);
}

///////////////////////////////////////

struct sGetElementData
{
   tScreenPoint point;
   tGUIElementList * pElements;
};

static tResult GetHitElementHelper(IGUIElement * pElement, IGUIElementRenderer * pRenderer,
                                   const tGUIPoint & position, sGetElementData * pGetElementData)
{
   const tGUISize & size = pElement->GetSize();

   tRectf rect(position.x, position.y, position.x + size.width, position.y + size.height);

   if (rect.PtInside(static_cast<float>(pGetElementData->point.x), static_cast<float>(pGetElementData->point.y)))
   {
      pGetElementData->pElements->push_front(CTAddRef(pElement));
      return S_OK;
   }

   return S_FALSE;
}

tResult cGUIPage::GetHitElements(const tScreenPoint & point, tGUIElementList * pElements) const
{
   if (pElements == NULL)
   {
      return E_POINTER;
   }

   if (m_elements.empty())
   {
      return S_FALSE;
   }

   sGetElementData getElementData;
   getElementData.point = point;
   getElementData.pElements = pElements;
   GUIElementRenderLoop(m_elements.begin(), m_elements.end(), GetHitElementHelper, &getElementData);

   return pElements->empty() ? S_FALSE : S_OK;
}

///////////////////////////////////////

void cGUIPage::ClearElements()
{
   std::for_each(m_elements.begin(), m_elements.end(), CTInterfaceMethod(&IGUIElement::Release));
   m_elements.clear();
}

///////////////////////////////////////

void cGUIPage::ClearLayoutRequests()
{
   tLayoutRequests::iterator iter = m_layoutRequests.begin();
   for (; iter != m_layoutRequests.end(); iter++)
   {
      SafeRelease(iter->first);
   }
   m_layoutRequests.clear();
}

///////////////////////////////////////

static tResult RunScriptHelper(IGUIElement * pElement)
{
   if (pElement == NULL)
   {
      return E_POINTER;
   }
   cAutoIPtr<IGUIScriptElement> pScript;
   if (pElement->QueryInterface(IID_IGUIScriptElement, (void**)&pScript) == S_OK)
   {
      tGUIString script;
      if (pScript->GetScript(&script) == S_OK)
      {
         UseGlobal(ScriptInterpreter);
         if (pScriptInterpreter->ExecString(script.c_str()) != S_OK)
         {
            WarnMsg("An error occured running script element\n");
         }
         else
         {
            return S_OK;
         }
      }
   }
   else
   {
      cAutoIPtr<IGUIElementEnum> pEnum;
      if (pElement->EnumChildren(&pEnum) == S_OK)
      {
         ForEach(pEnum, RunScriptHelper);
      }
   }
   return S_FALSE;
}

void cGUIPage::RunScripts()
{
   std::for_each(BeginElements(), EndElements(), RunScriptHelper);
}


///////////////////////////////////////////////////////////////////////////////
