///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_CONFIGAPI_H
#define INCLUDED_CONFIGAPI_H

#include "techdll.h"
#include "comtools.h"

F_DECLARE_INTERFACE(IConfig);
F_DECLARE_INTERFACE(IConfigStore);
class cFileSpec;
class cStr;

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

TECH_API void ParseConfigLine(char * pBuffer, const char * * ppKey,
                              const char * * ppVal, const char * * ppComment);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IConfig
//

interface IConfig : IUnknown
{
   virtual tResult Get(const char * name, char * val, int maxlen) = 0;
   virtual tResult Get(const char * name, cStr * val) = 0;
   virtual tResult Get(const char * name, int * val) = 0;
   virtual tResult Get(const char * name, float * val) = 0;

   virtual tResult Set(const char * name, const char * val) = 0;
   virtual tResult Set(const char * name, int val) = 0;
   virtual tResult Set(const char * name, float val) = 0;

   virtual tResult UnSet(const char * name) = 0;

   virtual BOOL IsSet(const char * name) = 0;
   virtual BOOL IsTrue(const char * name) = 0;

   virtual void IterCfgVarBegin(HANDLE * phIter) = 0;
   virtual BOOL IterNextCfgVar(HANDLE * phIter, cStr * pName, cStr * pValue) = 0;
   virtual void IterCfgVarEnd(HANDLE * phIter) = 0;

   virtual tResult ParseCmdLine(int argc, char *argv[]) = 0;
};

interface IConfigStore : IUnknown
{
   virtual tResult Load(IConfig * pConfig) = 0;
   virtual tResult Save(IConfig * pConfig) = 0;
   virtual tResult MergeSave(IConfig * pConfig) = 0;
};

///////////////////////////////////////

TECH_API IConfig * CreateGenericConfig();

///////////////////////////////////////

TECH_API IConfigStore * CreateTextConfigStore(const cFileSpec & file);
TECH_API IConfigStore * CreateIniSectionConfigStore(const cFileSpec & file, const char * pszSection);

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

inline BOOL ConfigIsTrue(const char * pszName)
{
   if (g_pConfig == NULL)
      return FALSE;
   return g_pConfig->IsTrue(pszName);
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_CONFIGAPI_H
