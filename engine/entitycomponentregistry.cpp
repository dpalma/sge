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
   return S_OK;
}

///////////////////////////////////////

tResult cEntityComponentRegistry::RegisterComponentFactory(const tChar * pszComponent,
                                                           tEntityComponentFactoryFn pfnFactory,
                                                           void * pUser)
{
   if (pszComponent == NULL || pfnFactory == NULL)
   {
      return E_POINTER;
   }

   pair<tComponentFactoryMap::iterator, bool> result = 
      m_componentFactoryMap.insert(make_pair(pszComponent, make_pair(pfnFactory, pUser)));
   if (result.second)
   {
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
   size_t nErased = m_componentFactoryMap.erase(pszComponent);
   return (nErased == 0) ? S_FALSE : S_OK;
}

///////////////////////////////////////

tResult cEntityComponentRegistry::CreateComponent(const TiXmlElement * pTiXmlElement,
                                                  IEntity * pEntity, IEntityComponent * * ppComponent)
{
   if (pTiXmlElement == NULL || pEntity == NULL || ppComponent == NULL)
   {
      return E_POINTER;
   }

#ifdef _UNICODE
   cMultiVar temp(pTiXmlElement->Value());
   const wchar_t * pszComponent = temp.ToWideString();
#else
   const char * pszComponent = pTiXmlElement->Value();
#endif

   tComponentFactoryMap::iterator f = m_componentFactoryMap.find(pszComponent);
   if (f != m_componentFactoryMap.end())
   {
      const tComponentFactoryDataPair & p = f->second;
      return (*p.first)(pTiXmlElement, pEntity, p.second, ppComponent);
   }

   return E_FAIL;
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

class cTestEntityComponent : public cComObject<IMPLEMENTS(IEntityComponent)> {};

static tResult TestEntityComponentFactory(const TiXmlElement * pTiXmlElement,
                                          IEntity * pEntity, void * pUser,
                                          IEntityComponent * * ppComponent)
{
   *ppComponent = new cTestEntityComponent;
   return S_OK;
}

TEST(EntityComponentBasics)
{
   cAutoIPtr<IEntityComponentRegistry> pECR(static_cast<IEntityComponentRegistry*>(new cEntityComponentRegistry));

   const tChar testComponentName[] = _T("TestComponent");

   CHECK_EQUAL(S_FALSE, pECR->RevokeComponentFactory(testComponentName));
   CHECK_EQUAL(S_OK, pECR->RegisterComponentFactory(testComponentName, TestEntityComponentFactory, NULL));
   CHECK_EQUAL(S_OK, pECR->RevokeComponentFactory(testComponentName));
}

#endif

///////////////////////////////////////////////////////////////////////////////
