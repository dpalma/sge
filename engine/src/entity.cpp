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

tResult ModelEntityCreate(tEntityId id, const tVec3 & position, IEntity * * ppEntity)
{
   if (ppEntity == NULL)
   {
      return E_POINTER;
   }

   cEntity * pEntity = new cEntity(id, position);
   if (pEntity == NULL)
   {
      return E_OUTOFMEMORY;
   }

   *ppEntity = static_cast<IEntity*>(pEntity);
   return S_OK;
}

///////////////////////////////////////

cEntity::cEntity(tEntityId id, const tVec3 & position)
 : m_id(id)
 , m_position(position)
 , m_bUpdateWorldTransform(true)
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

tEntityId cEntity::GetId() const
{
   return m_id;
}

///////////////////////////////////////

tResult cEntity::GetPosition(tVec3 * pPosition) const
{
   if (pPosition == NULL)
   {
      return E_POINTER;
   }
   *pPosition = m_position;
   return S_OK;
}

///////////////////////////////////////

const tMatrix4 & cEntity::GetWorldTransform() const
{
   if (m_bUpdateWorldTransform)
   {
      m_bUpdateWorldTransform = false;
      MatrixTranslate(m_position.x, m_position.y, m_position.z, &m_worldTransform);
   }
   return m_worldTransform;
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
