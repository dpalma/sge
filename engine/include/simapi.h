///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SIM_H
#define INCLUDED_SIM_H

#include "comtools.h"
#include "ConnPtImpl.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(ISimClient);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ISimClient
//

interface ISimClient : IUnknown
{
   virtual void OnFrame(double elapsedTime) = 0;
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSim
//

class cSimBASE {};
class cSim : public cConnectionPoint<cSimBASE, ISimClient>
{
public:
   cSim();

   void Go();
   void Stop();
   void Reset();
   bool IsRunning() const;

   double GetTime() const;
   double GetFrameTime() const;

   double GetTimeScale() const;
   void SetTimeScale(double scale);

   void NextFrame();

private:
   int m_nStopCount;
   double m_lastFrameTime, m_frameTime;
   double m_totalTime;
   double m_timeScale;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SIM_H
