
#ifndef FAILURE_H
#define FAILURE_H

#include "CppUnitLite2Dll.h"

#include <iosfwd>


class CPPUNITLITE2_API Failure
{
public:
    Failure (const char* condition, const char* testName, 
             const char* fileName, int lineNumber);

    const char* Condition() const;

private:
    friend std::ostream& operator<< (std::ostream& stream, const Failure& failure);
    friend std::wostream& operator<< (std::wostream& stream, const Failure& failure);
    
    const char* m_condition;
    const char* m_testName;
    const char* m_fileName;
    int m_lineNumber;
};


#endif

