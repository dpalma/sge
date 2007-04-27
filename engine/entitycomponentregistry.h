////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYCOMPONENTREGISTRY_H
#define INCLUDED_ENTITYCOMPONENTREGISTRY_H

#include "engine/entityapi.h"

#include "tech/globalobjdef.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif


class cEntityComponentFactory : public cComObject<IMPLEMENTS(IEntityComponentFactory)>
{
public:
   cEntityComponentFactory(tEntityComponentFactoryFn factoryFn)
      : m_factoryFn(factoryFn)
   {
   }

   virtual tResult CreateComponent(const TiXmlElement * pTiXmlElement, IEntity * pEntity, IEntityComponent * * ppComponent)
   {
      return (*m_factoryFn)(pTiXmlElement, pEntity, ppComponent);
   }

private:
   tEntityComponentFactoryFn m_factoryFn;
};


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityComponentRegistry
//

class cEntityComponentRegistry : public cComObject2<IMPLEMENTS(IEntityComponentRegistry),
                                                    IMPLEMENTS(IGlobalObject)>
{
public:
   cEntityComponentRegistry();
   ~cEntityComponentRegistry();

   DECLARE_NAME(EntityComponentRegistry)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult RegisterComponentFactory(const tChar * pszComponent,
                                            IEntityComponentFactory * pFactory);
   virtual tResult RegisterComponentFactory(const tChar * pszComponent,
                                            tEntityComponentFactoryFn pfnFactory);
   virtual tResult RevokeComponentFactory(const tChar * pszComponent);
   virtual tResult CreateComponent(const TiXmlElement * pTiXmlElement, IEntity * pEntity,
                                   IEntityComponent * * ppComponent);

private:
   typedef std::map<cStr, IEntityComponentFactory*> tComponentFactoryMap;
   tComponentFactoryMap m_componentFactoryMap;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYCOMPONENTREGISTRY_H
