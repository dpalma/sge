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

const uint kInfiniteTimeout = ~0;

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
protected:
   cThread(const cThread &);
   const cThread & operator =(const cThread &);

public:
   cThread();
   virtual ~cThread();

   bool Create(int priority = kTP_Normal, uint stackSize = 0);

   ulong GetThreadId() const;

   void Join();

   bool Terminate();

protected:
   virtual int Run() = 0;

private:
   ulong m_threadId;
#ifdef _WIN32
   static ulong STDCALL ThreadEntry(void * param);
   HANDLE m_hThread;
#else
   static void * ThreadEntry(void * param);
   pthread_t m_thread;
#endif
};

////////////////////////////////////////

inline ulong cThread::GetThreadId() const
{
   return m_threadId;
}

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

   bool Wait(uint timeout = kInfiniteTimeout);
   bool Set();
   bool Reset();
   bool Pulse();

private:
#ifdef _WIN32
   HANDLE m_hEvent;
#else
#endif
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cThreadMutex
//

class TECH_API cThreadMutex
{
public:
   cThreadMutex();
   ~cThreadMutex();

   bool Create();

   bool Acquire(uint timeout = kInfiniteTimeout);
   bool Release();

private:
#ifdef _WIN32
   HANDLE m_hMutex;
#else
#endif
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMutexLock
//

class TECH_API cMutexLock
{
public:
   cMutexLock(cThreadMutex * pMutex);
   ~cMutexLock();

   bool Acquire(uint timeout = kInfiniteTimeout);
   void Release();

private:
   cThreadMutex * m_pMutex;
   bool m_bLocked;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_THREAD_H
