///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIELEMENTENUM_H
#define INCLUDED_GUIELEMENTENUM_H

#include "guielementapi.h"

#include "comenum.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

typedef std::list<IGUIElement *> tGUIElementList;
tResult GUIElementEnumCreate(const tGUIElementList & elements, IGUIElementEnum * * ppEnum);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIELEMENTENUM_H
