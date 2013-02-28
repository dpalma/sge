///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "aiagentmessage.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

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

tResult cAIAgentMessage::GetArguments(uint * pnArgs, cMultiVar * pArgs) const
{
   if (pnArgs == NULL || pArgs == NULL)
   {
      return E_POINTER;
   }

   uint nReturned = Min(*pnArgs, m_args.size());
   for (uint i = 0; i < nReturned; ++i)
   {
      *pArgs++ = m_args[i];
   }

   *pnArgs = nReturned;

   return (nReturned == m_args.size()) ? S_OK : S_FALSE;
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

#ifdef HAVE_UNITTESTPP

TEST(AIAgentMessageGetArguments)
{
   static const cMultiVar expectedArgs[] =
   {
      cMultiVar(3.1415f),
      cMultiVar(_T("test string")),
      cMultiVar(NULL),
      cMultiVar(10000),
   };

   cAutoIPtr<IAIAgentMessage> pMsg;
   CHECK_EQUAL(S_OK, AIAgentMessageCreate(1, 0, 0, kAIAMT_OrderStop, _countof(expectedArgs), expectedArgs, &pMsg));

   {
      cMultiVar actualArgs[_countof(expectedArgs)];

      uint nActualArgs = _countof(actualArgs);
      CHECK_EQUAL(S_OK, pMsg->GetArguments(&nActualArgs, actualArgs));

      for (uint i = 0; i < nActualArgs; ++i)
      {
         CHECK(expectedArgs[i].IsEqual(actualArgs[i]));
      }
   }

   {
      cMultiVar notEnoughArgs[_countof(expectedArgs) / 2];

      uint nNotEnoughArgs = _countof(notEnoughArgs);
      CHECK_EQUAL(S_FALSE, pMsg->GetArguments(&nNotEnoughArgs, notEnoughArgs));
      CHECK_EQUAL(nNotEnoughArgs, _countof(notEnoughArgs));

      for (uint i = 0; i < nNotEnoughArgs; ++i)
      {
         CHECK(expectedArgs[i].IsEqual(notEnoughArgs[i]));
      }
   }
}

#endif // HAVE_UNITTESTPP

///////////////////////////////////////////////////////////////////////////////
