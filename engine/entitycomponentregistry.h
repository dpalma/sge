////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYCOMPONENTREGISTRY_H
#define INCLUDED_ENTITYCOMPONENTREGISTRY_H

#include "engine/entityapi.h"

#include "tech/globalobjdef.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>

#ifdef _MSC_VER
#pragma once
#endif

struct sRegisteredComponentFactory
{
   cStr name;
   tEntityComponentID cid;
   IEntityComponentFactory * pFactory;
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

   void RevokeAll();

   virtual tResult RegisterComponentFactory(const tChar * pszComponent,
                                            IEntityComponentFactory * pFactory);
   virtual tResult RevokeComponentFactory(const tChar * pszComponent);

   tResult CreateComponent(const tChar * pszComponent, const TiXmlElement * pTiXmlElement,
      IEntity * pEntity, IEntityComponent * * ppComponent);

   virtual tResult CreateComponent(const tChar * pszComponent, IEntity * pEntity,
                                   IEntityComponent * * ppComponent);
   virtual tResult CreateComponent(const TiXmlElement * pTiXmlElement, IEntity * pEntity,
                                   IEntityComponent * * ppComponent);

private:
   tResult FindFactory(const tChar * pszComponent, IEntityComponentFactory * * ppFactory);

   struct name {};

   typedef boost::multi_index_container<
      sRegisteredComponentFactory,
      boost::multi_index::indexed_by<
         boost::multi_index::hashed_unique<
            boost::multi_index::tag<name>,
            boost::multi_index::member<sRegisteredComponentFactory, cStr, &sRegisteredComponentFactory::name>
         >
      >
   > tComponentFactoryContainer;
   tComponentFactoryContainer m_componentFactoryContainer;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYCOMPONENTREGISTRY_H
