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
   cEntityComponentFactory(tEntityComponentFactoryFn factoryFn, void * userData)
      : m_factoryFn(factoryFn)
      , m_userData(userData)
   {
   }

   virtual tResult CreateComponent(const TiXmlElement * pTiXmlElement, IEntity * pEntity, IEntityComponent * * ppComponent)
   {
      return (*m_factoryFn)(pTiXmlElement, pEntity, m_userData, ppComponent);
   }

private:
   tEntityComponentFactoryFn m_factoryFn;
   void * m_userData;
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
                                            tEntityComponentFactoryFn pfnFactory, void * pUser);
   virtual tResult RevokeComponentFactory(const tChar * pszComponent);
   virtual tResult CreateComponent(const TiXmlElement * pTiXmlElement, IEntity * pEntity,
                                   IEntityComponent * * ppComponent);

private:
   typedef std::map<cStr, IEntityComponentFactory*> tComponentFactoryMap;
   tComponentFactoryMap m_componentFactoryMap;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYCOMPONENTREGISTRY_H
