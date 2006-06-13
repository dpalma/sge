///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entitycmdui.h"

#include "guielementapi.h"

#include "multivar.h"

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityCmdUI
//

////////////////////////////////////////

cEntityCmdUI::cEntityCmdUI()
{
}

////////////////////////////////////////

cEntityCmdUI::~cEntityCmdUI()
{
}

////////////////////////////////////////

tResult cEntityCmdUI::Init()
{
   UseGlobal(EntityManager);
   pEntityManager->AddEntityManagerListener(static_cast<IEntityManagerListener*>(this));

   UseGlobal(GUIContext);
   pGUIContext->AddEventListener(static_cast<IGUIEventListener*>(this));

   return S_OK;
}

////////////////////////////////////////

tResult cEntityCmdUI::Term()
{
   UseGlobal(GUIContext);
   pGUIContext->RemoveEventListener(static_cast<IGUIEventListener*>(this));

   UseGlobal(EntityManager);
   pEntityManager->RemoveEntityManagerListener(static_cast<IEntityManagerListener*>(this));

   return S_OK;
}

////////////////////////////////////////

void cEntityCmdUI::OnEntitySelectionChange()
{
   //ClearGUIElements();

   UseGlobal(GUIContext);
   cAutoIPtr<IGUIElement> pPanelElement;
   cAutoIPtr<IGUIContainerElement> pContainer;
   if (pGUIContext->GetElementById(m_entityPanelId.c_str(), &pPanelElement) == S_OK)
   {
      if (pPanelElement->QueryInterface(IID_IGUIContainerElement, (void**)&pContainer) == S_OK)
      {
         pContainer->RemoveAll();
         pGUIContext->RequestLayout(pContainer);
      }
   }

   if (!pContainer)
   {
      return;
   }

   UseGlobal(EntityManager);
   if (pEntityManager->GetSelectedCount() == 1)
   {
      cAutoIPtr<IEnumEntities> pEnum;
      if (pEntityManager->GetSelected(&pEnum) == S_OK)
      {
         cAutoIPtr<IEntity> pEntity;
         ulong nEntities = 0;
         if (pEnum->Next(1, &pEntity, &nEntities) == S_OK && nEntities == 1)
         {
            cAutoIPtr<IEntitySpawnComponent> pSpawn;
            if (pEntity->GetComponent(kECT_Spawn, IID_IEntitySpawnComponent, &pSpawn) == S_OK)
            {
               size_t nSpawnTypes = pSpawn->GetSpawnTypeCount();
               for (uint i = 0; i < nSpawnTypes; i++)
               {
                  cStr spawnType;
                  if (pSpawn->GetSpawnType(i, &spawnType) != S_OK)
                  {
                     continue;
                  }

                  // TODO:
                  //cAutoIPtr<IGUIButtonElement> pButton;
                  //if (GUIButtonCreate(&pButton) == S_OK)
                  //{
                  //   pButton->SetText(spawnType.c_str());
                  //   pContainer->AddElement(pButton);
                  //   m_guiElements.insert(CTAddRef(pButton));
                  //}
               }

               pGUIContext->RequestLayout(pContainer);
            }
         }
      }
   }
}

////////////////////////////////////////

tResult cEntityCmdUI::OnEvent(IGUIEvent * pEvent)
{
   return S_OK;
}

////////////////////////////////////////

tResult cEntityCmdUI::SetEntityPanelId(const tChar * pszId)
{
   if (pszId == NULL)
   {
      m_entityPanelId.clear();
   }
   else
   {
      m_entityPanelId.assign(pszId);
   }
   return S_OK;
}

////////////////////////////////////////

tResult cEntityCmdUI::GetEntityPanelId(cStr * pId)
{
   if (pId == NULL)
   {
      return E_POINTER;
   }
   if (m_entityPanelId.empty())
   {
      return S_FALSE;
   }
   pId->assign(m_entityPanelId);
   return S_OK;
}

////////////////////////////////////////

tResult EntityCommandUICreate()
{
   cAutoIPtr<IEntityCommandUI> p(static_cast<IEntityCommandUI*>(new cEntityCmdUI));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IEntityCommandUI, p);
}

///////////////////////////////////////////////////////////////////////////////
