///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "aiagentmessage.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAIAgentMessage
//

////////////////////////////////////////

cAIAgentMessage::cAIAgentMessage(tAIAgentID receiver, tAIAgentID sender, double deliveryTime,
                                 eAIAgentMessageType messageType, uint nArgs, const cMultiVar * args)
 : m_receiver(receiver)
 , m_sender(sender)
 , m_deliveryTime(deliveryTime)
 , m_messageType(messageType)
{
   if (nArgs > 0 && args != NULL)
   {
      for (uint i = 0; i < nArgs; ++i)
      {
         m_args.push_back(args[i]);
      }
   }
}

////////////////////////////////////////

cAIAgentMessage::~cAIAgentMessage()
{
}

////////////////////////////////////////

tAIAgentID cAIAgentMessage::GetReceiver() const
{
   return m_receiver;
}

////////////////////////////////////////

tAIAgentID cAIAgentMessage::GetSender() const
{
   return m_sender;
}

////////////////////////////////////////

double cAIAgentMessage::GetDeliveryTime() const
{
   return m_deliveryTime;
}

////////////////////////////////////////

eAIAgentMessageType cAIAgentMessage::GetMessageType() const
{
   return m_messageType;
}

////////////////////////////////////////

uint cAIAgentMessage::GetArgumentCount() const
{
   return m_args.size();
}

////////////////////////////////////////

tResult cAIAgentMessage::GetArgument(uint index, cMultiVar * pArg) const
{
   if (index >= m_args.size())
   {
      return E_INVALIDARG;
   }
   if (pArg == NULL)
   {
      return E_POINTER;
   }
   *pArg = m_args[index];
   return S_OK;
}

////////////////////////////////////////

tResult AIAgentMessageCreate(tAIAgentID receiver, tAIAgentID sender, double deliveryTime,
                             eAIAgentMessageType messageType, uint nArgs, const cMultiVar * args,
                             IAIAgentMessage * * ppAgentMessage)
{
   if (receiver == 0)
   {
      return E_INVALIDARG;
   }

   if (ppAgentMessage == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IAIAgentMessage> pMsg(static_cast<IAIAgentMessage *>(
      new cAIAgentMessage(receiver, sender, deliveryTime, messageType, nArgs, args)));
   if (!pMsg)
   {
      return E_OUTOFMEMORY;
   }

   return pMsg.GetPointer(ppAgentMessage);
}


///////////////////////////////////////////////////////////////////////////////
