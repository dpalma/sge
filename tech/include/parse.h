//////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_PARSE_H
#define INCLUDED_PARSE_H

#include "techdll.h"

#include <vector>
#include <string>

#ifdef _MSC_VER
#pragma once
#endif

//////////////////////////////////////////////////////////////////////////////

enum eParseTupleError
{
   kPTE_InvalidArg = -1,
   kPTE_UnmatchedBrackets = -2,
};

int TECH_API ParseTuple(const char * psz, std::vector<std::string> * pStrings);
int TECH_API ParseTuple(const char * psz, double * pNumbers, int nMaxNumbers);
int TECH_API ParseTuple(const char * psz, float * pNumbers, int nMaxNumbers);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_PARSE_H
