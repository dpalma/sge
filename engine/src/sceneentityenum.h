///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCENEENTITYENUM_H
#define INCLUDED_SCENEENTITYENUM_H

#include "sceneapi.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneEntityEnum
//

typedef std::list<ISceneEntity *> tSceneEntityList;

class cSceneEntityEnum : public cComObject<IMPLEMENTS(ISceneEntityEnum)>
{
   cSceneEntityEnum(const cSceneEntityEnum &);
   const cSceneEntityEnum & operator =(const cSceneEntityEnum &);

public:
   cSceneEntityEnum(const tSceneEntityList & entities);
   ~cSceneEntityEnum();

   virtual tResult Next(ulong count, ISceneEntity * * ppEntities, ulong * pnEntities);
   virtual tResult Skip(ulong count);
   virtual tResult Reset();
   virtual tResult Clone(ISceneEntityEnum * * ppEnum);

private:
   tSceneEntityList m_entities;
   tSceneEntityList::iterator m_iterator;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCENEENTITYENUM_H
