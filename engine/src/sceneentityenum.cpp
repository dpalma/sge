///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "sceneentityenum.h"

#include <algorithm>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneEntityEnum
//

///////////////////////////////////////

ISceneEntityEnum * SceneEntityEnumCreate(const tSceneEntityList & entities)
{
   return static_cast<ISceneEntityEnum *>(new cSceneEntityEnum(entities));
}

///////////////////////////////////////

cSceneEntityEnum::cSceneEntityEnum(const tSceneEntityList & entities)
 : m_entities(entities.size())
{
   std::copy(entities.begin(), entities.end(), m_entities.begin());
   std::for_each(m_entities.begin(), m_entities.end(), CTInterfaceMethodRef(&IUnknown::AddRef));
   m_iterator = m_entities.begin();
}

///////////////////////////////////////

cSceneEntityEnum::~cSceneEntityEnum()
{
   std::for_each(m_entities.begin(), m_entities.end(), CTInterfaceMethodRef(&IUnknown::Release));
   m_entities.clear();
   m_iterator = m_entities.end();
}

///////////////////////////////////////

tResult cSceneEntityEnum::Next(ulong count, ISceneEntity * * ppEntities, ulong * pnEntities)
{
   Assert(ppEntities != NULL && pnEntities != NULL);

   ulong nReturned = 0;

   for (uint i = 0; i < count; i++)
   {
      if (m_iterator == m_entities.end())
      {
         break;
      }
      else
      {
         *ppEntities = CTAddRef(*m_iterator);
         nReturned++;
         m_iterator++;
         ppEntities++;
      }
   }

   if (pnEntities != NULL)
   {
      *pnEntities = nReturned;
   }

   return (nReturned == count) ? S_OK : S_FALSE;
}

///////////////////////////////////////

tResult cSceneEntityEnum::Skip(ulong count)
{
   uint nSkipped = 0;
   while (count-- && (m_iterator != m_entities.end()))
   {
      nSkipped++;
      m_iterator++;
   }
   return (nSkipped == count) ? S_OK : S_FALSE;
}

///////////////////////////////////////

tResult cSceneEntityEnum::Reset()
{
   m_iterator = m_entities.begin();
   return S_OK;
}

///////////////////////////////////////

tResult cSceneEntityEnum::Clone(ISceneEntityEnum * * ppEnum)
{
   Assert(ppEnum != NULL);
   *ppEnum = SceneEntityEnumCreate(m_entities);
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
