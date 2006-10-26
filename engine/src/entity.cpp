///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entity.h"

#include "tech/color.h"
#include "tech/globalobj.h"
#include "tech/ray.h"
#include "tech/resourceapi.h"

#include "tech/dbgalloc.h" // must be last header

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
   SafeRelease(m_components[ect]);
   m_components[ect] = CTAddRef(pComponent);
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
   return m_components[ect].GetPointer(ppComponent);
}


///////////////////////////////////////////////////////////////////////////////
