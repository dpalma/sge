///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCHEDULERAPI_H
#define INCLUDED_SCHEDULERAPI_H

/// @file schedulerapi.h
/// Interface definitions for the task scheduler

#include "techdll.h"
#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE_GUID(ITask, "A237CD9F-32CA-4229-9AB2-705776BCAD9F");
F_DECLARE_INTERFACE_GUID(IScheduler, "197D6E0A-DA84-4727-B9CB-7AECAACDC653");

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

   /// @brief Registers a task to run on every call to IScheduler::NextFrame
   /// @param pTask specifies the task to run every frame
   /// @return S_OK if successful, or an E_xxx error code
	virtual tResult AddRenderTask(ITask * pTask) = 0;
	virtual tResult RemoveRenderTask(ITask * pTask) = 0;

	virtual tResult AddFrameTask(ITask * pTask, ulong start, ulong period, ulong duration) = 0;
	virtual tResult RemoveFrameTask(ITask * pTask) = 0;

	virtual tResult AddTimeTask(ITask * pTask, double start, double period, double duration) = 0;
	virtual tResult RemoveTimeTask(ITask * pTask) = 0;
};

///////////////////////////////////////

#define kSchedulerName "Scheduler"
TECH_API tResult SchedulerCreate();

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCHEDULERAPI_H
