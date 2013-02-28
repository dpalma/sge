////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCENARIO_H
#define INCLUDED_SCENARIO_H

#include "engine/scenarioapi.h"
#include "engine/saveloadapi.h"

#include "tech/globalobjdef.h"
#include "tech/techmath.h"

#ifdef _MSC_VER
#pragma once
#endif


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cScenario
//

class cScenario : public cComObject3<IMPLEMENTS(IScenario),
                                     IMPLEMENTS(ISaveLoadListener),
                                     IMPLEMENTS(IGlobalObject)>
{
   friend void * ScenarioMapLoad(IReader * pReader, void * typeParam);
   friend void ScenarioMapUnload(void * pData);

public:
   cScenario();
   ~cScenario();

   DECLARE_NAME(Scenario)
   DECLARE_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   // IScenario
   virtual tResult Start(const tChar * pszMap, const tChar * pszGUI, ulong randSeed);
   virtual tResult Stop();

   virtual ulong NextRand();
   virtual float NextRandFloat();

   // ISaveLoadListener methods
   virtual void OnBeginSave();
   virtual void OnSaveProgress(uint current, uint bound);
   virtual void OnEndSave();

   virtual void OnBeginLoad();
   virtual void OnLoadProgress(uint current, uint bound);
   virtual void OnEndLoad();

private:
   cStr m_map;

   cRand m_rand;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCENARIO_H
