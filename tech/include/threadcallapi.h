///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_THREADCALLAPI_H
#define INCLUDED_THREADCALLAPI_H

#include "techdll.h"
#include "comtools.h"
#include "functor.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IThreadCaller);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IThreadCaller
//

interface IThreadCaller : IUnknown
{
   virtual tResult ThreadInit() = 0;
   virtual tResult ThreadTerm() = 0;
   virtual tResult ThreadIsInitialized(uint threadId) = 0;

   virtual tResult ReceiveCalls(uint * pnCalls) = 0;

   virtual tResult PostCall(uint threadId, cFunctor * pFunctor) = 0;
};

////////////////////////////////////////

TECH_API void ThreadCallerCreate();

///////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////

template <typename RETURN>
cFunctor * MakeThreadCall(RETURN (* pfn)(void))
{
   return new cFunctor0<RETURN>(pfn);
}

////////////////////////////////////////

template <typename RETURN, typename ARG1>
cFunctor * MakeThreadCall(RETURN (* pfn)(ARG1), ARG1 arg1)
{
   return new cFunctor1<RETURN, ARG1>(pfn, arg1);
}

////////////////////////////////////////

template <typename RETURN, typename ARG1, typename ARG2>
cFunctor * MakeThreadCall(RETURN (* pfn)(ARG1, ARG2), ARG1 arg1, ARG2 arg2)
{
   return new cFunctor2<RETURN, ARG1, ARG2>(pfn, arg1, arg2);
}

////////////////////////////////////////

template <typename RETURN, typename ARG1, typename ARG2, typename ARG3>
cFunctor * MakeThreadCall(RETURN (* pfn)(ARG1, ARG2, ARG3), ARG1 arg1, ARG2 arg2, ARG3 arg3)
{
   return new cFunctor2<RETURN, ARG1, ARG2, ARG3>(pfn, arg1, arg2, arg3);
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_THREADCALLAPI_H
