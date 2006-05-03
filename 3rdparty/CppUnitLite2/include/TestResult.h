
#ifndef TESTRESULT_H
#define TESTRESULT_H

#include "CppUnitLite2Dll.h"

class Failure;

class CPPUNITLITE2_API TestResult
{
public:
    TestResult ();
    virtual ~TestResult();

    virtual void TestBegin (const char * name);
    virtual void TestEnd (const char * name);
    virtual void TestWasRun ();
    virtual void StartTests ();
    virtual void AddFailure (const Failure & failure);
    virtual void EndTests ();

    int FailureCount() const;
    int TestCount() const;

protected:
    int m_failureCount;
    int m_testCount;
    long int m_startTime;
    float m_secondsElapsed;
};

#endif
