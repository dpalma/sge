////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_AIAGENTMESSAGEROUTER_H
#define INCLUDED_AIAGENTMESSAGEROUTER_H

#include "ai/aiagentapi.h"

#include "tech/globalobjdef.h"
#include "tech/simapi.h"

#include <deque>
#include <map>
#include <queue>

#ifdef _MSC_VER
#pragma once
#endif


struct sAIAgentMessageCompare
{
   bool operator()(const IAIAgentMessage * pMsg1, const IAIAgentMessage * pMsg2) const
   {
      // Returning true puts the first argument toward the back of the
      // priority queue. Use a greater than test so that messages with earlier
      // delivery times are sorted toward the front of the priority queue.
      return pMsg1->GetDeliveryTime() > pMsg2->GetDeliveryTime();
   }
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAIAgentMessageRouter
//

class cAIAgentMessageRouter : public cComObject3<IMPLEMENTS(IAIAgentMessageRouter),
                                                 IMPLEMENTS(ISimClient),
                                                 IMPLEMENTS(IGlobalObject)>
{
public:
   cAIAgentMessageRouter(bool bAutoConnectToSim);
   ~cAIAgentMessageRouter();

   DECLARE_NAME(AIAgentMessageRouter)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   void RevokeAllAgents();
   void ClearMessages();

   virtual tResult SendMessage(tAIAgentID receiver, tAIAgentID sender,
      eAIAgentMessageType messageType, uint nArgs, const cMultiVar * args);
   virtual tResult SendMessage(tAIAgentID receiver, tAIAgentID sender, double deliveryTime,
      eAIAgentMessageType messageType, uint nArgs, const cMultiVar * args);

   virtual tResult RegisterAgent(IAIAgent * pAgent);
   virtual tResult RevokeAgent(IAIAgent * pAgent);

   virtual tResult Execute(double time);

   void PumpMessages(double time);

   void DeliverMessage(IAIAgentMessage * pMsg);

private:
   bool m_bAutoConnectToSim;

   typedef std::map<tAIAgentID, IAIAgent *> tAgentMap;
   tAgentMap m_agentMap;

   typedef std::deque<IAIAgentMessage *> tMsgList;
   uint m_msgQueueIndex;
   tMsgList m_msgQueue[2];

   typedef std::priority_queue<IAIAgentMessage *, std::deque<IAIAgentMessage *>, sAIAgentMessageCompare> tMsgQueue;
   tMsgQueue m_delayedMsgQueue;

   double m_lastSimTime; // HACK: for calculating elapsed (maybe tasks should take the time itself instead of elapsed)
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_AIAGENTMESSAGEROUTER_H
