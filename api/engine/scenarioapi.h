////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCENARIOAPI_H
#define INCLUDED_SCENARIOAPI_H

#include "enginedll.h"

#include "tech/comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE_GUID(IScenario, "B29D0BF0-171F-4826-85EA-E8EA7CB61E11");


////////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IScenario
//

interface IScenario : IUnknown
{
   virtual tResult Start(const tChar * pszMap) = 0;
   virtual tResult Stop() = 0;
};

///////////////////////////////////////

ENGINE_API tResult ScenarioCreate();


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCENARIOAPI_H
