///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entity.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include "tech/dbgalloc.h" // must be last header


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
   class cTestEntityComponent : public cComObject<IMPLEMENTS(IEntityComponent)> {};
}

TEST(EntityComponentBasics)
{
   cAutoIPtr<IEntity> pTestEntity(static_cast<IEntity*>(new cEntity(_T("TestEntity"), 0)));

   cAutoIPtr<IEntityComponent> pTestComponent(static_cast<IEntityComponent*>(new cTestEntityComponent));

   cAutoIPtr<IEntityComponent> pGetComponent;

   CHECK_EQUAL(S_FALSE, pTestEntity->GetComponent(kECT_Custom1, &pGetComponent));
   CHECK_EQUAL(S_OK, pTestEntity->SetComponent(kECT_Custom1, pTestComponent));
   CHECK_EQUAL(S_OK, pTestEntity->GetComponent(kECT_Custom1, &pGetComponent));
   CHECK(CTIsSameObject(pTestComponent, pGetComponent));
}

#endif // HAVE_UNITTESTPP

///////////////////////////////////////////////////////////////////////////////
