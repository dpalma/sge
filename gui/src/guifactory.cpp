///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guifactory.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIFactory
//

///////////////////////////////////////

struct cGUIFactory::sElementFactoryMapEntry * cGUIFactory::gm_pElementFactoryMapEntries = NULL;

///////////////////////////////////////

bool cGUIFactory::gm_bInitialized = false;

///////////////////////////////////////

cGUIFactory::cGUIFactory()
 : cGlobalObject<IMPLEMENTS(IGUIFactory)>("GUIFactory")
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

   while (gm_pElementFactoryMapEntries != NULL)
   {
      RegisterElementFactory(gm_pElementFactoryMapEntries->szType, gm_pElementFactoryMapEntries->pFactory);
      SafeRelease(gm_pElementFactoryMapEntries->pFactory);
      sElementFactoryMapEntry * p = gm_pElementFactoryMapEntries;
      gm_pElementFactoryMapEntries = gm_pElementFactoryMapEntries->pNext;
      delete p;
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGUIFactory::Term()
{
   sElementFactoryMapEntry * p = gm_pElementFactoryMapEntries;
   while (p != NULL)
   {
      gm_pElementFactoryMapEntries = gm_pElementFactoryMapEntries->pNext;
      delete p;
      p = gm_pElementFactoryMapEntries;
   }

   tGUIElementFactoryMap::iterator iter;
   for (iter = m_elementFactoryMap.begin(); iter != m_elementFactoryMap.end(); iter++)
   {
      iter->second->Release();
   }
   m_elementFactoryMap.clear();

   return S_OK;
}

///////////////////////////////////////

tResult cGUIFactory::CreateElement(const char * pszType, const TiXmlElement * pXmlElement, IGUIElement * * ppElement)
{
   if (pszType == NULL || ppElement == NULL)
   {
      return E_POINTER;
   }

   tGUIElementFactoryMap::iterator iter = m_elementFactoryMap.find(pszType);
   if (iter == m_elementFactoryMap.end())
   {
      return E_FAIL;
   }

   return iter->second->CreateElement(pXmlElement, ppElement);
}

///////////////////////////////////////

tResult cGUIFactory::RegisterElementFactory(const char * pszType, IGUIElementFactory * pFactory)
{
   if (pszType == NULL || pFactory == NULL)
   {
      return E_POINTER;
   }

   std::pair<tGUIElementFactoryMap::iterator, bool> result = m_elementFactoryMap.insert(std::make_pair(pszType, pFactory));

   if (result.second)
   {
      pFactory->AddRef();

      return S_OK;
   }

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

///////////////////////////////////////////////////////////////////////////////
