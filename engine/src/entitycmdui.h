////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYCMDUI_H
#define INCLUDED_ENTITYCMDUI_H

#include "entityapi.h"
#include "guiapi.h"

#include "globalobjdef.h"

#ifdef _MSC_VER
#pragma once
#endif


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityCmdUI
//

class cEntityCmdUI : public cComObject4<IMPLEMENTS(IEntityCommandUI),
                                        IMPLEMENTS(IGlobalObject),
                                        IMPLEMENTS(IEntityManagerListener),
                                        IMPLEMENTS(IGUIEventListener)>
{
public:
   cEntityCmdUI();
   ~cEntityCmdUI();

   DECLARE_NAME(EntityCmdUI)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual void OnEntitySelectionChange();

   virtual tResult OnEvent(IGUIEvent * pEvent);

   virtual tResult SetEntityPanelId(const tChar * pszId);
   virtual tResult GetEntityPanelId(cStr * pId);

private:
   cStr m_entityPanelId;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYCMDUI_H
