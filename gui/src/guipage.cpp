///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guipage.h"
#include "guielementenum.h"
#include "guielementtools.h"
#include "guipagelayout.h"
#include "guiparse.h"
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

   if (pXmlElement->Attribute(kAttribId))
   {
      pElement->SetId(pXmlElement->Attribute(kAttribId));
   }

   {
      bool bVisible = true;
      if (GUIParseBool(pXmlElement->Attribute(kAttribVisible), &bVisible) == S_OK)
      {
         pElement->SetVisible(bVisible);
      }
   }

   {
      bool bEnabled = true;
      if (GUIParseBool(pXmlElement->Attribute(kAttribEnabled), &bEnabled) == S_OK)
      {
         pElement->SetEnabled(bEnabled);
      }
   }

   if (pXmlElement->Attribute(kAttribRendererClass))
   {
      pElement->SetRendererClass(pXmlElement->Attribute(kAttribRendererClass));
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

template <typename ITERATOR, typename FUNCTOR, typename DATA>
void GUIElementRenderLoop(ITERATOR begin, ITERATOR end, FUNCTOR f, DATA d)
{
   std::stack< std::pair<IGUIElement*, IGUIElementRenderer*> > s;

   typename ITERATOR iter = begin;
   for (; iter != end; iter++)
   {
      if ((*iter)->IsVisible())
      {
         cAutoIPtr<IGUIElementRenderer> pRenderer;
         if ((*iter)->GetRenderer(&pRenderer) != S_OK)
         {
            continue;
         }

         s.push(std::make_pair(CTAddRef(*iter), (IGUIElementRenderer*)CTAddRef(pRenderer)));
      }
   }

   while (!s.empty())
   {
      cAutoIPtr<IGUIElement> pElement(s.top().first);
      cAutoIPtr<IGUIElementRenderer> pRenderer(s.top().second);
      s.pop();

      tResult result = f(pElement, pRenderer, d);
      if (result == S_FALSE)
      {
         continue;
      }
      else if (FAILED(result))
      {
         while (!s.empty())
         {
            SafeRelease(s.top().first);
            SafeRelease(s.top().second);
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
                  s.push(std::make_pair(pChildren[i], (IGUIElementRenderer*)CTAddRef(pChildRenderer)));
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
 : m_bUpdateLayout(pElements != NULL)
 , m_bIsOverlay(false)
{
   if (pElements != NULL)
   {
      m_elements.resize(pElements->size());
      std::copy(pElements->begin(), pElements->end(), m_elements.begin());
      std::for_each(m_elements.begin(), m_elements.end(), CTInterfaceMethod(&IGUIElement::AddRef));
   }
}

///////////////////////////////////////

cGUIPage::~cGUIPage()
{
   Clear();
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

   CreateElements(pXmlDoc, NULL, GUIPageCreateElementsCallback, &pPage->m_elements);

   pGUIFactory->RemoveFactoryListener(pFL);

   if (pPage->m_elements.empty())
   {
      delete pPage;
      return E_FAIL;
   }

   pPage->m_bUpdateLayout = true;

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
   if (CountElements() == 1)
   {
      cAutoIPtr<IGUIDialogElement> pDlg;
      if (m_elements.back()->QueryInterface(IID_IGUIDialogElement, (void**)&pDlg) == S_OK)
      {
         cAutoIPtr<IGUIElement> pOk, pCancel;
         if (GetElementHelper(pDlg, cIdMatch("ok"), &pOk) != S_OK
            || GetElementHelper(pDlg, cIdMatch("cancel"), &pCancel) != S_OK)
         {
            WarnMsg("Dialog box has no \"ok\" nor \"cancel\" button\n");
         }
         return true;
      }
   }
   return false;
}

///////////////////////////////////////

void cGUIPage::Clear()
{
   std::for_each(m_elements.begin(), m_elements.end(), CTInterfaceMethod(&IGUIElement::Release));
   m_elements.clear();
}

///////////////////////////////////////

size_t cGUIPage::CountElements() const
{
   return m_elements.size();
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

tResult cGUIPage::GetActiveModalDialog(IGUIDialogElement * * ppDialog)
{
   if (ppDialog == NULL)
   {
      return E_POINTER;
   }

   if (m_elements.empty())
   {
      return S_FALSE;
   }

   return m_elements.back()->QueryInterface(IID_IGUIDialogElement, (void**)ppDialog);
}

///////////////////////////////////////

void cGUIPage::RequestLayout(IGUIElement * pRequester)
{
   m_bUpdateLayout = true;
}

///////////////////////////////////////

void cGUIPage::UpdateLayout(const tGUIRect & rect)
{
   if (m_bUpdateLayout)
   {
      cGUIPageLayout pageLayout(rect);
      GUIElementRenderLoop(m_elements.rbegin(), m_elements.rend(), pageLayout, static_cast<void*>(NULL));
      m_bUpdateLayout = false;
   }
}

///////////////////////////////////////

static tResult DoRender(IGUIElement * pElement, IGUIElementRenderer * pRenderer, IGUIRenderDevice * pRenderDevice)
{
   if (FAILED(pRenderer->Render(pElement, pRenderDevice)))
   {
      ErrorMsg1("A GUI element of type \"%s\" failed to render\n", GUIElementType(pElement).c_str());
   }
   return S_OK;
}

void cGUIPage::Render(IGUIRenderDevice * pRenderDevice)
{
   if (pRenderDevice == NULL)
   {
      return;
   }

   GUIElementRenderLoop(m_elements.rbegin(), m_elements.rend(), DoRender, pRenderDevice);
}

///////////////////////////////////////

typedef std::pair<uint, IGUIElement*> tQueueEntry;

tResult cGUIPage::GetHitElements(const tGUIPoint & point, tGUIElementList * pElements) const
{
   if (pElements == NULL)
   {
      return E_POINTER;
   }

   if (m_elements.empty())
   {
      return S_FALSE;
   }

   std::priority_queue<tQueueEntry, std::vector<tQueueEntry>, std::greater<tQueueEntry> > q;

   tGUIElementList::const_iterator iter = BeginElements();
   for (; iter != EndElements(); iter++)
   {
      uint zorder = 0;
      q.push(std::make_pair(zorder, CTAddRef(*iter)));
   }

   while (!q.empty())
   {
      cAutoIPtr<IGUIElement> pElement(q.top().second);
      uint zorder = q.top().first;
      q.pop();

      tGUIPoint pos(GUIElementAbsolutePosition(pElement));
      tGUIPoint relative(point - pos);

      if (pElement->Contains(relative))
      {
         cAutoIPtr<IGUIElementEnum> pEnum;
         if (pElement->EnumChildren(&pEnum) == S_OK)
         {
            IGUIElement * pChildren[32];
            ulong count = 0;
            while (SUCCEEDED(pEnum->Next(_countof(pChildren), &pChildren[0], &count)) && (count > 0))
            {
               for (ulong i = 0; i < count; i++)
               {
                  q.push(std::make_pair(zorder+1, pChildren[i]));
               }
               count = 0;
            }
         }

         pElements->push_front(CTAddRef(pElement));
      }
   }

   return pElements->empty() ? S_FALSE : S_OK;
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
