///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIELEMENTENUM_H
#define INCLUDED_GUIELEMENTENUM_H

#include "guiapi.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIElementEnum
//

typedef std::list<IGUIElement *> tGUIElementList;

class cGUIElementEnum : public cComObject<IMPLEMENTS(IGUIElementEnum)>
{
   cGUIElementEnum(const cGUIElementEnum &);
   const cGUIElementEnum & operator =(const cGUIElementEnum &);

   cGUIElementEnum(const tGUIElementList & elements);
   ~cGUIElementEnum();

public:
   friend tResult GUIElementEnumCreate(const tGUIElementList & elements, IGUIElementEnum * * ppEnum);

   virtual tResult Next(ulong count, IGUIElement * * ppElements, ulong * pnElements);
   virtual tResult Skip(ulong count);
   virtual tResult Reset();
   virtual tResult Clone(IGUIElementEnum * * ppEnum);

private:
   tGUIElementList m_elements;
   tGUIElementList::iterator m_iterator;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIELEMENTENUM_H
