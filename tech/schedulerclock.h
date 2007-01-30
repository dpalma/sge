///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCHEDULERCLOCK_H
#define INCLUDED_SCHEDULERCLOCK_H

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSchedulerClock
//

class cSchedulerClock
{
   cSchedulerClock(const cSchedulerClock &);
   const cSchedulerClock & operator =(const cSchedulerClock &);

public:
	cSchedulerClock();
	~cSchedulerClock();

	void Reset();
	bool IsRunning() const;
	void Start();
	void Stop();

	void UpdateRealTime();

	void BeginFrame();
	void EndFrame();
	void AdvanceTo(double newTime);

	double GetSimTime() const;
	ulong GetFrameCount() const;
	double GetFrameStart() const;
	double GetFrameEnd() const;

private:
	bool m_bRunning;

	double m_thisTime;
	double m_lastTime;

	double m_realTime;
	double m_realOffset;
	double m_pauseTime;

	ulong m_frameCount;
	double m_frameStart;
	double m_frameEnd;

	double m_simTime;
	double m_simOffset;
};

////////////////////////////////////////

inline bool cSchedulerClock::IsRunning() const
{
   return m_bRunning;
}

////////////////////////////////////////

inline double cSchedulerClock::GetSimTime() const
{
   return m_simTime;
}

////////////////////////////////////////

inline ulong cSchedulerClock::GetFrameCount() const
{
   return m_frameCount;
}

////////////////////////////////////////

inline double cSchedulerClock::GetFrameStart() const
{
   return m_frameStart;
}

////////////////////////////////////////

inline double cSchedulerClock::GetFrameEnd() const
{
   return m_frameEnd;
}

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_SCHEDULERCLOCK_H
