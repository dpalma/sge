///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "sim.h"

#define BOOST_MEM_FN_ENABLE_STDCALL
#include <boost/mem_fn.hpp>

#include "tech/dbgalloc.h" // must be last header

using namespace boost;
using namespace std;

///////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(Sim);

#define LocalMsg(msg)            DebugMsgEx(Sim,msg)
#define LocalMsg1(msg,a)         DebugMsgEx1(Sim,msg,(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(Sim,msg,(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(Sim,msg,(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)   DebugMsgEx4(Sim,msg,(a),(b),(c),(d))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(Sim,(cond),msg)
#define LocalMsgIf1(cond,msg,a)        DebugMsgIfEx1(Sim,(cond),msg,(a))
#define LocalMsgIf2(cond,msg,a,b)      DebugMsgIfEx2(Sim,(cond),msg,(a),(b))
#define LocalMsgIf3(cond,msg,a,b,c)    DebugMsgIfEx3(Sim,(cond),msg,(a),(b),(c))
#define LocalMsgIf4(cond,msg,a,b,c,d)  DebugMsgIfEx4(Sim,(cond),msg,(a),(b),(c),(d))


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSim
//

////////////////////////////////////////

cSim::cSim()
 : m_bIsRunning(false)
 , m_lastSchedTime(0)
 , m_simTime(0)
 , m_timeScale(1)
 , m_lockSimClients(0)
{
}

////////////////////////////////////////

cSim::~cSim()
{
}

////////////////////////////////////////

BEGIN_CONSTRAINTS(cSim)
   AFTER_GUID(IID_IScheduler)
END_CONSTRAINTS()

////////////////////////////////////////

tResult cSim::Init()
{
   UseGlobal(Scheduler);
   pScheduler->AddFrameTask(static_cast<ITask*>(this), 0, 1, 0);

   return S_OK;
}

////////////////////////////////////////

tResult cSim::Term()
{
   DisconnectAll();

   UseGlobal(Scheduler);
   pScheduler->RemoveFrameTask(static_cast<ITask*>(this));

   return S_OK;
}

////////////////////////////////////////

tResult cSim::Start()
{
   if (m_bIsRunning)
   {
      return S_FALSE;
   }

   m_bIsRunning = true;
   m_lastSchedTime = 0;
   return S_OK;
}

////////////////////////////////////////

tResult cSim::Stop()
{
   if (!m_bIsRunning)
   {
      return S_FALSE;
   }

   m_bIsRunning = false;
   return S_OK;
}

////////////////////////////////////////

bool cSim::IsRunning() const
{
   return m_bIsRunning;
}

///////////////////////////////////////

double cSim::GetTime() const
{
   return m_simTime;
}

///////////////////////////////////////

double cSim::GetTimeScale() const
{
   return m_timeScale;
}

///////////////////////////////////////

void cSim::SetTimeScale(double scale)
{
   if (scale < 0 || scale > 25)
   {
      ErrorMsg1("Attempt to set insane sim time scale, %f\n", scale);
      return;
   }
   m_timeScale = scale;
}

////////////////////////////////////////

tResult cSim::AddSimClient(ISimClient * pSimClient)
{
   if (pSimClient == NULL)
   {
      return E_POINTER;
   }
   if (m_lockSimClients != 0)
   {
      return E_FAIL;
   }
   return Connect(pSimClient);
}

////////////////////////////////////////

tResult cSim::RemoveSimClient(ISimClient * pSimClient)
{
   if (pSimClient == NULL)
   {
      return E_POINTER;
   }
   if (m_lockSimClients != 0)
   {
      return E_FAIL;
   }
   return Disconnect(pSimClient);
}

////////////////////////////////////////

tResult cSim::Execute(double time)
{
   if (m_bIsRunning)
   {
      // If sim was stopped, catch up (elapsed will be zero this sim frame)
      if (m_lastSchedTime == 0)
      {
         m_lastSchedTime = time;
      }

      double elapsed = time - m_lastSchedTime;

      double frameTime = elapsed * m_timeScale;

      m_simTime += frameTime;

      ++m_lockSimClients;
      tSinksIterator iter = BeginSinks(), end = EndSinks();
      for (; iter != end; ++iter)
      {
         cAutoIPtr<ISimClient> pSimClient(CTAddRef(*iter));
         if (pSimClient->Execute(m_simTime) != S_OK)
         {
            iter = Disconnect(iter);
         }
      }
      --m_lockSimClients;

      m_lastSchedTime = time;
   }

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

tResult SimCreate()
{
   cAutoIPtr<ISim> pSim(static_cast<ISim*>(new cSim));
   if (!pSim)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_ISim, pSim);
}

///////////////////////////////////////////////////////////////////////////////
