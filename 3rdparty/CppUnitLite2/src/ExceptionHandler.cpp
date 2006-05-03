#include "ExceptionHandler.h"
#include "TestResult.h"
#include "Failure.h"
#include "TestException.h"



namespace ExceptionHandler {


namespace {
    bool g_bHandleExceptions = true;
}


bool IsOn ()
{
    return g_bHandleExceptions;
}

void TurnOn (bool bOn)
{
    g_bHandleExceptions = bOn;
}


void Handle (TestResult& result, const TestException& exception, 
             const char* testname, const char* filename, int linenumber )
{
    char msg[4096];
#if _MSC_VER >= 1400
    sprintf_s( msg, sizeof(msg) / sizeof(msg[0]), "Raised exception %s from:\n  %s(%i)", exception.GetMessage(), exception.GetFile(), exception.GetLine() );
#else
    sprintf( msg, "Raised exception %s from:\n  %s(%i)", exception.GetMessage(), exception.GetFile(), exception.GetLine() );
#endif
    result.AddFailure (Failure (msg, testname, filename, linenumber));
}

void Handle (TestResult& result, const char* condition, 
             const char* testname, const char* filename, int linenumber)
{
    if (!g_bHandleExceptions) 
        throw;
        
    char msg[1024] = "Unhandled exception ";
#if _MSC_VER >= 1400
    strcat_s(msg, sizeof(msg) / sizeof(msg[0]), condition);
#else
    strcat(msg, condition);
#endif
    result.AddFailure (Failure (msg, testname, filename, linenumber));
}



}
