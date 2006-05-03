
// Central point to turn on and off exception handling for the framework. 
// You might want to turn it off automatically if a debugger is present.
//
// You also should hook your own assert to disable it and have it go through the
// handler instead so any asserts encountered are reported correctly as test failures.


#ifndef EXCEPTIONHANDLER_H_
#define EXCEPTIONHANDLER_H_

#include "CppUnitLite2Dll.h"

class TestResult;
class TestException;

namespace ExceptionHandler {

bool IsOn ();
void TurnOn (bool bOn = true);

CPPUNITLITE2_API void Handle (TestResult& result, const TestException& exception, 
                              const char* testname, const char* filename, int linenumber );
CPPUNITLITE2_API void Handle (TestResult& result, const char* condition, 
                              const char* testname, const char* filename, int linenumber);
 

                      
}

                      
#endif
