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
                                            tEntityComponentFactoryFn pfnFactory, void * pUser);
   virtual tResult RevokeComponentFactory(const tChar * pszComponent);
   virtual tResult CreateComponent(const TiXmlElement * pTiXmlElement, IEntity * pEntity,
                                   IEntityComponent * * ppComponent);

private:
   typedef std::pair<tEntityComponentFactoryFn, void*> tComponentFactoryDataPair;
   typedef std::map<cStr, tComponentFactoryDataPair> tComponentFactoryMap;
   tComponentFactoryMap m_componentFactoryMap;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYCOMPONENTREGISTRY_H
