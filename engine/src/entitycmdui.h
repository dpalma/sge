////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYCMDUI_H
#define INCLUDED_ENTITYCMDUI_H

#include "entityapi.h"
#include "guiapi.h"

#include "globalobjdef.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityCmdInfo
//

class cEntityCmdInfo
{
public:
   cEntityCmdInfo();
   cEntityCmdInfo(const tChar * pszImage, const tChar * pszTooltip, tEntityCmdInstance cmdInst);
   cEntityCmdInfo(const cEntityCmdInfo & other);

   const cEntityCmdInfo operator =(const cEntityCmdInfo & other);

   inline const tChar * GetImage() const { return m_image.c_str(); }
   inline const tChar * GetToolTip() const { return m_toolTip.c_str(); }
   inline tEntityCmdInstance GetCmdInstance() const { return m_cmdInst; }

private:
   cStr m_image, m_toolTip;
   tEntityCmdInstance m_cmdInst;
};


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
   static tResult EntityCmdUIComponentFactory(const TiXmlElement * pTiXmlElement,
      IEntity * pEntity, void * pUser, IEntityComponent * * ppComponent);

   cStr m_entityPanelId;

   typedef std::multimap<cStr, cEntityCmdInfo> tEntityTypeCmdMap;
   tEntityTypeCmdMap m_entityTypeCmdMap;

   cAutoIPtr<IEntity> m_pTargetEntity;

   typedef std::map<cStr, tEntityCmdInstance> tActiveCmdMap;
   tActiveCmdMap m_activeCmds;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYCMDUI_H
