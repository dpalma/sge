///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guielementenum.h"

#include "guiapi.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

class cGUIElementEnum : public cComObject<cComEnum<IGUIElementEnum,
                                                   &IID_IGUIElementEnum,
                                                   IGUIElement*,
                                                   CopyInterface<IGUIElement>,
                                                   tGUIElementList>,
                                          &IID_IGUIElementEnum>
{
};

tResult GUIElementEnumCreate(const tGUIElementList & elements, IGUIElementEnum * * ppEnum)
{
   return cGUIElementEnum::Create(elements, ppEnum);
}

///////////////////////////////////////////////////////////////////////////////
