///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "sim.h"

#include "tech/connptimpl.h"

#include "tech/dbgalloc.h" // must be last header

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
 , m_time(0)
 , m_timeScale(1)
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
   std::for_each(m_simClients.begin(), m_simClients.end(), CTInterfaceMethod(&ISimClient::Release));
   m_simClients.clear();

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

double cSim::GetTimeScale() const
{
   return m_timeScale;
}

///////////////////////////////////////

void cSim::SetTimeScale(double scale)
{
   m_timeScale = scale;
}

////////////////////////////////////////

tResult cSim::AddSimClient(ISimClient * pSimClient)
{
   if (pSimClient == NULL)
   {
      return E_POINTER;
   }
   return add_interface(m_simClients, pSimClient) ? S_OK : S_FALSE;
}

////////////////////////////////////////

tResult cSim::RemoveSimClient(ISimClient * pSimClient)
{
   if (pSimClient == NULL)
   {
      return E_POINTER;
   }
   return remove_interface(m_simClients, pSimClient) ? S_OK : S_FALSE;
}

////////////////////////////////////////

tResult cSim::Execute(double time)
{
   if (m_bIsRunning)
   {
      double frameTime = time * m_timeScale;

      m_time += frameTime;

      tSimClientList::iterator iter = m_simClients.begin(), end = m_simClients.end();
      for (; iter != end; ++iter)
      {
         cAutoIPtr<ISimClient> pSimClient(CTAddRef(*iter));
         pSimClient->OnSimFrame(frameTime, m_time);
      }
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
