///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCRIPT_H
#define INCLUDED_SCRIPT_H

#include "scriptapi.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

#define SCRIPT_FUNCTION_NAME(name) name

#define SCRIPT_DEFINE_FUNCTION_NO_AUTOADD(name) \
   int SCRIPT_FUNCTION_NAME(name)(int argc, const tScriptVar * argv, int nMaxResults, tScriptVar * pResults)

#define SCRIPT_DEFINE_FUNCTION(name) \
   SCRIPT_DEFINE_FUNCTION_NO_AUTOADD(name); \
   AUTOADD_SCRIPTFUNCTION(name, SCRIPT_FUNCTION_NAME(name)); \
   SCRIPT_DEFINE_FUNCTION_NO_AUTOADD(name)

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCRIPT_H
