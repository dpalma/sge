///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guifactory.h"
#include "guielementapi.h"
#include "guiparse.h"
#include "guistrings.h"

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
REFERENCE_GUIFACTORY(titlebar);

REFERENCE_GUIFACTORY(beveled);
REFERENCE_GUIFACTORY(basic);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIFactory
//

///////////////////////////////////////

bool cGUIFactory::gm_bInitialized = false;
cGUIFactory::tElementFactoryFnNode * cGUIFactory::gm_pElementFactoryFns = NULL;
cGUIFactory::tRendererFactoryFnNode * cGUIFactory::gm_pRendererFactoryFns = NULL;

///////////////////////////////////////

cGUIFactory::cGUIFactory()
{
}

///////////////////////////////////////

cGUIFactory::~cGUIFactory()
{
}

///////////////////////////////////////

tResult cGUIFactory::Init()
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

tResult cGUIFactory::Term()
{
   CleanupElementFactories();
   CleanupRendererFactories();
   return S_OK;
}

///////////////////////////////////////

tResult cGUIFactory::AddFactoryListener(IGUIFactoryListener * pListener)
{
   return tCP::Connect(pListener);
}

///////////////////////////////////////

tResult cGUIFactory::RemoveFactoryListener(IGUIFactoryListener * pListener)
{
   return tCP::Disconnect(pListener);
}

///////////////////////////////////////

tResult cGUIFactory::CreateElement(const TiXmlElement * pXmlElement,
                                   IGUIElement * pParent,
                                   IGUIElement * * ppElement)
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   {
      tCP::tSinksIterator iter = tCP::BeginSinks();
      tCP::tSinksIterator end = tCP::EndSinks();
      for (; iter != end; iter++)
      {
         if ((*iter)->PreCreateElement(pXmlElement, pParent) != S_OK)
         {
            InfoMsg("GUI element creation vetoed by a listener\n");
            return S_FALSE;
         }
      }
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
      cAutoIPtr<IGUIElementRenderer> pRenderer;
      if (CreateRenderer(pXmlElement->Attribute(kAttribRendererClass), &pRenderer) == S_OK)
      {
         Verify(pElement->SetRenderer(pRenderer) == S_OK);
      }
   }

   {
      tCP::tSinksIterator iter = tCP::BeginSinks();
      tCP::tSinksIterator end = tCP::EndSinks();
      for (; iter != end; iter++)
      {
         (*iter)->OnCreateElement(pXmlElement, pParent, pElement);
      }
   }

   return pElement.GetPointer(ppElement);
}

///////////////////////////////////////

tResult cGUIFactory::CreateRenderer(const tChar * pszRendererClass, IGUIElementRenderer * * ppRenderer)
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

tResult cGUIFactory::RegisterElementFactory(const tChar * pszType, tGUIElementFactoryFn pFactoryFn)
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

tResult cGUIFactory::RevokeElementFactory(const char * pszType)
{
   if (pszType == NULL)
   {
      return E_POINTER;
   }
   size_t nErased = m_elementFactoryFnMap.erase(pszType);
   return (nErased == 0) ? S_FALSE : S_OK;
}

///////////////////////////////////////

tResult cGUIFactory::RegisterRendererFactory(const tChar * pszRenderer,
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

tResult cGUIFactory::RevokeRendererFactory(const tChar * pszRenderer)
{
   if (pszRenderer == NULL)
   {
      return E_POINTER;
   }
   size_t nErased = m_rendererFactoryFnMap.erase(pszRenderer);
   return (nErased == 0) ? S_FALSE : S_OK;
}

///////////////////////////////////////

void cGUIFactory::CleanupElementFactories()
{
   m_elementFactoryFnMap.clear();
}

///////////////////////////////////////

void cGUIFactory::CleanupRendererFactories()
{
   m_rendererFactoryFnMap.clear();
}

///////////////////////////////////////

cGUIFactory::cAutoCleanupStatics::~cAutoCleanupStatics()
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

cGUIFactory::cAutoCleanupStatics cGUIFactory::g_autoCleanupStatics;

///////////////////////////////////////

tResult GUIFactoryCreate()
{
   cAutoIPtr<IGUIFactory> p(static_cast<IGUIFactory*>(new cGUIFactory));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IGUIFactory, p);
}

///////////////////////////////////////

tResult GUIRegisterElementFactory(const tChar * pszType, tGUIElementFactoryFn pFactoryFn)
{
   if (pszType == NULL || pFactoryFn == NULL)
   {
      return E_POINTER;
   }

   if (cGUIFactory::gm_bInitialized)
   {
      UseGlobal(GUIFactory);
      return pGUIFactory->RegisterElementFactory(pszType, pFactoryFn);
   }
   else
   {
      // simple queue to support adding at static init time
      cGUIFactory::tElementFactoryFnNode * pNode = new cGUIFactory::tElementFactoryFnNode;
      if (pNode == NULL)
      {
         return E_OUTOFMEMORY;
      }
      pNode->type.assign(pszType);
      pNode->pFactoryFn = pFactoryFn;
      pNode->pNext = cGUIFactory::gm_pElementFactoryFns;
      cGUIFactory::gm_pElementFactoryFns = pNode;
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

   if (cGUIFactory::gm_bInitialized)
   {
      UseGlobal(GUIFactory);
      return pGUIFactory->RegisterRendererFactory(pszRenderer, pFactoryFn);
   }
   else
   {
      // simple queue to support adding at static init time
      cGUIFactory::tRendererFactoryFnNode * pNode = new cGUIFactory::tRendererFactoryFnNode;
      if (pNode == NULL)
      {
         return E_OUTOFMEMORY;
      }
      pNode->renderer.assign(pszRenderer);
      pNode->pFactoryFn = pFactoryFn;
      pNode->pNext = cGUIFactory::gm_pRendererFactoryFns;
      cGUIFactory::gm_pRendererFactoryFns = pNode;
      return S_OK;
   }
}

///////////////////////////////////////////////////////////////////////////////
