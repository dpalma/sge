////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYLIST_H
#define INCLUDED_ENTITYLIST_H

#include "engine/entityapi.h"

#include "tech/comenum.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif

////////////////////////////////////////////////////////////////////////////////

typedef std::list<IEntity *> tEntityList;

typedef cComObject<cComEnum<IEnumEntities,
                            &IID_IEnumEntities,
                            IEntity*,
                            CopyInterface<IEntity>,
                            tEntityList>,
                   &IID_IEnumEntities> tEntityListEnum;

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYLIST_H
