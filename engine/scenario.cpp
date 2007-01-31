////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "scenario.h"

#include "engine/engineapi.h"
#include "gui/guiapi.h"
#include "tech/resourceapi.h"
#include "tech/simapi.h"
#include "tech/techtime.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(Scenario);

#define LocalMsg(msg)            DebugMsgEx(Scenario,msg)
#define LocalMsg1(msg,a)         DebugMsgEx1(Scenario,msg,(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(Scenario,msg,(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(Scenario,msg,(a),(b),(c))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(Scenario,(cond),msg)
#define LocalMsgIf1(cond,msg,a)        DebugMsgIfEx1(Scenario,(cond),msg,(a))
#define LocalMsgIf2(cond,msg,a,b)      DebugMsgIfEx2(Scenario,(cond),msg,(a),(b))
#define LocalMsgIf3(cond,msg,a,b,c)    DebugMsgIfEx3(Scenario,(cond),msg,(a),(b),(c))


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

tResult cScenario::Start(const tChar * pszMap, const tChar * pszGUI, ulong randSeed)
{
   if (pszMap == NULL)
   {
      return E_POINTER;
   }

   if (randSeed == 0)
   {
      randSeed = static_cast<ulong>(ReadTSC());
   }

   m_rand.Seed(randSeed);

   void * pData = NULL;
   UseGlobal(ResourceManager);
   if (pResourceManager->Load(pszMap, kRT_Map, NULL, &pData) != S_OK)
   {
      return E_FAIL;
   }

   if (pszGUI != NULL)
   {
      UseGlobal(GUIContext);
      pGUIContext->PushPage(pszGUI);
   }

   UseGlobal(Sim);
   pSim->Start();

   return S_OK;
}

////////////////////////////////////////

tResult cScenario::Stop()
{
   UseGlobal(Sim);
   pSim->Stop();

   UseGlobal(SaveLoadManager);
   pSaveLoadManager->Reset();

   UseGlobal(GUIContext);
   pGUIContext->PopPage();

   return S_OK;
}

////////////////////////////////////////

ulong cScenario::NextRand()
{
   return m_rand.Next();
}

////////////////////////////////////////

float cScenario::NextRandFloat()
{
   return m_rand.NextFloat();
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
   LocalMsg2("Load progress: %d/%d\n", current, bound);
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
