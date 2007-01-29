////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "scenario.h"
#include "engine/engineapi.h"

#include "tech/resourceapi.h"

#include "tech/dbgalloc.h" // must be last header


////////////////////////////////////////////////////////////////////////////////

class cScenarioMapToken { private: int unused; };

void * ScenarioMapLoad(IReader * pReader, void * typeParam)
{
   cScenario * pScenario = reinterpret_cast<cScenario*>(typeParam);

   UseGlobal(SaveLoadManager);
   if (pSaveLoadManager->Load(pReader) == S_OK)
   {
      return new cScenarioMapToken;
   }

   return NULL;
}

////////////////////////////////////////////////////////////////////////////////

void ScenarioMapUnload(void * pData)
{
   delete (cScenarioMapToken*)pData;
}


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cScenario
//

////////////////////////////////////////

cScenario::cScenario()
{
}

////////////////////////////////////////

cScenario::~cScenario()
{
}

////////////////////////////////////////

BEGIN_CONSTRAINTS(cScenario)
   AFTER_GUID(IID_ISaveLoadManager)
END_CONSTRAINTS()

////////////////////////////////////////

tResult cScenario::Init()
{
   UseGlobal(SaveLoadManager);
   pSaveLoadManager->AddSaveLoadListener(static_cast<ISaveLoadListener*>(this));

   UseGlobal(ResourceManager);
   if (!!pResourceManager)
   {
      if (pResourceManager->RegisterFormat(kRT_Map, kMapExt, ScenarioMapLoad, NULL, ScenarioMapUnload, this) != S_OK)
      {
         return E_FAIL;
      }
   }

   return S_OK;
}

////////////////////////////////////////

tResult cScenario::Term()
{
   UseGlobal(SaveLoadManager);
   pSaveLoadManager->RemoveSaveLoadListener(static_cast<ISaveLoadListener*>(this));

   return S_OK;
}

////////////////////////////////////////

tResult cScenario::Start(const tChar * pszMap)
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cScenario::Stop()
{
   return E_NOTIMPL;
}

////////////////////////////////////////

void cScenario::OnBeginSave()
{
}

////////////////////////////////////////

void cScenario::OnSaveProgress(uint current, uint bound)
{
}

////////////////////////////////////////

void cScenario::OnEndSave()
{
}

////////////////////////////////////////

void cScenario::OnBeginLoad()
{
}

////////////////////////////////////////

void cScenario::OnLoadProgress(uint current, uint bound)
{
}

////////////////////////////////////////

void cScenario::OnEndLoad()
{
}

///////////////////////////////////////////////////////////////////////////////

tResult ScenarioCreate()
{
   cAutoIPtr<IScenario> pScenario(static_cast<IScenario*>(new cScenario));
   if (!pScenario)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IScenario, pScenario);
}

////////////////////////////////////////////////////////////////////////////////
