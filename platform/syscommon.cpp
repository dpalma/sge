///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "platform/sys.h"

#include "tech/techtime.h"

#ifdef HAVE_UNITTESTPP
#include "Test.h"
#include "TestDetails.h"
#include "TestReporter.h"
#include "TestRunner.h"
#endif

#include <cstdlib>
#include <list>

#include "tech/dbgalloc.h" // must be last header

#ifdef HAVE_UNITTESTPP
using namespace UnitTest;
#endif

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

   virtual void ReportTestStart(TestDetails const & test);
   virtual void ReportFailure(TestDetails const & test, char const * failure);
   virtual void ReportTestFinish(TestDetails const & test, float secondsElapsed);
   virtual void ReportSummary(int totalTestCount, int failedTestCount, int failureCount, float secondsElapsed);

private:
   class cTestFailure
   {
   public:
      cTestFailure(const TestDetails & details, const char * failure)
       : m_testName(details.testName)
       , m_suiteName(details.suiteName)
       , m_filename(details.filename)
       , m_lineNumber(details.lineNumber)
       , m_failure((failure != NULL) ? failure : "")
      {
      }

      std::string m_testName;
      std::string m_suiteName;
      std::string m_filename;
      int m_lineNumber;
      std::string m_failure;
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

void cSysTestReporter::ReportTestStart(TestDetails const & test)
{
   if (LOG_IS_CHANNEL_ENABLED(VerboseUnitTests))
   {
      LogMsgNoFL1(kInfo, "Test \"%s\" begin\n", test.testName);
   }
}

////////////////////////////////////////

void cSysTestReporter::ReportFailure(TestDetails const & test, char const * failure)
{
   m_failures.push_back(cTestFailure(test, failure));
}

////////////////////////////////////////

void cSysTestReporter::ReportTestFinish(TestDetails const & test, float secondsElapsed)
{
   if (LOG_IS_CHANNEL_ENABLED(VerboseUnitTests))
   {
      LogMsgNoFL2(kInfo, "Test \"%s\" end (%f seconds elapsed)\n", test.testName, secondsElapsed);
   }
}

////////////////////////////////////////

void cSysTestReporter::ReportSummary(int totalTestCount, int failedTestCount, int failureCount, float secondsElapsed)
{
   Assert(m_failures.size() == failureCount);
   if (failureCount > 0)
   {
      LogMsgNoFL2(kError, "%d of %d UNIT TESTS FAILED!\n", failedTestCount, totalTestCount);
      std::list<cTestFailure>::const_iterator iter = m_failures.begin(), end = m_failures.end();
      for (; iter != end; ++iter)
      {
         techlog.Print(iter->m_filename.c_str(), iter->m_lineNumber, kError,
            "Failure in %s: %s\n", iter->m_testName.c_str(), iter->m_failure.c_str());
      }
   }
   else
   {
      LogMsgNoFL2(kInfo, "%d unit tests succeeded (%f seconds elapsed)\n", totalTestCount, secondsElapsed);
   }
}


///////////////////////////////////////////////////////////////////////////////

static tResult SysRunUnitTestPP()
{
   cSysTestReporter reporter;
   const TestList & tests = Test::GetTestList();
   int nFailures = RunAllTests(reporter, tests, NULL);
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
