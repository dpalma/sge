////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYFACTORY_H
#define INCLUDED_ENTITYFACTORY_H

#include "engine/entityapi.h"

#include "tech/globalobjdef.h"

#ifdef _MSC_VER
#pragma once
#endif

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityFactory
//

class cEntityFactory : public cComObject2<IMPLEMENTS(IEntityFactory),
                                          IMPLEMENTS(IGlobalObject)>
{
public:
   cEntityFactory();
   ~cEntityFactory();

   DECLARE_NAME(EntityFactory)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult CreateEntity(IEntity * * ppEntity);
   virtual tResult CreateEntity(const tChar * pszEntityType, IEntity * * ppEntity);

private:
   tEntityId m_nextId;
};

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYFACTORY_H
