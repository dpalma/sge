///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_CONFIG_H
#define INCLUDED_CONFIG_H

#include "configapi.h"

#include "str.h"
#include <map>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cConfig
//

class cConfig : public cComObject<IMPLEMENTS(IConfig)>
{
public:
   cConfig();
   virtual ~cConfig();

   virtual tResult Get(const char * name, char * pVal, int maxLen);
   virtual tResult Get(const char * name, cStr * pVal);
   virtual tResult Get(const char * name, int * pVal);
   virtual tResult Get(const char * name, float * pVal);

   virtual tResult Set(const char * name, const char * val);
   virtual tResult Set(const char * name, int val);
   virtual tResult Set(const char * name, float val);

   virtual tResult UnSet(const char * name);

   virtual BOOL IsSet(const char * name);
   virtual BOOL IsTrue(const char * name);

   virtual void IterCfgVarBegin(HANDLE * phIter);
   virtual BOOL IterNextCfgVar(HANDLE * phIter, cStr * pName, cStr * pValue);
   virtual void IterCfgVarEnd(HANDLE * phIter);

   virtual tResult ParseCmdLine(int argc, char *argv[]);

private:
   class cStringLessNoCase
   {
   public:
      bool operator()(const cStr & lhs, const cStr & rhs) const;
   };

   typedef std::map<cStr, cStr, cStringLessNoCase> tMap;
   tMap m_vars;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_CONFIG_H
