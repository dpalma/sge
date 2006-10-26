////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITY_H
#define INCLUDED_ENTITY_H

#include "engine/entityapi.h"

#include "tech/techstring.h"

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

   virtual tResult SetComponent(eEntityComponentType ect, IEntityComponent * pComponent);
   virtual tResult GetComponent(eEntityComponentType ect, IEntityComponent * * ppComponent);

private:
   cStr m_typeName;
   tEntityId m_id;
   cAutoIPtr<IEntityComponent> m_components[kMaxEntityComponentTypes];
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITY_H
