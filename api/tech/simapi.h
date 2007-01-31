///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SIMAPI_H
#define INCLUDED_SIMAPI_H

#include "techdll.h"
#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE_GUID(ISim, "E0E7065A-9C21-4c01-8509-8C7299BEC6EF");
F_DECLARE_INTERFACE_GUID(ISimClient, "B3E05500-6AB3-4182-BF64-319EE6D4D5DC");


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ISim
//

interface ISim : IUnknown
{
	virtual tResult Start() = 0;
	virtual tResult Stop() = 0;
	virtual bool IsRunning() const = 0;

   virtual double GetTimeScale() const = 0;
   virtual void SetTimeScale(double timeScale) = 0;

	virtual tResult AddSimClient(ISimClient * pSimClient) = 0;
	virtual tResult RemoveSimClient(ISimClient * pSimClient) = 0;
};

///////////////////////////////////////

#define kSimName "Sim"
TECH_API tResult SimCreate();


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ISimClient
//

interface ISimClient : IUnknown
{
   virtual tResult Execute(double time) = 0;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SIMAPI_H
