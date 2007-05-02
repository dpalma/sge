///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entity.h"

#include "tech/comenum.h"

#include <list>

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include "tech/dbgalloc.h" // must be last header

using namespace std;


///////////////////////////////////////////////////////////////////////////////

typedef list<IEntityComponent *> tEntityComponentList;
typedef cComObject<cComEnum<IEnumEntityComponents,
                            &IID_IEnumEntityComponents,
                            IEntityComponent*,
                            CopyInterface<IEntityComponent>,
                            tEntityComponentList>,
                   &IID_IEnumEntityComponents> tEntityComponentListEnum;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntity
//

///////////////////////////////////////

tResult EntityCreate(const tChar * pszTypeName, tEntityId id, IEntity * * ppEntity)
{
   if (pszTypeName == NULL || ppEntity == NULL)
   {
      return E_POINTER;
   }

   cEntity * pEntity = new cEntity(pszTypeName, id);
   if (pEntity == NULL)
   {
      return E_OUTOFMEMORY;
   }

   *ppEntity = static_cast<IEntity*>(pEntity);
   return S_OK;
}

///////////////////////////////////////

cEntity::cEntity(const tChar * pszTypeName, tEntityId id)
 : m_typeName((pszTypeName != NULL) ? pszTypeName : _T(""))
 , m_id(id)
{
}

///////////////////////////////////////

cEntity::~cEntity()
{
   RemoveAllComponents();
}

///////////////////////////////////////

tResult cEntity::GetTypeName(cStr * pTypeName) const
{
   if (pTypeName == NULL)
   {
      return E_POINTER;
   }
   *pTypeName = m_typeName;
   return m_typeName.empty() ? S_FALSE : S_OK;
}

///////////////////////////////////////

tEntityId cEntity::GetId() const
{
   return m_id;
}

///////////////////////////////////////

tResult cEntity::SetComponent(eEntityComponentType ect, IEntityComponent * pComponent)
{
   Assert(static_cast<int>(ect) < kMaxEntityComponentTypes);
   if (pComponent == NULL)
   {
      return E_POINTER;
   }
   tEntityComponentMap::iterator iter = m_entityComponentMap.find(ect);
   if (iter != m_entityComponentMap.end())
   {
      SafeRelease(m_entityComponentMap[ect]);
   }
   m_entityComponentMap[ect] = CTAddRef(pComponent);
   return S_OK;
}

///////////////////////////////////////

tResult cEntity::GetComponent(eEntityComponentType ect, IEntityComponent * * ppComponent)
{
   Assert(static_cast<int>(ect) < kMaxEntityComponentTypes);
   if (ppComponent == NULL)
   {
      return E_POINTER;
   }
   tEntityComponentMap::iterator iter = m_entityComponentMap.find(ect);
   if (iter == m_entityComponentMap.end())
   {
      return S_FALSE;
   }
   *ppComponent = CTAddRef(iter->second);
   return S_OK;
}

///////////////////////////////////////

tResult cEntity::RemoveComponent(eEntityComponentType ect)
{
   tEntityComponentMap::iterator iter = m_entityComponentMap.find(ect);
   if (iter == m_entityComponentMap.end())
   {
      return S_FALSE;
   }
   iter->second->Release();
   m_entityComponentMap.erase(iter);
   return S_OK;
}

///////////////////////////////////////

tResult cEntity::EnumComponents(REFGUID iid, IEnumEntityComponents * * ppEnum)
{
   if (ppEnum == NULL)
   {
      return E_POINTER;
   }
   tEntityComponentList components;
   tEntityComponentMap::iterator iter = m_entityComponentMap.begin(), end = m_entityComponentMap.end();
   for (; iter != end; ++iter)
   {
      cAutoIPtr<IUnknown> pUnkQuery;
      if (iter->second->QueryInterface(iid, (void**)&pUnkQuery) == S_OK)
      {
         // Do not AddRef.  It will be done by cComEnum::Create.
         components.push_back(iter->second);
      }
   }
   return tEntityComponentListEnum::Create(components, ppEnum);
}

///////////////////////////////////////

void cEntity::RemoveAllComponents()
{
   tEntityComponentMap::iterator iter = m_entityComponentMap.begin(), end = m_entityComponentMap.end();
   for (; iter != end; ++iter)
   {
      SafeRelease(iter->second);
   }
   m_entityComponentMap.clear();
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

namespace
{
   GUID IID_IEntityTestComponent1 =
   { 0x4b665744, 0xae4, 0x4907, { 0xa4, 0xcc, 0xd6, 0xbf, 0xcd, 0x56, 0xdf, 0x46 } };
   interface IEntityTestComponent1 : IEntityComponent {};

   GUID IID_IEntityTestComponent2 = 
   { 0x20dfe10e, 0xc1f6, 0x4c1d, { 0x83, 0x53, 0x57, 0xd8, 0xad, 0xba, 0x27, 0xc0 } };
   interface IEntityTestComponent2 : IEntityComponent {};

   class cTestEntityComponent1 : public cComObject<IMPLEMENTS(IEntityTestComponent1)> {};
   class cTestEntityComponent2 : public cComObject<IMPLEMENTS(IEntityTestComponent2)> {};
}

TEST(EntityComponentBasics)
{
   cAutoIPtr<IEntity> pTestEntity(static_cast<IEntity*>(new cEntity(_T("TestEntity"), 0)));

   cAutoIPtr<IEntityComponent> pTestComponent(static_cast<IEntityComponent*>(new cTestEntityComponent1));

   cAutoIPtr<IEntityComponent> pGetComponent;

   CHECK_EQUAL(S_FALSE, pTestEntity->GetComponent(kECT_Custom1, &pGetComponent));
   CHECK_EQUAL(S_OK, pTestEntity->SetComponent(kECT_Custom1, pTestComponent));
   CHECK_EQUAL(S_OK, pTestEntity->GetComponent(kECT_Custom1, &pGetComponent));
   CHECK(CTIsSameObject(pTestComponent, pGetComponent));
}

TEST(RemoveEntityComponent)
{
   cAutoIPtr<IEntity> pTestEntity(static_cast<IEntity*>(new cEntity(_T("TestEntity"), 0)));

   cAutoIPtr<IEntityComponent> pTestComponent(static_cast<IEntityComponent*>(new cTestEntityComponent1));

   CHECK_EQUAL(S_OK, pTestEntity->SetComponent(kECT_Custom1, pTestComponent));
   CHECK_EQUAL(S_FALSE, pTestEntity->RemoveComponent(kECT_Custom2));
   CHECK_EQUAL(S_OK, pTestEntity->RemoveComponent(kECT_Custom1));
}

TEST(EntityComponentEnumeration)
{
   cAutoIPtr<cEntity> pTestEntity(new cEntity(_T("TestEntity"), 0));

   cAutoIPtr<IEntityComponent> pTestComponent1(static_cast<IEntityComponent*>(new cTestEntityComponent1));
   cAutoIPtr<IEntityComponent> pTestComponent2(static_cast<IEntityComponent*>(new cTestEntityComponent2));

   CHECK_EQUAL(S_OK, pTestEntity->SetComponent(kECT_Custom1, pTestComponent1));
   CHECK_EQUAL(S_OK, pTestEntity->SetComponent(kECT_Custom2, pTestComponent2));

   {
      cAutoIPtr<IEnumEntityComponents> pEnum;
      CHECK_EQUAL(S_OK, pTestEntity->EnumComponents(IID_IEntityTestComponent1, &pEnum));

      IEntityComponent * pGetComponents[32];
      ulong count = 0;

      CHECK(SUCCEEDED(pEnum->Next(_countof(pGetComponents), &pGetComponents[0], &count)));
      CHECK_EQUAL(1, static_cast<long>(count));
      CHECK(CTIsSameObject(pTestComponent1, pGetComponents[0]));
      SafeRelease(pGetComponents[0]);
   }

   {
      cAutoIPtr<IEnumEntityComponents> pEnum;
      CHECK_EQUAL(S_OK, pTestEntity->EnumComponents(IID_IUnknown, &pEnum));

      IEntityComponent * pGetComponents[32];
      ulong count = 0;

      CHECK(SUCCEEDED(pEnum->Next(_countof(pGetComponents), &pGetComponents[0], &count)));
      CHECK_EQUAL(2, static_cast<long>(count));
      CHECK(CTIsSameObject(pTestComponent1, pGetComponents[0]) || CTIsSameObject(pTestComponent2, pGetComponents[0]));
      CHECK(CTIsSameObject(pTestComponent1, pGetComponents[1]) || CTIsSameObject(pTestComponent2, pGetComponents[1]));
      CHECK(!CTIsSameObject(pGetComponents[0], pGetComponents[1]));
      SafeRelease(pGetComponents[0]);
      SafeRelease(pGetComponents[1]);
   }
}

#endif // HAVE_UNITTESTPP

///////////////////////////////////////////////////////////////////////////////
