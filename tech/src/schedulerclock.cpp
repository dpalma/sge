///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "schedulerclock.h"

#include "tech/techtime.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSchedulerClock
//

////////////////////////////////////////

cSchedulerClock::cSchedulerClock()
 : m_bRunning(false)
 , m_thisTime(0)
 , m_lastTime(0)
 , m_realTime(0)
 , m_realOffset(0)
 , m_pauseTime(0)
 , m_frameCount(0)
 , m_frameStart(0)
 , m_frameEnd(0)
 , m_simTime(0)
 , m_simOffset(0)
{
   Reset();
}

////////////////////////////////////////

cSchedulerClock::~cSchedulerClock()
{
}

////////////////////////////////////////

void cSchedulerClock::Reset()
{
   m_bRunning = false;

   m_thisTime = TimeGetSecs();
   m_lastTime = m_thisTime;

   m_realTime = 0;
   m_pauseTime = 0;
   m_realOffset = m_thisTime;

   m_frameCount = 0;
   m_frameStart = 0;
   m_frameEnd = 0;

   m_simTime = 0;
   m_simOffset = 0;
}

////////////////////////////////////////

void cSchedulerClock::Start()
{
   if (!m_bRunning)
   {
      UpdateRealTime();
      m_simOffset += (m_realTime - m_pauseTime);
      m_bRunning = true;
   }
}

////////////////////////////////////////

void cSchedulerClock::Stop()
{
   if (m_bRunning)
   {
      UpdateRealTime();
      m_pauseTime = m_realTime;
      m_bRunning = false;
   }
}

////////////////////////////////////////

void cSchedulerClock::UpdateRealTime()
{
   m_lastTime = m_thisTime;
   m_thisTime = TimeGetSecs();

   double elapsed = 0;
   if (m_thisTime < m_lastTime)
   {
      elapsed = m_lastTime - m_thisTime;
   }
   else
   {
      elapsed = m_thisTime - m_lastTime;
   }

   m_realTime += elapsed;
}

////////////////////////////////////////

void cSchedulerClock::BeginFrame()
{
   m_frameCount++;

   UpdateRealTime();

   if (IsRunning())
   {
      m_frameStart = m_frameEnd;
      m_frameEnd = m_realTime - m_simOffset;
      m_simTime = m_frameStart;
   }
}

////////////////////////////////////////

void cSchedulerClock::EndFrame()
{
   if (IsRunning())
   {
      m_simTime = m_frameEnd;
   }
}

////////////////////////////////////////

void cSchedulerClock::AdvanceTo(double newTime)
{
   if (IsRunning() && (newTime >= m_simTime))
   {
      m_simTime = newTime;
   }
}

///////////////////////////////////////////////////////////////////////////////
