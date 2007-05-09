///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SIM_H
#define INCLUDED_SIM_H

#include "tech/simapi.h"

#include "tech/connptimpl.h"
#include "tech/globalobjdef.h"
#include "tech/schedulerapi.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSim
//

class cSim : public cComObject3<IMPLEMENTSCP(ISim, ISimClient), IMPLEMENTS(ITask), IMPLEMENTS(IGlobalObject)>
{
public:
	cSim();
	~cSim();

   DECLARE_NAME_STRING(kSimName)
   DECLARE_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   // ISim
	virtual tResult Start();
	virtual tResult Stop();
	virtual bool IsRunning() const;

   virtual double GetTime() const;

   virtual double GetTimeScale() const;
   virtual void SetTimeScale(double timeScale);

	virtual tResult AddSimClient(ISimClient * pSimClient);
	virtual tResult RemoveSimClient(ISimClient * pSimClient);

   // ITask
   virtual tResult Execute(double time);

private:
   bool m_bIsRunning;
   double m_lastSchedTime; // the time argument from the last ITask::Execute
   double m_simTime;
   double m_timeScale;

   uint m_lockSimClients;
};

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_SIM_H
