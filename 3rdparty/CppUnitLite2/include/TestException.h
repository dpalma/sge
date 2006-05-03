#ifndef TEST_EXCEPTION_H_
#define TEST_EXCEPTION_H_

#include "CppUnitLite2Dll.h"

class CPPUNITLITE2_API TestException
{
public:
    TestException( const char* file, int line, const char* message );

    const char* GetFile() const { return file; }
    int GetLine() const { return line; }
    const char* GetMessage() const { return message; }

private:
    const char* file;
    int line;
    const char* message;
};


#endif
