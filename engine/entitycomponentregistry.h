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

   typedef std::map<cStr, sRegisteredComponentFactory> tComponentFactoryMap;
   tComponentFactoryMap m_componentFactoryMap;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYCOMPONENTREGISTRY_H
