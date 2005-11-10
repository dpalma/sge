///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guipage.h"
#include "guielementtools.h"

#include "scriptapi.h"

#include "globalobj.h"

#include <stack>
#include <queue>

#include "dbgalloc.h" // must be last header

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

void cGUIPage::Clear()
{
   std::for_each(m_elements.begin(), m_elements.end(), CTInterfaceMethod(&IGUIElement::Release));
   m_elements.clear();
}

///////////////////////////////////////

tResult cGUIPage::AddElement(IGUIElement * pElement)
{
   if (pElement == NULL)
   {
      return E_POINTER;
   }

   if (HasElement(pElement) == S_OK)
   {
      return S_FALSE;
   }

   m_elements.push_back(CTAddRef(pElement));
   m_bUpdateLayout = true;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIPage::RemoveElement(IGUIElement * pElement)
{
   if (pElement == NULL)
   {
      return E_POINTER;
   }

   tGUIElementList::iterator f = std::find_if(m_elements.begin(), m_elements.end(), cSameAs(pElement));
   if (f != m_elements.end())
   {
      (*f)->Release();
      m_elements.erase(f);
      m_bUpdateLayout = true;
      return S_OK;
   }

   return S_FALSE;
}

///////////////////////////////////////

size_t cGUIPage::CountElements() const
{
   return m_elements.size();
}

///////////////////////////////////////

tResult cGUIPage::HasElement(IGUIElement * pElement) const
{
   if (pElement == NULL)
   {
      return E_POINTER;
   }

   tGUIElementList::const_iterator f = std::find_if(m_elements.begin(), m_elements.end(), cSameAs(pElement));
   return (f != m_elements.end()) ? S_OK : S_FALSE;
}

///////////////////////////////////////

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

tResult GUIGetElement(const tGUIElementList & elements, const tChar * pszId, IGUIElement * * ppElement)
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

class cTypeMatch
{
public:
   cTypeMatch(REFGUID iid, tGUIElementList * pElements) : m_iid(iid), m_pElements(pElements) {}
   bool operator()(IGUIElement * pElement)
   {
      cAutoIPtr<IUnknown> pUnk;
      if (pElement != NULL && pElement->QueryInterface(m_iid, (void**)&pUnk) == S_OK)
      {
         m_pElements->push_back(CTAddRef(pElement));
      }
      return false;
   }
private:
   REFGUID m_iid;
   tGUIElementList * m_pElements;
};

tResult cGUIPage::GetElementsOfType(REFGUID iid, tGUIElementList * pElements) const
{
   if (pElements == NULL)
   {
      return E_POINTER;
   }
   cAutoIPtr<IGUIElement> pUnused;
   tGUIElementList::const_iterator iter = m_elements.begin();
   for (; iter != m_elements.end(); iter++)
   {
      GetElementHelper(*iter, cTypeMatch(iid, pElements), &pUnused);
   }
   return pElements->empty() ? S_FALSE : S_OK;
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

void cGUIPage::UpdateLayout(const tGUIRect & rect)
{
   if (m_bUpdateLayout)
   {
      std::for_each(BeginElements(), EndElements(), cSizeAndPlaceElement(rect));
      m_bUpdateLayout = false;
   }
}

///////////////////////////////////////

template <typename ITERATOR, typename FUNCTOR, typename DATA>
void GUIElementRenderLoop(ITERATOR begin, ITERATOR end, FUNCTOR f, DATA d)
{
   std::stack< std::pair<IGUIElement*, IGUIElementRenderer*> > s;

   tGUIElementList::reverse_iterator iter = begin;
   for (; iter != end; iter++)
   {
      if ((*iter)->IsVisible())
      {
         cAutoIPtr<IGUIElementRenderer> pRenderer;
         if ((*iter)->GetRenderer(&pRenderer) != S_OK)
         {
            WarnMsg("Top-level GUI element has no renderer\n");
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

      f(pElement, pRenderer, d);

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

void DoRender(IGUIElement * pElement, IGUIElementRenderer * pRenderer, IGUIRenderDevice * pRenderDevice)
{
   if (FAILED(pRenderer->Render(pElement, pRenderDevice)))
   {
      cStr type;
      if (GUIElementType(pElement, &type) != S_OK)
      {
         type = _T("Unknown");
      }

      ErrorMsg1("A GUI element of type \"%s\" failed to render\n", type.c_str());
   }
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

typedef std::pair<IGUIElement*, uint> tQueueEntry;

bool operator >(const tQueueEntry & lhs, const tQueueEntry & rhs)
{
   return lhs.second > rhs.second;
}

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
      cAutoIPtr<IGUIDialogElement> pTempDlg;
      if ((*iter)->QueryInterface(IID_IGUIDialogElement, (void**)&pTempDlg) == S_OK)
      {
         zorder = 1000; // modal dialogs should have a higher z order
      }
      q.push(tQueueEntry(CTAddRef(*iter), zorder));
   }

   while (!q.empty())
   {
      cAutoIPtr<IGUIElement> pElement(q.top().first);
      uint zorder = q.top().second;
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
                  q.push(tQueueEntry(pChildren[i], zorder+1));
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
