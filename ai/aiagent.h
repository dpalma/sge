////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_AIAGENT_H
#define INCLUDED_AIAGENT_H

#include "ai/aiagentapi.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAIAgent
//

class cAIAgent : public cComAggregableObject<IMPLEMENTS(IAIAgent)>
{
public:
   cAIAgent(tAIAgentID id, IUnknown * pUnkOuter);
   ~cAIAgent();

   tResult SetDefaultTask(IAIAgentTask * pTask);
   tResult GetDefaultTask(IAIAgentTask * * ppTask);

   tResult AddTask(IAIAgentTask * pTask);
   tResult RemoveTask(IAIAgentTask * pTask);
   tResult RemoveAllTasks();
   tResult GetActiveTask(IAIAgentTask * * ppTask);

   virtual tAIAgentID GetID() const;

   virtual tResult SetLocationProvider(IAIAgentLocationProvider * pLocationProvider);
   virtual tResult GetLocationProvider(IAIAgentLocationProvider * * ppLocationProvider);

   virtual tResult SetAnimationProvider(IAIAgentAnimationProvider * pAnimationProvider);
   virtual tResult GetAnimationProvider(IAIAgentAnimationProvider * * ppAnimationProvider);

   virtual tResult Update(double time);

   virtual tResult HandleMessage(IAIAgentMessage * pMessage);

private:
   tAIAgentID m_id;

   cAutoIPtr<IAIAgentTask> m_pDefaultTask;

   typedef std::list<IAIAgentTask*> tTaskList;
   tTaskList m_taskQueue;

   cAutoIPtr<IAIAgentLocationProvider> m_pLocationProvider;
   cAutoIPtr<IAIAgentAnimationProvider> m_pAnimationProvider;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_AIAGENT_H
