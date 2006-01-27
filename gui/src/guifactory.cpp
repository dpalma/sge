///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guifactory.h"
#include "guielementapi.h"

#include <tinyxml.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

REFERENCE_GUIFACTORY(button);
REFERENCE_GUIFACTORY(dialog);
REFERENCE_GUIFACTORY(label);
REFERENCE_GUIFACTORY(layout);
REFERENCE_GUIFACTORY(listbox);
REFERENCE_GUIFACTORY(panel);
REFERENCE_GUIFACTORY(script);
REFERENCE_GUIFACTORY(scrollbar);
REFERENCE_GUIFACTORY(style);
REFERENCE_GUIFACTORY(textedit);

REFERENCE_GUIFACTORY(beveled);
REFERENCE_GUIFACTORY(basic);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIFactories
//

///////////////////////////////////////

bool cGUIFactories::gm_bInitialized = false;
cGUIFactories::tElementFactoryFnNode * cGUIFactories::gm_pElementFactoryFns = NULL;
cGUIFactories::tRendererFactoryFnNode * cGUIFactories::gm_pRendererFactoryFns = NULL;

///////////////////////////////////////

cGUIFactories::cGUIFactories()
{
}

///////////////////////////////////////

cGUIFactories::~cGUIFactories()
{
}

///////////////////////////////////////

tResult cGUIFactories::Init()
{
   gm_bInitialized = true;

   while (gm_pElementFactoryFns != NULL)
   {
      tElementFactoryFnNode * pNode = gm_pElementFactoryFns;
      RegisterElementFactory(pNode->type.c_str(), pNode->pFactoryFn);
      gm_pElementFactoryFns = pNode->pNext;
      delete pNode;
   }

   while (gm_pRendererFactoryFns != NULL)
   {
      tRendererFactoryFnNode * pNode = gm_pRendererFactoryFns;
      RegisterRendererFactory(pNode->renderer.c_str(), pNode->pFactoryFn);
      gm_pRendererFactoryFns = pNode->pNext;
      delete pNode;
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGUIFactories::Term()
{
   CleanupElementFactories();
   CleanupRendererFactories();
   return S_OK;
}

///////////////////////////////////////

tResult cGUIFactories::CreateElement(const TiXmlElement * pXmlElement, IGUIElement * pParent, IGUIElement * * ppElement)
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   *ppElement = NULL;
   cAutoIPtr<IGUIElement> pElement;
   tResult result = E_FAIL;

   tGUIElementFactoryFnMap::iterator f = m_elementFactoryFnMap.find(pXmlElement->Value());
   if (f != m_elementFactoryFnMap.end())
   {
      result = (*f->second)(pXmlElement, pParent, &pElement);
   }

   if (result != S_OK)
   {
      return result;
   }

   tGUIString rendererClass;
   if (pElement->GetRendererClass(&rendererClass) == S_OK)
   {
      cAutoIPtr<IGUIElementRenderer> pRenderer;
      if (CreateRenderer(rendererClass.c_str(), &pRenderer) == S_OK)
      {
         Verify(pElement->SetRenderer(pRenderer) == S_OK);
      }
   }

   return pElement.GetPointer(ppElement);
}

///////////////////////////////////////

tResult cGUIFactories::CreateRenderer(const tChar * pszRendererClass, IGUIElementRenderer * * ppRenderer)
{
   if (pszRendererClass == NULL || ppRenderer == NULL)
   {
      return E_POINTER;
   }

   tGUIRendererFactoryFnMap::iterator f = m_rendererFactoryFnMap.find(pszRendererClass);
   if (f != m_rendererFactoryFnMap.end())
   {
      return (*f->second)(NULL, ppRenderer);
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIFactories::RegisterElementFactory(const tChar * pszType, tGUIElementFactoryFn pFactoryFn)
{
   if (pszType == NULL || pFactoryFn == NULL)
   {
      return E_POINTER;
   }

   std::pair<tGUIElementFactoryFnMap::iterator, bool> result = 
      m_elementFactoryFnMap.insert(std::make_pair(pszType, pFactoryFn));
   if (result.second)
   {
      return S_OK;
   }

   WarnMsg1("Failed to register \"%s\" element factory\n", pszType);
   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIFactories::RevokeElementFactory(const char * pszType)
{
   if (pszType == NULL)
   {
      return E_POINTER;
   }
   size_t nErased = m_elementFactoryFnMap.erase(pszType);
   return (nErased == 0) ? S_FALSE : S_OK;
}

///////////////////////////////////////

tResult cGUIFactories::RegisterRendererFactory(const tChar * pszRenderer,
                                               tGUIRendererFactoryFn pFactoryFn)
{
   if (pszRenderer == NULL || pFactoryFn == NULL)
   {
      return E_POINTER;
   }

   std::pair<tGUIRendererFactoryFnMap::iterator, bool> result = 
      m_rendererFactoryFnMap.insert(std::make_pair(pszRenderer, pFactoryFn));
   if (result.second)
   {
      return S_OK;
   }

   WarnMsg1("Failed to register \"%s\" renderer factory\n", pszRenderer);
   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIFactories::RevokeRendererFactory(const tChar * pszRenderer)
{
   if (pszRenderer == NULL)
   {
      return E_POINTER;
   }
   size_t nErased = m_rendererFactoryFnMap.erase(pszRenderer);
   return (nErased == 0) ? S_FALSE : S_OK;
}

///////////////////////////////////////

void cGUIFactories::CleanupElementFactories()
{
   m_elementFactoryFnMap.clear();
}

///////////////////////////////////////

void cGUIFactories::CleanupRendererFactories()
{
   m_rendererFactoryFnMap.clear();
}

///////////////////////////////////////

cGUIFactories::cAutoCleanupStatics::~cAutoCleanupStatics()
{
   {
      while (gm_pElementFactoryFns != NULL)
      {
         tElementFactoryFnNode * pNode = gm_pElementFactoryFns;
         gm_pElementFactoryFns = pNode->pNext;
         delete pNode;
      }
   }

   {
      while (gm_pRendererFactoryFns != NULL)
      {
         tRendererFactoryFnNode * pNode = gm_pRendererFactoryFns;
         gm_pRendererFactoryFns = pNode->pNext;
         delete pNode;
      }
   }
}

///////////////////////////////////////

cGUIFactories::cAutoCleanupStatics cGUIFactories::g_autoCleanupStatics;

///////////////////////////////////////

tResult GUIFactoriesCreate()
{
   cAutoIPtr<IGUIFactories> p(static_cast<IGUIFactories*>(new cGUIFactories));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IGUIFactories, p);
}

///////////////////////////////////////

tResult GUIRegisterElementFactory(const tChar * pszType, tGUIElementFactoryFn pFactoryFn)
{
   if (pszType == NULL || pFactoryFn == NULL)
   {
      return E_POINTER;
   }

   if (cGUIFactories::gm_bInitialized)
   {
      UseGlobal(GUIFactories);
      return pGUIFactories->RegisterElementFactory(pszType, pFactoryFn);
   }
   else
   {
      // simple queue to support adding at static init time
      cGUIFactories::tElementFactoryFnNode * pNode = new cGUIFactories::tElementFactoryFnNode;
      if (pNode == NULL)
      {
         return E_OUTOFMEMORY;
      }
      pNode->type.assign(pszType);
      pNode->pFactoryFn = pFactoryFn;
      pNode->pNext = cGUIFactories::gm_pElementFactoryFns;
      cGUIFactories::gm_pElementFactoryFns = pNode;
      return S_OK;
   }
}

///////////////////////////////////////

tResult GUIRegisterRendererFactory(const tChar * pszRenderer, tGUIRendererFactoryFn pFactoryFn)
{
   if (pszRenderer == NULL || pFactoryFn == NULL)
   {
      return E_POINTER;
   }

   if (cGUIFactories::gm_bInitialized)
   {
      UseGlobal(GUIFactories);
      return pGUIFactories->RegisterRendererFactory(pszRenderer, pFactoryFn);
   }
   else
   {
      // simple queue to support adding at static init time
      cGUIFactories::tRendererFactoryFnNode * pNode = new cGUIFactories::tRendererFactoryFnNode;
      if (pNode == NULL)
      {
         return E_OUTOFMEMORY;
      }
      pNode->renderer.assign(pszRenderer);
      pNode->pFactoryFn = pFactoryFn;
      pNode->pNext = cGUIFactories::gm_pRendererFactoryFns;
      cGUIFactories::gm_pRendererFactoryFns = pNode;
      return S_OK;
   }
}

///////////////////////////////////////////////////////////////////////////////
