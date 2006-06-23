///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCHEDULERAPI_H
#define INCLUDED_SCHEDULERAPI_H

#include "enginedll.h"
#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(ITask);
F_DECLARE_INTERFACE(IScheduler);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ITask
//

interface ITask : IUnknown
{
   virtual tResult Execute(double time) = 0;
};

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IScheduler
//

interface IScheduler : IUnknown
{
	virtual void Start() = 0;
	virtual void Stop() = 0;
	virtual bool IsRunning() const = 0;

	virtual void NextFrame() = 0;

	virtual tResult AddRenderTask(ITask * pTask) = 0;
	virtual tResult RemoveRenderTask(ITask * pTask) = 0;

	virtual tResult AddFrameTask(ITask * pTask, ulong start, ulong period, ulong duration) = 0;
	virtual tResult RemoveFrameTask(ITask * pTask) = 0;

	virtual tResult AddTimeTask(ITask * pTask, double start, double period, double duration) = 0;
	virtual tResult RemoveTimeTask(ITask * pTask) = 0;
};

///////////////////////////////////////

#define kSchedulerName "Scheduler"
ENGINE_API tResult SchedulerCreate();

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCHEDULERAPI_H
