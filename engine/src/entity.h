////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITY_H
#define INCLUDED_ENTITY_H

#include "entityapi.h"

#include "techstring.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntity
//

class cEntity : public cComObject<IMPLEMENTS(IEntity)>
{
public:
   cEntity(tEntityId id, const tVec3 & position);
   ~cEntity();

   virtual tEntityId GetId() const;

   virtual tResult GetPosition(tVec3 * pPosition) const;

   virtual const tMatrix4 & GetWorldTransform() const;

   virtual tResult AddComponent(REFGUID guid, IEntityComponent * pComponent);
   virtual tResult FindComponent(REFGUID guid, IEntityComponent * * ppComponent);

private:
   tEntityId m_id;

   tVec3 m_position;

   mutable bool m_bUpdateWorldTransform;
   mutable tMatrix4 m_worldTransform;

   struct sLessGuid
   {
      bool operator()(const GUID * pLhs, const GUID * pRhs) const
      {
         return (memcmp(pLhs, pRhs, sizeof(GUID)) < 0);
      }
   };

   typedef std::map<const GUID *, IEntityComponent *, sLessGuid> tComponentMap;
   tComponentMap m_componentMap;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITY_H
