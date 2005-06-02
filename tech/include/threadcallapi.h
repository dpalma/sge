///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_THREADCALLAPI_H
#define INCLUDED_THREADCALLAPI_H

/// @file threadcallapi.h
/// Framework for making function calls across threads

#include "techdll.h"
#include "comtools.h"
#include "functor.h"

#ifdef _WIN32
typedef ulong tThreadId;
#else
#include <sys/types.h>
typedef pthread_t tThreadId;
#endif

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IThreadCaller);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IThreadCaller
//
/// @interface IThreadCaller
/// @brief Central arbiter of cross-thread function calls

interface IThreadCaller : IUnknown
{
   /// @brief Registers the calling thread, preparing it to receive calls
   /// from other threads
   virtual tResult ThreadInit() = 0;
   virtual tResult ThreadTerm() = 0;

   /// @return S_OK or S_FALSE for whether the thread is registered with the thread caller
   /// @remarks Use with caution since is it possible for the specified thread
   /// to call ThreadTerm() before the caller has time to comprehend the
   /// return value here
   virtual tResult ThreadIsInitialized(tThreadId threadId) = 0;

   /// @brief Invokes all function calls queued for the calling thread
   /// since the last call to ReceiveCalls()
   /// @remarks Should be called periodically; once per frame or something
   virtual tResult ReceiveCalls(uint * pnCalls) = 0;

   /// @brief Queues a function call from the calling thread to the thread
   /// specified by the threadId parameter
   virtual tResult PostCall(tThreadId threadId, cFunctor * pFunctor) = 0;

   template <typename RETURN>
   tResult PostCall(tThreadId threadId, RETURN (* pfn)())
   {
      cFunctor * pFunctor = new cFunctor0<RETURN>(pfn);
      if (pFunctor == NULL)
      {
         return E_OUTOFMEMORY;
      }
      return PostCall(threadId, pFunctor);
   }

   template <typename RETURN, typename ARG1>
   tResult PostCall(tThreadId threadId, RETURN (* pfn)(ARG1), ARG1 arg1)
   {
      cFunctor * pFunctor = new cFunctor1<RETURN, ARG1>(pfn, arg1);
      if (pFunctor == NULL)
      {
         return E_OUTOFMEMORY;
      }
      return PostCall(threadId, pFunctor);
   }

   template <typename RETURN, typename ARG1, typename ARG2>
   tResult PostCall(tThreadId threadId, RETURN (* pfn)(ARG1, ARG2), ARG1 arg1, ARG2 arg2)
   {
      cFunctor * pFunctor = new cFunctor2<RETURN, ARG1, ARG2>(pfn, arg1, arg2);
      if (pFunctor == NULL)
      {
         return E_OUTOFMEMORY;
      }
      return PostCall(threadId, pFunctor);
   }

   template <typename RETURN, typename ARG1, typename ARG2, typename ARG3>
   tResult PostCall(tThreadId threadId, RETURN (* pfn)(ARG1, ARG2, ARG3), ARG1 arg1, ARG2 arg2, ARG3 arg3)
   {
      cFunctor * pFunctor = new cFunctor3<RETURN, ARG1, ARG2, ARG3>(pfn, arg1, arg2, arg3);
      if (pFunctor == NULL)
      {
         return E_OUTOFMEMORY;
      }
      return PostCall(threadId, pFunctor);
   }
};

////////////////////////////////////////

TECH_API void ThreadCallerCreate();

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_THREADCALLAPI_H
