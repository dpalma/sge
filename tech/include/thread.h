///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_THREAD_H
#define INCLUDED_THREAD_H

#include "techdll.h"

#ifndef _WIN32
#include <pthread.h>
#endif

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

const uint kInfiniteTimeout = (uint)-1;

TECH_API void ThreadSleep(uint milliseconds);

TECH_API uint ThreadGetCurrentId();

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cThread
//

enum eThreadPriority
{
   kTP_Lowest = 0,
   kTP_Highest = 15,
   kTP_Normal = (kTP_Lowest + kTP_Highest) / 2,
};

class TECH_API cThread
{
public:
   cThread();
   virtual ~cThread();

   bool Create(int priority = kTP_Normal, uint stackSize = 0);

   void Join();

   bool Terminate();

protected:
   virtual int Run() = 0;

private:
#ifdef _WIN32
   static ulong STDCALL ThreadEntry(void * param);
   HANDLE m_hThread;
#else
   static void * ThreadEntry(void * param);
   pthread_t m_thread;
#endif
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cThreadEvent
//

class TECH_API cThreadEvent
{
public:
   cThreadEvent();
   ~cThreadEvent();

   bool Create();

   bool Set();
   bool Reset();

   void Wait(uint timeout = kInfiniteTimeout);

private:
#ifdef _WIN32
   HANDLE m_hEvent;
#else
#endif
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_THREAD_H
