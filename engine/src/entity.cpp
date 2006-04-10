///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entity.h"

#include "color.h"
#include "globalobj.h"
#include "ray.h"
#include "resourceapi.h"

#include "dbgalloc.h" // must be last header

#define IsFlagSet(f, b) (((f)&(b))==(b))


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
   tComponentMap::iterator iter = m_componentMap.begin();
   for (; iter != m_componentMap.end(); iter++)
   {
      iter->second->Release();
   }
   m_componentMap.clear();
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

tResult cEntity::AddComponent(REFGUID guid, IEntityComponent * pComponent)
{
   if (pComponent == NULL)
   {
      return E_POINTER;
   }

   std::pair<tComponentMap::iterator, bool> result = m_componentMap.insert(
      std::make_pair(&guid, CTAddRef(pComponent)));

   return result.second ? S_OK : S_FALSE;
}

///////////////////////////////////////

tResult cEntity::FindComponent(REFGUID guid, IEntityComponent * * ppComponent)
{
   if (ppComponent == NULL)
   {
      return E_POINTER;
   }

   tComponentMap::iterator f = m_componentMap.find(&guid);
   if (f == m_componentMap.end())
   {
      return S_FALSE;
   }

   *ppComponent = CTAddRef(f->second);
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
