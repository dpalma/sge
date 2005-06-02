///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_THREADCALLER_H
#define INCLUDED_THREADCALLER_H

#include "threadcallapi.h"
#include "thread.h"
#include "globalobj.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cThreadCaller
//

class cThreadCaller : public cGlobalObject<IMPLEMENTS(IThreadCaller)>
{
public:
   cThreadCaller();
   virtual ~cThreadCaller();

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

   typedef std::map<tThreadId, cThreadEvent *> tReceiptEventMap;
   tReceiptEventMap m_receiptEvents;

   cThreadMutex m_mutex;
};

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_THREADCALLER_H
