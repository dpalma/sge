///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guielementenum.h"

#include "guiapi.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

tResult GUIElementEnumCreate(const tGUIElementList & elements, IGUIElementEnum * * ppEnum)
{
   typedef cComObject<cComEnum<IGUIElementEnum, &IID_IGUIElementEnum, IGUIElement*, CopyInterface<IGUIElement>, tGUIElementList>, &IID_IGUIElementEnum> Class;
   return Class::Create(elements, ppEnum);
}

///////////////////////////////////////////////////////////////////////////////
