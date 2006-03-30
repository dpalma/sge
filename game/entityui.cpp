///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entityui.h"
#include "script.h"

#include "guielementapi.h"

#include "globalobj.h"
#include "multivar.h"

#include <algorithm>

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
      cAutoIPtr<IEntityEnum> pEnum;
      if (pEntityManager->GetSelected(&pEnum) == S_OK)
      {
         cAutoIPtr<IEntity> pEntity;
         ulong nEntities = 0;
         if (pEnum->Next(1, &pEntity, &nEntities) == S_OK && nEntities == 1)
         {
            cAutoIPtr<IEntitySpawnComponent> pSpawn;
            if (pEntity->FindComponent(IID_IEntitySpawnComponent, &pSpawn) == S_OK)
            {
               // TODO:
               //cAutoIPtr<IGUIButtonElement> pButton;
               //if (GUIButtonCreate(&pButton) == S_OK)
               //{
               //   pButton->SetText(model.c_str());
               //   pContainer->AddElement(pButton);
               //   m_guiElements.insert(CTAddRef(pButton));
               //   pGUIContext->RequestLayout(pContainer);
               //}
            }
         }
      }
   }
}

////////////////////////////////////////

tResult cEntityUI::OnEvent(IGUIEvent * pEvent)
{
   if (pEvent)
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
