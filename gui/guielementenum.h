///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIELEMENTENUM_H
#define INCLUDED_GUIELEMENTENUM_H

#include "gui/guiapi.h"
#include "gui/guielementapi.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

typedef std::list<IGUIElement *> tGUIElementList;
tResult GUIElementEnumCreate(const tGUIElementList & elements, IGUIElementEnum * * ppEnum);


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIELEMENTENUM_H
