///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "TechTime.h"

#include "sim.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSim
//

///////////////////////////////////////

cSim::cSim()
 : m_nStopCount(1),
   m_lastFrameTime(0), m_frameTime(0),
   m_totalTime(0),
   m_timeScale(1)
{
}

///////////////////////////////////////

void cSim::Go()
{
   m_nStopCount--;
}

///////////////////////////////////////

void cSim::Stop()
{
   m_lastFrameTime = 0.0;
   m_nStopCount++;
}

///////////////////////////////////////

void cSim::Reset()
{
   m_frameTime = 0;
   m_totalTime = 0;
}

///////////////////////////////////////

bool cSim::IsRunning() const
{
   return m_nStopCount == 0;
}

///////////////////////////////////////

double cSim::GetTime() const
{
   return m_totalTime;
}

///////////////////////////////////////

double cSim::GetFrameTime() const
{
   return m_frameTime;
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

///////////////////////////////////////

void cSim::NextFrame()
{
   if (IsRunning())
   {
      double curTime = TimeGetSecs();

      if (m_lastFrameTime == 0.0)
      {
         m_lastFrameTime = curTime;
         return;
      }

      m_frameTime = curTime - m_lastFrameTime;
      m_lastFrameTime = curTime;

      m_frameTime *= m_timeScale;

      m_totalTime += m_frameTime;

      ForEachConnection(&ISimClient::OnFrame, m_frameTime);
   }
}

///////////////////////////////////////////////////////////////////////////////
