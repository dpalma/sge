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

tResult ParseCommandLine(int argc, tChar *argv[], IConfig * pConfig);

//////////////////////////////////////////////////////////////////////////////

extern TECH_API IConfig * g_pConfig;

///////////////////////////////////////

template <typename T>
tResult ConfigGet(const tChar * pszName, T * pValue)
{
   if (g_pConfig == NULL)
      return E_FAIL;
   return g_pConfig->Get(pszName, pValue);
}

template <typename T>
tResult ConfigSet(const tChar * pszName, T value)
{
   if (g_pConfig == NULL)
      return E_FAIL;
   return g_pConfig->Set(pszName, value);
}

inline tResult ConfigGetString(const tChar * pszName, tChar * pValue, int maxValueLen)
{
   if (g_pConfig == NULL)
      return E_FAIL;
   return g_pConfig->Get(pszName, pValue, maxValueLen);
}

bool ConfigIsTrue(const tChar * pszName);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_CONFIGAPI_H
