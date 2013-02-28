///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_THREADCALLER_H
#define INCLUDED_THREADCALLER_H

#include "tech/threadcallapi.h"
#include "tech/thread.h"
#include "tech/globalobjdef.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cThreadCaller
//

class cThreadCaller : public cComObject2<IMPLEMENTS(IThreadCaller), IMPLEMENTS(IGlobalObject)>
{
public:
   cThreadCaller();
   virtual ~cThreadCaller();

   DECLARE_NAME(ThreadCaller)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult ThreadInit();
   virtual tResult ThreadTerm();
   virtual tResult ThreadIsInitialized(tThreadId threadId);

   virtual tResult ReceiveCalls(uint * pnCalls);

   virtual tResult PostCall(tThreadId threadId, cFunctor * pFunctor);

private:
   typedef std::multimap<tThreadId, cFunctor *> tThreadCalls;
   tThreadCalls m_calls;

   struct sThreadInfo
   {
      long initCount;
      cThreadEvent callEvent;
   };

   typedef std::map<tThreadId, sThreadInfo *> tThreadInfoMap;
   tThreadInfoMap m_threadInfoMap;

   cThreadMutex m_mutex;
};

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_THREADCALLER_H
