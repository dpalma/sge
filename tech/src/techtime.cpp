///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "techtime.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <limits.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// STRUCT: sFrameStats
//

///////////////////////////////////////

sFrameStats::sFrameStats()
 : fps(0),
   worst(999999),
   best(0),
   average(0),
   lastTime(0),
   frameCount(0)
{
}

///////////////////////////////////////

void sFrameStats::Update()
{
   double time = TimeGetSecs();
   double elapsed = time - lastTime;
   frameCount++;

   if (elapsed >= 1.0)
   {
      if (lastTime != 0)
      {
         double fpsNew = (double)frameCount / elapsed;
         if (average == 0)
            average = fpsNew;
         else
            average = (fpsNew + fps) * 0.5;
         fps = fpsNew;
         if (fps > best)
            best = fps;
         if (fps < worst)
            worst = fps;
      }
      lastTime = time;
      frameCount = 0;
   }
}

///////////////////////////////////////////////////////////////////////////////

double CalcFramesPerSec()
{
   static sFrameStats stats;
   stats.Update();
   return stats.fps;
}

///////////////////////////////////////////////////////////////////////////////

double TimeGetSecs()
{
#ifdef _WIN32
   static bool usingQPC = false;
   static bool first = true;
   static double oneOverTicksPerSec;
   static LARGE_INTEGER startQPC;
   static DWORD startTime;

   if (first)
   {
      first = false;
      LARGE_INTEGER tps;
      if (QueryPerformanceFrequency(&tps))
      {
         usingQPC = true;
         oneOverTicksPerSec = 1.0 / tps.QuadPart;
         QueryPerformanceCounter(&startQPC);
      }
      else
      {
         startTime = timeGetTime();
      }
      return 0.0;
   }

   if (usingQPC)
   {
      LARGE_INTEGER now;
      QueryPerformanceCounter(&now);
      return (double)(now.QuadPart - startQPC.QuadPart) * oneOverTicksPerSec;
   }
   else
   {
      DWORD now = timeGetTime();

      if (now < startTime) // wrapped?
         return (now * 0.001) + (ULONG_MAX - startTime * 0.001);

      if (now - startTime == 0)
         return 0.0;

      return (now - startTime) * 0.001;
   }
#else
   static bool first = true;
   static double start;

   struct timeval tv;
   struct timezone tz;
   
   if (first)
   {
      first = false;
      gettimeofday(&tv, &tz);
      start = (double)tv.tv_sec + ((double)tv.tv_usec * 0.000001);
      return 0;
   }

   gettimeofday(&tv, &tz);
   double now = (double)tv.tv_sec + ((double)tv.tv_usec * 0.000001);

   return now - start;
#endif
}

///////////////////////////////////////////////////////////////////////////////
