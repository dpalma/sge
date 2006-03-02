///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "sys.h"

#include "techtime.h"

#ifdef HAVE_CPPUNITLITE2
#include "CppUnitLite2.h"
#endif

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/ui/text/TestRunner.h>
#endif

#include <cstdlib>
#include <list>

#include "dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(VerboseUnitTests);

///////////////////////////////////////////////////////////////////////////////

tSysKeyEventFn       g_pfnKeyCallback = NULL;
tSysMouseEventFn     g_pfnMouseCallback = NULL;
tSysFrameFn          g_pfnFrameCallback = NULL;
tSysResizeFn         g_pfnResizeCallback = NULL;

uint_ptr             g_keyCallbackUserData = 0;
uint_ptr             g_mouseCallbackUserData = 0;

///////////////////////////////////////////////////////////////////////////////

void SysSetKeyEventCallback(tSysKeyEventFn pfn, uint_ptr userData)
{
   g_pfnKeyCallback = pfn;
   g_keyCallbackUserData = userData;
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


#ifdef HAVE_CPPUNITLITE2

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSysTestResult
//

class cSysTestResult : public TestResult
{
public:
   virtual void TestBegin(const char * pszTestName);
   virtual void TestEnd(const char * pszTestName);
   virtual void AddFailure(const Failure & failure);
   virtual void EndTests();
   std::list<Failure>::const_iterator BeginFailures() const { return m_failures.begin(); }
   std::list<Failure>::const_iterator EndFailures() const { return m_failures.end(); }
private:
   std::list<Failure> m_failures;
};

////////////////////////////////////////

void cSysTestResult::TestBegin(const char * pszTestName)
{
   if (LOG_IS_CHANNEL_ENABLED(VerboseUnitTests))
   {
      techlog.Print(kInfo, "Test \"%s\" begin\n", pszTestName);
   }
}

////////////////////////////////////////

void cSysTestResult::TestEnd(const char * pszTestName)
{
   if (LOG_IS_CHANNEL_ENABLED(VerboseUnitTests))
   {
      techlog.Print(kInfo, "Test \"%s\" end\n", pszTestName);
   }
}

////////////////////////////////////////

void cSysTestResult::AddFailure(const Failure & failure) 
{
   TestResult::AddFailure(failure);
   m_failures.push_back(failure);
}

////////////////////////////////////////

void cSysTestResult::EndTests() 
{
   TestResult::EndTests();

   if (FailureCount() > 0)
   {
      techlog.Print(kInfo, "%d of %d UNIT TESTS FAILED!\n", FailureCount(), TestCount());
   }
   else
   {
      techlog.Print(kInfo, "%d unit tests succeeded (%f seconds elapsed)\n", TestCount(), m_secondsElapsed);
   }
}

///////////////////////////////////////

static tResult SysRunCppUnitLite2()
{
   cSysTestResult result;
   TestRegistry::Instance().Run(result);
   TestRegistry::Destroy();
   if (result.FailureCount() > 0)
   {
      std::list<Failure>::const_iterator iter = result.BeginFailures();
      std::list<Failure>::const_iterator end = result.EndFailures();
      for (; iter != end; iter++)
      {
         const Failure & failure = *iter;
         techlog.Print(kError, "%s\n", failure.Condition());
      }
      return E_FAIL;
   }
   return S_OK;
}

#else

static tResult SysRunCppUnitLite2()
{
   return S_OK;
}

#endif // HAVE_CPPUNITLITE2

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT
tResult SysRunCppUnit()
{
   CppUnit::TextUi::TestRunner runner;
   runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
   runner.run();
   if (runner.result().testFailuresTotal() > 0)
   {
      techlog.Print(kError, "%d LEGACY UNIT TESTS FAILED!\n", runner.result().testFailuresTotal());
      CppUnit::TestResultCollector::TestFailures::const_iterator iter;
      for (iter = runner.result().failures().begin(); iter != runner.result().failures().end(); iter++)
      {
         techlog.Print(kError, "%s(%d) : %s : %s\n",
            (*iter)->sourceLine().fileName().c_str(),
            (*iter)->sourceLine().isValid() ? (*iter)->sourceLine().lineNumber() : -1,
            (*iter)->failedTestName().c_str(),
            (*iter)->thrownException()->what());
      }
      return E_FAIL;
   }
   else
   {
      techlog.Print(kInfo, "%d legacy unit tests succeeded\n", runner.result().tests().size());
      return S_OK;
   }
}
#endif

///////////////////////////////////////////////////////////////////////////////

tResult SysRunUnitTests()
{
#ifdef HAVE_CPPUNITLITE2
   if (FAILED(SysRunCppUnitLite2()))
   {
      return E_FAIL;
   }
#endif

#ifdef HAVE_CPPUNIT
   if (FAILED(SysRunCppUnit()))
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
