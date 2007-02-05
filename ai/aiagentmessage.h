////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_AIAGENTMESSAGE_H
#define INCLUDED_AIAGENTMESSAGE_H

#include "ai/aiagentapi.h"

#include "tech/multivar.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAIAgentMessage
//

class cAIAgentMessage : public cComObject<IMPLEMENTS(IAIAgentMessage)>
{
public:
   cAIAgentMessage(tAIAgentID receiver, tAIAgentID sender, double deliveryTime,
      eAIAgentMessageType messageType, uint nArgs, const cMultiVar * args);
   ~cAIAgentMessage();

   virtual tAIAgentID GetReceiver() const;
   virtual tAIAgentID GetSender() const;
   virtual double GetDeliveryTime() const;
   virtual eAIAgentMessageType GetMessageType() const;
   virtual uint GetArgumentCount() const;
   virtual tResult GetArgument(uint index, cMultiVar * pArg) const;
   virtual tResult GetArguments(uint * pnArgs, cMultiVar * pArgs) const;

private:
   tAIAgentID m_receiver;
   tAIAgentID m_sender;
   double m_deliveryTime;
   eAIAgentMessageType m_messageType;
   std::vector<cMultiVar> m_args;
};

////////////////////////////////////////

tResult AIAgentMessageCreate(tAIAgentID receiver, tAIAgentID sender, double deliveryTime,
                             eAIAgentMessageType messageType, uint nArgs, const cMultiVar * args,
                             IAIAgentMessage * * ppAgentMessage);


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAIAgentMessageNoSelfDelete
//

class cAIAgentMessageNoSelfDelete : public cAIAgentMessage
{
   // TODO: prevent heap allocation

public:
   cAIAgentMessageNoSelfDelete(tAIAgentID receiver, tAIAgentID sender, double deliveryTime,
      eAIAgentMessageType messageType, uint nArgs, const cMultiVar * args)
    : cAIAgentMessage(receiver, sender, deliveryTime, messageType, nArgs, args)
   {
   }

   virtual void DeleteThis()
   {
      // Don't actually delete
   }
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_AIAGENTMESSAGE_H
