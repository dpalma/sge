///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entitycomponentregistry.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include <tinyxml.h>

#include "tech/dbgalloc.h" // must be last header

using namespace std;

///////////////////////////////////////////////////////////////////////////////

extern void RegisterBuiltInComponents();


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityComponentRegistry
//

///////////////////////////////////////

cEntityComponentRegistry::cEntityComponentRegistry()
{
}

///////////////////////////////////////

cEntityComponentRegistry::~cEntityComponentRegistry()
{
}

////////////////////////////////////////

tResult cEntityComponentRegistry::Init()
{
   RegisterBuiltInComponents();

   return S_OK;
}

///////////////////////////////////////

tResult cEntityComponentRegistry::Term()
{
   tComponentFactoryMap::iterator iter = m_componentFactoryMap.begin(), end = m_componentFactoryMap.end();
   for (; iter != end; ++iter)
   {
      SafeRelease(iter->second);
   }
   m_componentFactoryMap.clear();

   return S_OK;
}

///////////////////////////////////////

tResult cEntityComponentRegistry::RegisterComponentFactory(const tChar * pszComponent,
                                                           IEntityComponentFactory * pFactory)
{
   if (pszComponent == NULL || pFactory == NULL)
   {
      return E_POINTER;
   }

   pair<tComponentFactoryMap::iterator, bool> result =
      m_componentFactoryMap.insert(make_pair(pszComponent, pFactory));
   if (result.second)
   {
      pFactory->AddRef();
      return S_OK;
   }

   WarnMsg1("Failed to register entity component factory \"%s\"\n", pszComponent);
   return E_FAIL;
}

///////////////////////////////////////

tResult cEntityComponentRegistry::RevokeComponentFactory(const tChar * pszComponent)
{
   if (pszComponent == NULL)
   {
      return E_POINTER;
   }
   tComponentFactoryMap::iterator iter = m_componentFactoryMap.find(pszComponent);
   if (iter == m_componentFactoryMap.end())
   {
      return S_FALSE;
   }
   iter->second->Release();
   m_componentFactoryMap.erase(iter);
   return S_OK;
}

///////////////////////////////////////

tResult cEntityComponentRegistry::CreateComponent(const tChar * pszComponent,
                                                  const TiXmlElement * pTiXmlElement,
                                                  IEntity * pEntity,
                                                  IEntityComponent * * ppComponent)
{
   if (pszComponent == NULL || pEntity == NULL || ppComponent == NULL)
   {
      return E_POINTER;
   }

   tComponentFactoryMap::iterator f = m_componentFactoryMap.find(pszComponent);
   if (f == m_componentFactoryMap.end())
   {
      return E_FAIL;
   }

   IEntityComponentFactory * pFactory = f->second;
   return pFactory->CreateComponent(pTiXmlElement, pEntity, ppComponent);
}

///////////////////////////////////////

tResult cEntityComponentRegistry::CreateComponent(const TiXmlElement * pTiXmlElement,
                                                  IEntity * pEntity, IEntityComponent * * ppComponent)
{
#ifdef _UNICODE
   cMultiVar temp(pTiXmlElement->Value());
   const wchar_t * pszComponent = temp.ToWideString();
#else
   const char * pszComponent = pTiXmlElement->Value();
#endif

   return CreateComponent(pszComponent, pTiXmlElement, pEntity, ppComponent);
}

///////////////////////////////////////

tResult EntityComponentRegistryCreate()
{
   cAutoIPtr<IEntityComponentRegistry> p(static_cast<IEntityComponentRegistry*>(new cEntityComponentRegistry));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IEntityComponentRegistry, p);
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

namespace
{
   class cTestEntityComponent : public cComObject<IMPLEMENTS(IEntityComponent)> {};

   class cTestEntityComponentFactory : public cComObject<IMPLEMENTS(IEntityComponentFactory)>
   {
   public:
      tResult CreateComponent(const TiXmlElement *, IEntity *, IEntityComponent * * ppComponent)
      {
         *ppComponent = new cTestEntityComponent;
         return S_OK;
      }
   };
}

TEST(EntityComponentRegistryBasics)
{
   cAutoIPtr<IEntityComponentRegistry> pECR(static_cast<IEntityComponentRegistry*>(new cEntityComponentRegistry));

   cAutoIPtr<IEntityComponentFactory> pECF(static_cast<IEntityComponentFactory*>(new cTestEntityComponentFactory));

   const tChar testComponentName[] = _T("TestComponent");

   CHECK_EQUAL(S_FALSE, pECR->RevokeComponentFactory(testComponentName));
   CHECK_EQUAL(S_OK, pECR->RegisterComponentFactory(testComponentName, pECF));
   CHECK_EQUAL(S_OK, pECR->RevokeComponentFactory(testComponentName));
}

#endif

///////////////////////////////////////////////////////////////////////////////
