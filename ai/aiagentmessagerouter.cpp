///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "aiagentmessagerouter.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include "tech/dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(AIAgentMessageRouter);

#define LocalMsg(msg)            DebugMsgEx(AIAgentMessageRouter,msg)
#define LocalMsg1(msg,a)         DebugMsgEx1(AIAgentMessageRouter,msg,(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(AIAgentMessageRouter,msg,(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(AIAgentMessageRouter,msg,(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)   DebugMsgEx4(AIAgentMessageRouter,msg,(a),(b),(c),(d))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(AIAgentMessageRouter,(cond),msg)
#define LocalMsgIf1(cond,msg,a)        DebugMsgIfEx1(AIAgentMessageRouter,(cond),msg,(a))
#define LocalMsgIf2(cond,msg,a,b)      DebugMsgIfEx2(AIAgentMessageRouter,(cond),msg,(a),(b))
#define LocalMsgIf3(cond,msg,a,b,c)    DebugMsgIfEx3(AIAgentMessageRouter,(cond),msg,(a),(b),(c))
#define LocalMsgIf4(cond,msg,a,b,c,d)  DebugMsgIfEx4(AIAgentMessageRouter,(cond),msg,(a),(b),(c),(d))


///////////////////////////////////////////////////////////////////////////////

extern tResult AIAgentMessageCreate(tAIAgentID receiver, tAIAgentID sender, double deliveryTime,
                                    eAIAgentMessageType messageType, uint nArgs, const cMultiVar * args,
                                    IAIAgentMessage * * ppAgentMessage);


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAIAgentMessageRouter
//

////////////////////////////////////////

cAIAgentMessageRouter::cAIAgentMessageRouter(bool bAutoConnectToSim)
 : m_bAutoConnectToSim(bAutoConnectToSim)
 , m_msgQueueIndex(0)
 , m_lastSimTime(0)
{
}

////////////////////////////////////////

cAIAgentMessageRouter::~cAIAgentMessageRouter()
{
}

////////////////////////////////////////

tResult cAIAgentMessageRouter::Init()
{
   if (m_bAutoConnectToSim)
   {
      UseGlobal(Sim);
      pSim->AddSimClient(static_cast<ISimClient*>(this));
   }

   return S_OK;
}

////////////////////////////////////////

tResult cAIAgentMessageRouter::Term()
{
   UseGlobal(Sim);
   pSim->RemoveSimClient(static_cast<ISimClient*>(this));

   ClearMessages();

   RevokeAllAgents();

   return S_OK;
}

////////////////////////////////////////

void cAIAgentMessageRouter::RevokeAllAgents()
{
   tAgentMap::iterator iter = m_agentMap.begin(), end = m_agentMap.end();
   for (; iter != end; ++iter)
   {
      iter->second->Release();
   }
   m_agentMap.clear();
}

////////////////////////////////////////

void cAIAgentMessageRouter::ClearMessages()
{
   for (int i = 0; i < _countof(m_msgQueue); ++i)
   {
      std::for_each(m_msgQueue[i].begin(), m_msgQueue[i].end(), CTInterfaceMethod(&IAIAgentMessage::Release));
      m_msgQueue[i].clear();
   }

   while (!m_delayedMsgQueue.empty())
   {
      IAIAgentMessage * pMsg = m_delayedMsgQueue.top();
      m_delayedMsgQueue.pop();
      SafeRelease(pMsg);
   }
}

////////////////////////////////////////

tResult cAIAgentMessageRouter::SendMessage(tAIAgentID receiver,
                                           tAIAgentID sender,
                                           eAIAgentMessageType messageType,
                                           uint nArgs,
                                           const cMultiVar * args)
{
   tResult result = E_FAIL;
   cAutoIPtr<IAIAgentMessage> pMsg;
   if ((result = AIAgentMessageCreate(receiver, sender, 0, messageType, nArgs, args, &pMsg)) == S_OK)
   {
      uint otherMsgQueueIndex = (m_msgQueueIndex + 1) & 1;
      m_msgQueue[otherMsgQueueIndex].push_back(CTAddRef(pMsg));
   }
   return result;
}

////////////////////////////////////////

tResult cAIAgentMessageRouter::SendMessage(tAIAgentID receiver,
                                           tAIAgentID sender,
                                           double deliveryTime,
                                           eAIAgentMessageType messageType,
                                           uint nArgs,
                                           const cMultiVar * args)
{
   tResult result = E_FAIL;
   cAutoIPtr<IAIAgentMessage> pMsg;
   if ((result = AIAgentMessageCreate(receiver, sender, deliveryTime, messageType, nArgs, args, &pMsg)) == S_OK)
   {
      m_delayedMsgQueue.push(CTAddRef(pMsg));
   }
   return result;
}

////////////////////////////////////////

tResult cAIAgentMessageRouter::RegisterAgent(IAIAgent * pAgent)
{
   if (pAgent == NULL)
   {
      return E_POINTER;
   }

   tAIAgentID id = pAgent->GetID();

   tAgentMap::iterator f = m_agentMap.find(id);
   if (f != m_agentMap.end())
   {
      ErrorMsg1("An agent with id %d is already registered\n", id);
      return E_FAIL;
   }

   m_agentMap.insert(std::make_pair(id, CTAddRef(pAgent)));
   return S_OK;
}

////////////////////////////////////////

tResult cAIAgentMessageRouter::RevokeAgent(IAIAgent * pAgent)
{
   if (pAgent == NULL)
   {
      return E_POINTER;
   }

   tAIAgentID id = pAgent->GetID();

   tAgentMap::iterator f = m_agentMap.find(id);
   if (f == m_agentMap.end())
   {
      WarnMsg1("No agent with id %d is registered\n", id);
      return S_FALSE;
   }
   else
   {
      f->second->Release();
      m_agentMap.erase(f);
      return S_OK;
   }
}

////////////////////////////////////////

tResult cAIAgentMessageRouter::Execute(double time)
{
   PumpMessages(time);
   return S_OK;
}

////////////////////////////////////////

void cAIAgentMessageRouter::PumpMessages(double time)
{
   tMsgList & msgQueue = m_msgQueue[m_msgQueueIndex];
   while (!msgQueue.empty())
   {
      cAutoIPtr<IAIAgentMessage> pMsg = msgQueue.front();
      AssertMsg(!!pMsg, "NULL message pointer should never have been queued");
      msgQueue.pop_front();
      DeliverMessage(pMsg);
   }
   m_msgQueueIndex = (m_msgQueueIndex + 1) & 1;

   while (!m_delayedMsgQueue.empty())
   {
      cAutoIPtr<IAIAgentMessage> pMsg = m_delayedMsgQueue.top();
      AssertMsg(!!pMsg, "NULL message pointer should never have been queued");
      if (pMsg->GetDeliveryTime() <= time)
      {
         m_delayedMsgQueue.pop();
         DeliverMessage(pMsg);
      }
      else
      {
         break;
      }
   }

   double elapsed = (m_lastSimTime == 0) ? 0 : (time - m_lastSimTime);
   tAgentMap::iterator iter = m_agentMap.begin(), end = m_agentMap.end();
   for (; iter != end; ++iter)
   {
      cAutoIPtr<IAIAgent> pAgent(CTAddRef(iter->second));
      pAgent->Update(elapsed);
   }
   m_lastSimTime = time;
}

////////////////////////////////////////

void cAIAgentMessageRouter::DeliverMessage(IAIAgentMessage * pMsg)
{
   tAgentMap::iterator f = m_agentMap.find(pMsg->GetReceiver());
   if (f != m_agentMap.end())
   {
      cAutoIPtr<IAIAgent> pAgent(CTAddRef(f->second));
      pAgent->HandleMessage(pMsg);

      LocalMsg4("AIAgentMessage delivered: receiver %d, sender %d, delivery %f, type %d\n",
         pMsg->GetReceiver(), pMsg->GetSender(), pMsg->GetDeliveryTime(), pMsg->GetMessageType());
   }
}

////////////////////////////////////////

tResult AIAgentMessageRouterCreate()
{
   cAutoIPtr<IAIAgentMessageRouter> pMessageRouter(static_cast<IAIAgentMessageRouter*>(new cAIAgentMessageRouter(true)));
   if (!pMessageRouter)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IAIAgentMessageRouter, pMessageRouter);
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

class cAIAgentMessageRouterTests
{
public:
   cAIAgentMessageRouterTests();
   ~cAIAgentMessageRouterTests();

   IAIAgentMessageRouter * AccessMsgRouter() { return static_cast<IAIAgentMessageRouter*>(m_pMsgRouter); }
   const IAIAgentMessageRouter * AccessMsgRouter() const { return static_cast<const IAIAgentMessageRouter*>(m_pMsgRouter); }

   cAutoIPtr<cAIAgentMessageRouter> m_pMsgRouter;
};

cAIAgentMessageRouterTests::cAIAgentMessageRouterTests()
{
   SafeRelease(m_pMsgRouter);
   m_pMsgRouter = new cAIAgentMessageRouter(false);
   m_pMsgRouter->Init();
}

cAIAgentMessageRouterTests::~cAIAgentMessageRouterTests()
{
   if (!!m_pMsgRouter)
   {
      m_pMsgRouter->Term();
      SafeRelease(m_pMsgRouter);
   }
}

class cMessageCollectingAgent : public cComObject<IMPLEMENTS(IAIAgent)>
{
public:
   cMessageCollectingAgent(tAIAgentID id) : m_id(id) {}
   ~cMessageCollectingAgent() { ClearMessages(); }

   virtual tAIAgentID GetID() const { return m_id; }

   virtual tResult SetLocationProvider(IAIAgentLocationProvider *) { return E_NOTIMPL; }
   virtual tResult GetLocationProvider(IAIAgentLocationProvider * *) { return E_NOTIMPL; }

   virtual tResult SetAnimationProvider(IAIAgentAnimationProvider *) { return E_NOTIMPL; }
   virtual tResult GetAnimationProvider(IAIAgentAnimationProvider * *) { return E_NOTIMPL; }

   virtual tResult SetDefaultBehavior(IAIAgentBehavior * pBehavior) { return E_NOTIMPL; }
   virtual tResult GetDefaultBehavior(IAIAgentBehavior * * ppBehavior) { return E_NOTIMPL; }

   virtual tResult PushBehavior(IAIAgentBehavior * pBehavior) { return E_NOTIMPL; }
   virtual tResult PopBehavior() { return E_NOTIMPL; }

   virtual tResult GetActiveBehavior(IAIAgentBehavior * * ppBehavior) { return E_NOTIMPL; }

   virtual tResult SetActiveTask(IAIAgentTask * pTask) { return E_NOTIMPL; }
   virtual tResult GetActiveTask(IAIAgentTask * * ppTask) { return E_NOTIMPL; }

   virtual tResult Update(double time) { return E_NOTIMPL; }

   virtual tResult HandleMessage(IAIAgentMessage * pMsg) { m_msgs.push_back(CTAddRef(pMsg)); return S_OK; }

   std::deque<IAIAgentMessage *>::iterator GetFirstMessage() { return m_msgs.begin(); }
   std::deque<IAIAgentMessage *>::iterator GetLastMessage() { return m_msgs.end(); }
   void ClearMessages() { std::for_each(m_msgs.begin(), m_msgs.end(), CTInterfaceMethod(&IAIAgentMessage::Release)), m_msgs.clear(); }

private:
   tAIAgentID m_id;
   std::deque<IAIAgentMessage *> m_msgs;
};

TEST_FIXTURE(cAIAgentMessageRouterTests, ProperAgentMessageOrder)
{
   static const tAIAgentID id = 1;
   cAutoIPtr<cMessageCollectingAgent> pAgent(new cMessageCollectingAgent(id));

   CHECK_EQUAL(S_OK, AccessMsgRouter()->RegisterAgent(pAgent));

   static const struct 
   {
      eAIAgentMessageType msgType;
      double deliveryTime;
   }
   msgs[] =
   {
      { kAIAMT_OrderStop, 4 },
      { kAIAMT_OrderMoveTo, 2 },
      { kAIAMT_OrderMoveTo, 5 },
      { kAIAMT_OrderMoveTo, 8 },
      { kAIAMT_OrderStop, 6 },
      { kAIAMT_OrderStop, 3 },
      { kAIAMT_OrderStop, 9 },
      { kAIAMT_OrderStop, 1 },
      { kAIAMT_OrderMoveTo, 7 },
   };

   double maxTime = -99999;
   for (int i = 0; i < _countof(msgs); ++i)
   {
      CHECK_EQUAL(S_OK, AccessMsgRouter()->SendMessage(id, 0, msgs[i].deliveryTime, msgs[i].msgType, 0, NULL));
      if (msgs[i].deliveryTime > maxTime)
      {
         maxTime = msgs[i].deliveryTime;
      }
   }

   m_pMsgRouter->PumpMessages(maxTime);

   std::deque<IAIAgentMessage *>::iterator iter = pAgent->GetFirstMessage(), end = pAgent->GetLastMessage();
   for (double index = 1; iter != end; ++iter, ++index)
   {
      CHECK(index == (*iter)->GetDeliveryTime());
   }

   CHECK_EQUAL(S_OK, AccessMsgRouter()->RevokeAgent(pAgent));
}

#endif // HAVE_UNITTESTPP

///////////////////////////////////////////////////////////////////////////////
