///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_CONFIGAPI_H
#define INCLUDED_CONFIGAPI_H

#include "techdll.h"
#include "dictionaryapi.h"

#ifdef _MSC_VER
#pragma once
#endif

typedef IDictionary IConfig;

//////////////////////////////////////////////////////////////////////////////

TECH_API tResult ParseCommandLine(int argc, char *argv[], IConfig * pConfig);

//////////////////////////////////////////////////////////////////////////////

extern TECH_API IConfig * g_pConfig;

///////////////////////////////////////

template <typename T>
tResult ConfigGet(const char * pszName, T * pValue)
{
   if (g_pConfig == NULL)
      return E_FAIL;
   return g_pConfig->Get(pszName, pValue);
}

template <typename T>
tResult ConfigSet(const char * pszName, T value)
{
   if (g_pConfig == NULL)
      return E_FAIL;
   return g_pConfig->Set(pszName, value);
}

inline tResult ConfigGetString(const char * pszName, char * pValue, int maxValueLen)
{
   if (g_pConfig == NULL)
      return E_FAIL;
   return g_pConfig->Get(pszName, pValue, maxValueLen);
}

TECH_API bool ConfigIsTrue(const char * pszName);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_CONFIGAPI_H
