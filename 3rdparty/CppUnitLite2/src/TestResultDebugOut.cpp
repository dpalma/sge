#include "TestResultDebugOut.h"
#include "Failure.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <windows.h>
#include <tchar.h>


void TestResultDebugOut::StartTests ()
{
    OutputDebugString(_T("\n\nRunning unit tests...\n\n"));
}


void TestResultDebugOut::AddFailure (const Failure & failure) 
{
    TestResult::AddFailure(failure);

#ifdef _UNICODE
    std::wostringstream oss;
#else
    std::ostringstream oss;
#endif
    oss << failure;
    OutputDebugString(oss.str().c_str());
}

void TestResultDebugOut::EndTests () 
{
    TestResult::EndTests();

#ifdef _UNICODE
    std::wostringstream oss;
#else
    std::ostringstream oss;
#endif
    oss << m_testCount << _T(" tests run") << std::endl;
    if (m_failureCount > 0)
        oss << _T("****** There were ") << m_failureCount << _T(" failures.") << std::endl;
    else
        oss << _T("There were no test failures.") << std::endl;

    oss << _T("Test time: ") << std::setprecision(3) << m_secondsElapsed << _T(" seconds.") << std::endl;

    OutputDebugString(oss.str().c_str());
    OutputDebugString(_T("\n"));
}
