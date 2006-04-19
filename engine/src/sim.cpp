///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "simapi.h"

#include "techtime.h"
#include "globalobjdef.h"
#include "connptimpl.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSimGlobalObj
//

class cSimGlobalObj : public cComObject2<IMPLEMENTSCP(ISim, ISimClient), IMPLEMENTS(IGlobalObject)>
{
public:
   cSimGlobalObj();
   ~cSimGlobalObj();

   DECLARE_NAME_STRING(kSimObjName)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual void Go();
   virtual void Stop();
   virtual void Reset();
   virtual bool IsRunning() const;

   virtual double GetTime() const;
   virtual double GetFrameTime() const;

   virtual double GetTimeScale() const;
   virtual void SetTimeScale(double scale);

   virtual void NextFrame();

private:
   int m_nStopCount;
   double m_lastFrameTime, m_frameTime;
   double m_totalTime;
   double m_timeScale;
};

///////////////////////////////////////

cSimGlobalObj::cSimGlobalObj()
 : m_nStopCount(1),
   m_lastFrameTime(0), m_frameTime(0),
   m_totalTime(0),
   m_timeScale(1)
{
}

///////////////////////////////////////

cSimGlobalObj::~cSimGlobalObj()
{
}

///////////////////////////////////////

tResult cSimGlobalObj::Init()
{
   return S_OK;
}

///////////////////////////////////////

tResult cSimGlobalObj::Term()
{
   return S_OK;
}

///////////////////////////////////////

void cSimGlobalObj::Go()
{
   m_nStopCount--;
}

///////////////////////////////////////

void cSimGlobalObj::Stop()
{
   m_lastFrameTime = 0.0;
   m_nStopCount++;
}

///////////////////////////////////////

void cSimGlobalObj::Reset()
{
   m_frameTime = 0;
   m_totalTime = 0;
}

///////////////////////////////////////

bool cSimGlobalObj::IsRunning() const
{
   return m_nStopCount == 0;
}

///////////////////////////////////////

double cSimGlobalObj::GetTime() const
{
   return m_totalTime;
}

///////////////////////////////////////

double cSimGlobalObj::GetFrameTime() const
{
   return m_frameTime;
}

///////////////////////////////////////

double cSimGlobalObj::GetTimeScale() const
{
   return m_timeScale;
}

///////////////////////////////////////

void cSimGlobalObj::SetTimeScale(double scale)
{
   m_timeScale = scale;
}

///////////////////////////////////////

void cSimGlobalObj::NextFrame()
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

      ForEachConnection(&ISimClient::OnSimFrame, m_frameTime);
   }
}

///////////////////////////////////////

tResult SimCreate()
{
   cAutoIPtr<ISim> p(static_cast<ISim*>(new cSimGlobalObj));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_ISim, p);
}

///////////////////////////////////////////////////////////////////////////////
