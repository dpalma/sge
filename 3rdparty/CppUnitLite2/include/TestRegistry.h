
#ifndef TESTREGISTRY_H
#define TESTREGISTRY_H

#include "CppUnitLite2Dll.h"


#define MAX_TESTS   5000


class Test;
class TestResult;

class CPPUNITLITE2_API TestRegistry
{
public:
    static TestRegistry& Instance();
    static void Destroy();
    
    void Add (Test* test);
    void Run (TestResult& result);
        
private:
    TestRegistry();

    Test* m_tests[MAX_TESTS];
    int m_testCount;
};



#endif

