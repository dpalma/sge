///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guifactory.h"

#include <tinyxml.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

REFERENCE_GUIELEMENTFACTORY(button);
REFERENCE_GUIELEMENTFACTORY(dialog);
REFERENCE_GUIELEMENTFACTORY(label);
REFERENCE_GUIELEMENTFACTORY(panel);
REFERENCE_GUIELEMENTFACTORY(textedit);

REFERENCE_GUIELEMENTRENDERERFACTORY(beveled);
REFERENCE_GUIELEMENTRENDERERFACTORY(basic);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIFactory
//

///////////////////////////////////////

bool cGUIFactory::gm_bInitialized = false;

///////////////////////////////////////

struct cGUIFactory::sElementFactoryMapEntry * cGUIFactory::gm_pElementFactoryMapEntries = NULL;

///////////////////////////////////////

struct cGUIFactory::sRendererFactoryMapEntry * cGUIFactory::gm_pRendererFactoryMapEntries = NULL;

///////////////////////////////////////

cGUIFactory::cGUIFactory()
{
   RegisterGlobalObject(IID_IGUIFactory, static_cast<IGlobalObject*>(this));
}

///////////////////////////////////////

cGUIFactory::~cGUIFactory()
{
}

///////////////////////////////////////

tResult cGUIFactory::Init()
{
   gm_bInitialized = true;

   while (gm_pElementFactoryMapEntries != NULL)
   {
      RegisterElementFactory(gm_pElementFactoryMapEntries->szType, 
         gm_pElementFactoryMapEntries->pFactory);
      SafeRelease(gm_pElementFactoryMapEntries->pFactory);
      sElementFactoryMapEntry * p = gm_pElementFactoryMapEntries;
      gm_pElementFactoryMapEntries = gm_pElementFactoryMapEntries->pNext;
      delete p;
   }

   while (gm_pRendererFactoryMapEntries != NULL)
   {
      RegisterElementRendererFactory(gm_pRendererFactoryMapEntries->szRenderer, 
         gm_pRendererFactoryMapEntries->pFactory);
      SafeRelease(gm_pRendererFactoryMapEntries->pFactory);
      sRendererFactoryMapEntry * p = gm_pRendererFactoryMapEntries;
      gm_pRendererFactoryMapEntries = gm_pRendererFactoryMapEntries->pNext;
      delete p;
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

tResult cGUIFactory::CreateElement(const TiXmlElement * pXmlElement, IGUIElement * * ppElement)
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   *ppElement = NULL;

   tGUIElementFactoryMap::iterator iter = m_elementFactoryMap.find(pXmlElement->Value());
   if (iter == m_elementFactoryMap.end())
   {
      return E_FAIL;
   }

   cAutoIPtr<IGUIElement> pElement;
   tResult result = iter->second->CreateElement(pXmlElement, &pElement);

   if (result != S_OK)
   {
      return result;
   }

   tGUIString rendererClass;
   if (pElement->GetRendererClass(&rendererClass) == S_OK)
   {
      tGUIRendererFactoryMap::iterator iter = m_rendererFactoryMap.find(rendererClass);
      if (iter == m_rendererFactoryMap.end())
      {
         return E_FAIL;
      }

      cAutoIPtr<IGUIElementRenderer> pRenderer;
      if (iter->second->CreateRenderer(pElement, &pRenderer) != S_OK)
      {
         return E_FAIL;
      }

      pElement->SetRenderer(pRenderer);
   }

   *ppElement = CTAddRef(pElement);

   return S_OK;
}

///////////////////////////////////////

tResult cGUIFactory::RegisterElementFactory(const char * pszType, 
                                            IGUIElementFactory * pFactory)
{
   if (pszType == NULL || pFactory == NULL)
   {
      return E_POINTER;
   }

   std::pair<tGUIElementFactoryMap::iterator, bool> result = 
      m_elementFactoryMap.insert(std::make_pair(pszType, pFactory));

   if (result.second)
   {
      pFactory->AddRef();

      return S_OK;
   }

   DebugMsg1("WARNING: Failed to register \"%s\" element factory\n", pszType);

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIFactory::RevokeElementFactory(const char * pszType)
{
   tGUIElementFactoryMap::iterator iter = m_elementFactoryMap.find(pszType);
   if (iter == m_elementFactoryMap.end())
   {
      return E_FAIL;
   }

   iter->second->Release();
   m_elementFactoryMap.erase(iter);

   return S_OK;
}

///////////////////////////////////////

tResult cGUIFactory::RegisterElementRendererFactory(const char * pszRenderer, 
                                                    IGUIElementRendererFactory * pFactory)
{
   if (pszRenderer == NULL || pFactory == NULL)
   {
      return E_POINTER;
   }

   std::pair<tGUIRendererFactoryMap::iterator, bool> result = 
      m_rendererFactoryMap.insert(std::make_pair(pszRenderer, pFactory));

   if (result.second)
   {
      pFactory->AddRef();

      return S_OK;
   }

   DebugMsg1("WARNING: Failed to register \"%s\" renderer factory\n", pszRenderer);

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIFactory::RevokeElementRendererFactory(const char * pszRenderer)
{
   tGUIRendererFactoryMap::iterator iter = m_rendererFactoryMap.find(pszRenderer);
   if (iter == m_rendererFactoryMap.end())
   {
      return E_FAIL;
   }

   iter->second->Release();
   m_rendererFactoryMap.erase(iter);

   return S_OK;
}

///////////////////////////////////////

void cGUIFactory::CleanupElementFactories()
{
   tGUIElementFactoryMap::iterator iter;
   for (iter = m_elementFactoryMap.begin(); iter != m_elementFactoryMap.end(); iter++)
   {
      iter->second->Release();
   }
   m_elementFactoryMap.clear();
}

///////////////////////////////////////

void cGUIFactory::CleanupRendererFactories()
{
   tGUIRendererFactoryMap::iterator iter;
   for (iter = m_rendererFactoryMap.begin(); iter != m_rendererFactoryMap.end(); iter++)
   {
      iter->second->Release();
   }
   m_rendererFactoryMap.clear();
}

///////////////////////////////////////

cGUIFactory::cAutoCleanupStatics::~cAutoCleanupStatics()
{
   {
      sElementFactoryMapEntry * p = gm_pElementFactoryMapEntries;
      while (p != NULL)
      {
         gm_pElementFactoryMapEntries = p->pNext;
         SafeRelease(p->pFactory);
         delete p;
         p = gm_pElementFactoryMapEntries;
      }
   }

   {
      sRendererFactoryMapEntry * p = gm_pRendererFactoryMapEntries;
      while (p != NULL)
      {
         gm_pRendererFactoryMapEntries = p->pNext;
         SafeRelease(p->pFactory);
         delete p;
         p = gm_pRendererFactoryMapEntries;
      }
   }
}

///////////////////////////////////////

cGUIFactory::cAutoCleanupStatics cGUIFactory::g_autoCleanupStatics;

///////////////////////////////////////

void GUIFactoryCreate()
{
   cAutoIPtr<IGUIFactory>(new cGUIFactory);
}

///////////////////////////////////////

tResult RegisterGUIElementFactory(const char * pszType, IGUIElementFactory * pFactory)
{
   if (cGUIFactory::gm_bInitialized)
   {
      UseGlobal(GUIFactory);
      return pGUIFactory->RegisterElementFactory(pszType, pFactory);
   }
   else
   {
      // simple queue to support adding at static init time
      cGUIFactory::sElementFactoryMapEntry * p = new cGUIFactory::sElementFactoryMapEntry;
      if (p != NULL)
      {
         strcpy(p->szType, pszType);
         p->pFactory = CTAddRef(pFactory);
         p->pNext = cGUIFactory::gm_pElementFactoryMapEntries;
         cGUIFactory::gm_pElementFactoryMapEntries = p;
         return S_OK;
      }
      return E_FAIL;
   }
}

///////////////////////////////////////

tResult RegisterGUIElementRendererFactory(const char * pszRenderer, IGUIElementRendererFactory * pFactory)
{
   if (cGUIFactory::gm_bInitialized)
   {
      UseGlobal(GUIFactory);
      return pGUIFactory->RegisterElementRendererFactory(pszRenderer, pFactory);
   }
   else
   {
      // simple queue to support adding at static init time
      cGUIFactory::sRendererFactoryMapEntry * p = new cGUIFactory::sRendererFactoryMapEntry;
      if (p != NULL)
      {
         strcpy(p->szRenderer, pszRenderer);
         p->pFactory = CTAddRef(pFactory);
         p->pNext = cGUIFactory::gm_pRendererFactoryMapEntries;
         cGUIFactory::gm_pRendererFactoryMapEntries = p;
         return S_OK;
      }
      return E_FAIL;
   }
}

///////////////////////////////////////////////////////////////////////////////
