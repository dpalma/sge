///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "platform/sys.h"

#include "tech/techtime.h"

#ifdef HAVE_UNITTESTPP
#include "Test.h"
#include "TestReporter.h"
#include "TestRunner.h"
#endif

#include <cstdlib>
#include <list>

#include "tech/dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(VerboseUnitTests);

///////////////////////////////////////////////////////////////////////////////

tSysDestroyFn        g_pfnDestroyCallback = NULL;
tSysCharEventFn      g_pfnCharCallback = NULL;
tSysKeyEventFn       g_pfnKeyCallback = NULL;
tSysMouseEventFn     g_pfnMouseMoveCallback = NULL;
tSysMouseEventFn     g_pfnMouseCallback = NULL;
tSysFrameFn          g_pfnFrameCallback = NULL;
tSysResizeFn         g_pfnResizeCallback = NULL;

uint_ptr             g_charCallbackUserData = 0;
uint_ptr             g_keyCallbackUserData = 0;
uint_ptr             g_mouseMoveCallbackUserData = 0;
uint_ptr             g_mouseCallbackUserData = 0;

///////////////////////////////////////////////////////////////////////////////

tSysDestroyFn SysSetDestroyCallback(tSysDestroyFn pfn)
{
   tSysDestroyFn pfnFormer = g_pfnDestroyCallback;
   g_pfnDestroyCallback = pfn;
   return pfnFormer;
}

void SysSetCharEventCallback(tSysCharEventFn pfn, uint_ptr userData)
{
   g_pfnCharCallback = pfn;
   g_charCallbackUserData = userData;
}

void SysSetKeyEventCallback(tSysKeyEventFn pfn, uint_ptr userData)
{
   g_pfnKeyCallback = pfn;
   g_keyCallbackUserData = userData;
}

void SysSetMouseMoveCallback(tSysMouseEventFn pfn, uint_ptr userData)
{
   g_pfnMouseMoveCallback = pfn;
   g_mouseMoveCallbackUserData = userData;
}

void SysSetMouseEventCallback(tSysMouseEventFn pfn, uint_ptr userData)
{
   g_pfnMouseCallback = pfn;
   g_mouseCallbackUserData = userData;
}

tSysFrameFn SysSetFrameCallback(tSysFrameFn pfn)
{
   tSysFrameFn pfnFormer = g_pfnFrameCallback;
   g_pfnFrameCallback = pfn;
   return pfnFormer;
}

tSysFrameFn SysGetFrameCallback()
{
   return g_pfnFrameCallback;
}

tSysResizeFn SysSetResizeCallback(tSysResizeFn pfn)
{
   tSysResizeFn pfnFormer = g_pfnResizeCallback;
   g_pfnResizeCallback = pfn;
   return pfnFormer;
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSysTestReporter
//

class cSysTestReporter : public UnitTest::TestReporter
{
public:
   cSysTestReporter();
   virtual ~cSysTestReporter();

   virtual void ReportTestStart(char const* testName);
   virtual void ReportFailure(char const* file, int line, char const* testName, char const* failure);
   virtual void ReportTestFinish(char const* testName, float secondsElapsed);
   virtual void ReportSummary(int testCount, int failureCount, float secondsElapsed);

private:
   class cTestFailure
   {
   public:
      cTestFailure(const char * file, int line, const char * testName, const char * failure)
       : m_file((file != NULL) ? file : "")
       , m_line(line)
       , m_testName((testName != NULL) ? testName : "")
       , m_failure((failure != NULL) ? failure : "")
      {
      }

      cStr m_file, m_testName, m_failure;
      int m_line;
   };

   std::list<cTestFailure> m_failures;
};

////////////////////////////////////////

cSysTestReporter::cSysTestReporter()
{
}

////////////////////////////////////////

cSysTestReporter::~cSysTestReporter()
{
}

////////////////////////////////////////

void cSysTestReporter::ReportTestStart(char const* testName)
{
   if (LOG_IS_CHANNEL_ENABLED(VerboseUnitTests))
   {
      LogMsgNoFL1(kInfo, "Test \"%s\" begin\n", testName);
   }
}

////////////////////////////////////////

void cSysTestReporter::ReportFailure(char const* file, int line, char const* testName, char const* failure)
{
   m_failures.push_back(cTestFailure(file, line, testName, failure));
}

////////////////////////////////////////

void cSysTestReporter::ReportTestFinish(char const* testName, float secondsElapsed)
{
   if (LOG_IS_CHANNEL_ENABLED(VerboseUnitTests))
   {
      LogMsgNoFL2(kInfo, "Test \"%s\" end (%f seconds elapsed)\n", testName, secondsElapsed);
   }
}

////////////////////////////////////////

void cSysTestReporter::ReportSummary(int testCount, int failureCount, float secondsElapsed)
{
   Assert(m_failures.size() == failureCount);
   if (failureCount > 0)
   {
      LogMsgNoFL2(kError, "%d of %d UNIT TESTS FAILED!\n", failureCount, testCount);
      std::list<cTestFailure>::const_iterator iter = m_failures.begin(), end = m_failures.end();
      for (; iter != end; ++iter)
      {
         techlog.Print(iter->m_file.c_str(), iter->m_line, kError,
            "Failure in %s: %s\n", iter->m_testName.c_str(), iter->m_failure.c_str());
      }
   }
   else
   {
      LogMsgNoFL2(kInfo, "%d unit tests succeeded (%f seconds elapsed)\n", testCount, secondsElapsed);
   }
}


///////////////////////////////////////////////////////////////////////////////

static tResult SysRunUnitTestPP()
{
   using namespace UnitTest;
   cSysTestReporter reporter;
   const TestList & tests = Test::GetTestList();
   int nFailures = RunAllTests(reporter, tests);
   if (nFailures > 0)
   {
      return E_FAIL;
   }
   return S_OK;
}

#else

static tResult SysRunUnitTestPP()
{
   return S_OK;
}

#endif // HAVE_UNITTESTPP


///////////////////////////////////////////////////////////////////////////////

tResult SysRunUnitTests()
{
#ifdef HAVE_UNITTESTPP
   if (FAILED(SysRunUnitTestPP()))
   {
      return E_FAIL;
   }
#endif

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

static double fpsLast = 0;
static double fpsWorst = 99999;//DBL_MAX;
static double fpsBest = 0;
static double fpsAverage = 0;

void SysUpdateFrameStats()
{
   static double lastTime = 0;
   static double frameCount = 0;

   double time = TimeGetSecs();
   double elapsed = time - lastTime;
   frameCount++;

   double fps = 0;
   if (elapsed >= 0.5) // update about 2x per second
   {
      if (lastTime != 0.0)
      {
         double fps = frameCount / elapsed;
         if (fpsAverage == 0)
         {
            fpsAverage = fps;
         }
         else
         {
            fpsAverage = (fps + fpsLast) * 0.5;
         }
         if (fps > fpsBest)
         {
            fpsBest = fps;
         }
         if (fps < fpsWorst)
         {
            fpsWorst = fps;
         }
         fpsLast = fps;
      }
      lastTime = time;
      frameCount = 0;
   }
}

///////////////////////////////////////////////////////////////////////////////

void SysReportFrameStats(tChar * psz, ulong max)
{
   if (psz != NULL)
   {
#ifdef _WIN32
      _snprintf(psz, max,
         "%.2f fps\n"
         "%.2f worst\n"
         "%.2f best\n"
         "%.2f average",
         fpsLast, 
         fpsWorst,
         fpsBest, 
         fpsAverage);
#else
      // TODO: figure out a way to speed this up so the #ifdef and _snprintf call aren't needed
      cStr temp;
      Sprintf(&temp,
         "%.2f fps\n"
         "%.2f worst\n"
         "%.2f best\n"
         "%.2f average",
         fpsLast, 
         fpsWorst,
         fpsBest, 
         fpsAverage);
#endif
   }
}

///////////////////////////////////////////////////////////////////////////////
