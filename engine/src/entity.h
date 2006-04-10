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
   cEntity(const tChar * pszTypeName, tEntityId id);
   ~cEntity();

   virtual tResult GetTypeName(cStr * pTypeName) const;

   virtual tEntityId GetId() const;

   virtual tResult AddComponent(REFGUID guid, IEntityComponent * pComponent);
   virtual tResult FindComponent(REFGUID guid, IEntityComponent * * ppComponent);

private:
   cStr m_typeName;
   tEntityId m_id;

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
