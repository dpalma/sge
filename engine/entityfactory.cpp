///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entityfactory.h"

#include "engine/engineapi.h"
#include "tech/resourceapi.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include <tinyxml.h>

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

extern tResult EntityCreate(const tChar * pszTypeName, tEntityId id, IEntity * * ppEntity);


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityFactory
//

///////////////////////////////////////

cEntityFactory::cEntityFactory()
 : m_nextId(0)
{
}

///////////////////////////////////////

cEntityFactory::~cEntityFactory()
{
}

///////////////////////////////////////

tResult cEntityFactory::Init()
{
   return S_OK;
}

///////////////////////////////////////

tResult cEntityFactory::Term()
{
   return S_OK;
}

///////////////////////////////////////

tResult cEntityFactory::CreateEntity(IEntity * * ppEntity)
{
   if (ppEntity == NULL)
   {
      return E_POINTER;
   }

   uint oldNextId = m_nextId;
   uint entityId = ++m_nextId;

   tResult result = E_FAIL;
   cAutoIPtr<IEntity> pEntity;
   if ((result = EntityCreate(_T(""), entityId, &pEntity)) != S_OK)
   {
      m_nextId = oldNextId;
      return result;
   }

   *ppEntity = CTAddRef(pEntity);
   return S_OK;
}

///////////////////////////////////////

tResult cEntityFactory::CreateEntity(const tChar * pszEntityType, IEntity * * ppEntity)
{
   if (pszEntityType == NULL || ppEntity == NULL)
   {
      return E_POINTER;
   }

   const TiXmlDocument * pTiXmlDoc = NULL;

   UseGlobal(ResourceManager);
   if (pResourceManager->Load(pszEntityType, kRT_TiXml, NULL, (void**)&pTiXmlDoc) != S_OK)
   {
      return E_FAIL;
   }

   const TiXmlElement * pTiXmlElement = pTiXmlDoc->FirstChildElement();
   if ((pTiXmlElement == NULL) || (_stricmp(pTiXmlElement->Value(), "entity") != 0))
   {
      return E_FAIL;
   }

   WarnMsgIf1(pTiXmlElement->NextSiblingElement() != NULL,
      "There should be only one entity definition per file (%s)\n", pszEntityType);

   uint oldNextId = m_nextId;
   uint entityId = ++m_nextId;

   tResult result = E_FAIL;
   cAutoIPtr<IEntity> pEntity;
   if ((result = EntityCreate(pszEntityType, entityId, &pEntity)) != S_OK)
   {
      m_nextId = oldNextId;
      return result;
   }

   UseGlobal(EntityComponentRegistry);
   if (!!pEntityComponentRegistry)
   {
      for (const TiXmlElement * pTiXmlChild = pTiXmlElement->FirstChildElement();
         pTiXmlChild != NULL; pTiXmlChild = pTiXmlChild->NextSiblingElement())
      {
         Assert(pTiXmlChild->Type() == TiXmlNode::ELEMENT);

         cAutoIPtr<IEntityComponent> pComponent;
         if (pEntityComponentRegistry->CreateComponent(pTiXmlChild, pEntity, &pComponent) != S_OK)
         {
            WarnMsgIf1(pTiXmlChild->Value() != NULL, "Failed to create entity component \"%s\"\n", pTiXmlChild->Value());
         }
      }
   }

   *ppEntity = CTAddRef(pEntity);
   return S_OK;
}

///////////////////////////////////////

tResult EntityFactoryCreate()
{
   cAutoIPtr<IEntityFactory> p(static_cast<IEntityFactory*>(new cEntityFactory));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IEntityFactory, p);
}

///////////////////////////////////////////////////////////////////////////////
