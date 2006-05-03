

#ifndef TESTRESULTDEBUGOUT_WIN32_H
#define TESTRESULTDEBUGOUT_WIN32_H

#include "CppUnitLite2Dll.h"

#include "TestResult.h"


class CPPUNITLITE2_API TestResultDebugOut : public TestResult
{
public:
    virtual void StartTests ();
    virtual void AddFailure (const Failure & failure);
    virtual void EndTests ();
};


#endif
