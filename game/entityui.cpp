///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entityui.h"
#include "script.h"

#include "guielementapi.h"

#include "globalobj.h"
#include "multivar.h"

#include <algorithm>
#include <map>
#include <vector>

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

static cStr g_entityPanel;

SCRIPT_DEFINE_FUNCTION(SetEntityPanel)
{
   if (argc == 1 && argv[0].IsString())
   {
      g_entityPanel.assign(argv[0].ToString());
   }
   else
   {
      g_entityPanel.clear();
   }
   return 0;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityUI
//

////////////////////////////////////////

void InitEntityUI()
{
   cAutoIPtr<cEntityUI> pEntityUI = new cEntityUI;
   if (!!pEntityUI)
   {
      UseGlobal(EntityManager);
      pEntityManager->AddEntityManagerListener(static_cast<IEntityManagerListener*>(pEntityUI));

      UseGlobal(GUIContext);
      pGUIContext->AddEventListener(static_cast<IGUIEventListener*>(pEntityUI));
   }
}

////////////////////////////////////////

cEntityUI::cEntityUI()
{
}

////////////////////////////////////////

cEntityUI::~cEntityUI()
{
   ClearGUIElements();
}

////////////////////////////////////////

tResult EntityCommandSpawn(IEntity * pEntity, const cMultiVar * pArgs, uint nArgs)
{
   if (pArgs == NULL)
   {
      return E_POINTER;
   }

   if (nArgs != 1 || !pArgs[0].IsString())
   {
      return E_INVALIDARG;
   }

   UseGlobal(EntityManager);
   pEntityManager->SpawnEntity(pArgs[0].ToString(), tVec3());
   return S_OK;
}

tResult EntityCommandSetRallyPoint(IEntity * pEntity, const cMultiVar * pArgs, uint nArgs)
{
   return E_NOTIMPL;
}

static void RegisterBuiltinEntityCommands()
{
   static bool bCalledOnce = false;
   if (!bCalledOnce)
   {
      UseGlobal(EntityCommandManager);
      pEntityCommandManager->RegisterCommand(_T("Spawn"), EntityCommandSpawn);
      pEntityCommandManager->RegisterCommand(_T("SetRallyPoint"), EntityCommandSetRallyPoint);
      bCalledOnce = true;
   }
}

void cEntityUI::OnEntitySelectionChange()
{
   ClearGUIElements();

   UseGlobal(GUIContext);
   cAutoIPtr<IGUIElement> pPanelElement;
   cAutoIPtr<IGUIContainerElement> pContainer;
   if (pGUIContext->GetElementById(g_entityPanel.c_str(), &pPanelElement) == S_OK)
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
                  cAutoIPtr<IGUIButtonElement> pButton;
                  if (GUIButtonCreate(&pButton) == S_OK)
                  {
                     pButton->SetText(spawnType.c_str());
                     pContainer->AddElement(pButton);
//                     m_guiElements.insert(CTAddRef(pButton));
                  }
               }

               pGUIContext->RequestLayout(pContainer);
            }
         }
      }
   }
}

////////////////////////////////////////

tResult cEntityUI::OnEvent(IGUIEvent * pEvent)
{
   if (pEvent != NULL)
   {
      tGUIEventCode eventCode;
      if (pEvent->GetEventCode(&eventCode) == S_OK)
      {
         if (eventCode == kGUIEventClick)
         {
            cAutoIPtr<IGUIElement> pSourceElement;
            if (pEvent->GetSourceElement(&pSourceElement) == S_OK)
            {
            }
         }
      }
   }
   return S_OK;
}

////////////////////////////////////////

void cEntityUI::ClearGUIElements()
{
   std::for_each(m_guiElements.begin(), m_guiElements.end(), CTInterfaceMethod(&IGUIElement::Release));
   m_guiElements.clear();
}

///////////////////////////////////////////////////////////////////////////////
