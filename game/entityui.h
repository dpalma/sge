///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYUI_H
#define INCLUDED_ENTITYUI_H

#include "guiapi.h"
#include "entityapi.h"

#include <set>

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IGUIElement);

///////////////////////////////////////////////////////////////////////////////

typedef std::set<IGUIElement *, cCTLessInterface> tGUIElementSet;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityUI
//

class cEntityUI : public cComObject2<IMPLEMENTS(IEntityManagerListener),
                                     IMPLEMENTS(IGUIEventListener)>
{
public:
   cEntityUI();
   ~cEntityUI();

   virtual void OnEntitySelectionChange();

   virtual tResult OnEvent(IGUIEvent * pEvent);

   void ClearGUIElements();

private:
   tGUIElementSet m_guiElements;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYUI_H
