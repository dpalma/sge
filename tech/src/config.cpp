///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "config.h"

#include <cstdio>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cStackConfig
//

class cStackConfig : public cConfig
{
public:
   virtual ULONG STDMETHODCALLTYPE AddRef() { return 2; }
   virtual ULONG STDMETHODCALLTYPE Release() { return 1; }
};

cStackConfig g_config;
IConfig * g_pConfig = static_cast<IConfig *>(&g_config);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cConfig
//

///////////////////////////////////////

bool cConfig::cStringLessNoCase::operator()(const cStr & lhs, const cStr & rhs) const
{
   return (stricmp(lhs.c_str(), rhs.c_str()) < 0) ? true : false;
}

///////////////////////////////////////

cConfig::cConfig()
{
}

///////////////////////////////////////

cConfig::~cConfig()
{
}

///////////////////////////////////////

tResult cConfig::Get(const char * name, char * pVal, int maxLen)
{
   tMap::iterator iter = m_vars.find(name);
   if (iter != m_vars.end())
   {
      if (pVal != NULL && maxLen > 0)
      {
         strncpy(pVal, iter->second.c_str(), maxLen);
         pVal[maxLen-1] = '\0';
      }
      return S_OK;
   }
   return S_FALSE;
}

///////////////////////////////////////

tResult cConfig::Get(const char * name, cStr * pVal)
{
   tMap::iterator iter = m_vars.find(name);
   if (iter != m_vars.end())
   {
      if (pVal != NULL)
         *pVal = iter->second.c_str();
      return S_OK;
   }
   return S_FALSE;
}

///////////////////////////////////////

tResult cConfig::Get(const char * name, int * pVal)
{
   tMap::iterator iter = m_vars.find(name);
   if (iter != m_vars.end())
   {
      if (pVal != NULL)
         *pVal = atoi(iter->second.c_str());
      return S_OK;
   }
   return S_FALSE;
}

///////////////////////////////////////

tResult cConfig::Get(const char * name, float * pVal)
{
   tMap::iterator iter = m_vars.find(name);
   if (iter != m_vars.end())
   {
      if (pVal != NULL)
         *pVal = (float)atof(iter->second.c_str());
      return S_OK;
   }
   return S_FALSE;
}

///////////////////////////////////////

tResult cConfig::Set(const char * name, const char * val)
{
   m_vars.erase(name);
   m_vars.insert(std::make_pair(name, val));
   return S_OK;
}

///////////////////////////////////////

tResult cConfig::Set(const char * name, int val)
{
   char buffer[32];
   m_vars.erase(name);
   snprintf(buffer, _countof(buffer), "%d", val);
   m_vars.insert(std::make_pair(name, (const char *)buffer));
   return S_OK;
}

///////////////////////////////////////

tResult cConfig::Set(const char * name, float val)
{
   char buffer[64];
   snprintf(buffer, _countof(buffer), "%f", val);
   m_vars.erase(name);
   m_vars.insert(std::make_pair(name, (char *)buffer));
   return S_OK;
}

///////////////////////////////////////

tResult cConfig::UnSet(const char * name)
{
   return m_vars.erase(name) > 0 ? S_OK : S_FALSE;
}

///////////////////////////////////////

BOOL cConfig::IsSet(const char * name)
{
   return (m_vars.find(name) != m_vars.end());
}

///////////////////////////////////////

BOOL cConfig::IsTrue(const char * name)
{
   tMap::iterator iter = m_vars.find(name);
   if (iter != m_vars.end())
   {
      if (stricmp(iter->second.c_str(), "true") == 0)
         return TRUE;
      return atoi(iter->second.c_str());
   }
   return FALSE;
}

///////////////////////////////////////

void cConfig::IterCfgVarBegin(HANDLE * phIter)
{
   *phIter = new tMap::iterator(m_vars.begin());
}

///////////////////////////////////////

BOOL cConfig::IterNextCfgVar(HANDLE * phIter, cStr * pName, cStr * pValue)
{
   tMap::iterator * pIter = (tMap::iterator *)*phIter;
   if (*pIter != m_vars.end())
   {
      Assert(pName && pValue);
      *pName = (*pIter)->first.c_str();
      *pValue = (*pIter)->second.c_str();
      (*pIter)++;
      return TRUE;
   }
   return FALSE;
}

///////////////////////////////////////

void cConfig::IterCfgVarEnd(HANDLE * phIter)
{
   tMap::iterator * pIter = (tMap::iterator *)*phIter;
   delete pIter;
}

///////////////////////////////////////

// @TODO (dpalma 12/3/00): command-line variables should be set as
// "transient" when that facility is available
tResult cConfig::ParseCmdLine(int argc, char *argv[])
{
   cStr curKey, curVal;
   BOOL bLastWasKey = FALSE;

   for (int i = 0; i < argc; i++)
   {
      if (argv[i][0] == '+')
      {
         // set currently accumulating key/value
         if (curKey.GetLength() > 0)
         {
            Set(curKey, curVal);
            curKey.Empty();
            curVal.Empty();
         }

         const char * pszKey = NULL, * pszVal = NULL, * pszComment = NULL;
         ParseConfigLine(argv[i] + 1, &pszKey, &pszVal, &pszComment);
         if (pszKey && pszVal)
         {
            curKey = pszKey;
            curVal = (pszVal != NULL) ? pszVal : "1";
            bLastWasKey = TRUE;
         }
      }
      else if (argv[i][0] == '-')
      {
         UnSet(argv[i] + 1);
         bLastWasKey = FALSE;
      }
      else
      {
         if (!bLastWasKey)
         {
            curVal += " ";
            curVal += argv[i];
         }
         else
         {
            curVal = argv[i];
         }
         bLastWasKey = FALSE;
      }
   }

   // set final key
   if (curKey.GetLength() > 0)
   {
      Set(curKey, curVal);
   }

   return S_OK;
}

///////////////////////////////////////

IConfig * CreateGenericConfig()
{
   return new cConfig;
}

//////////////////////////////////////////////////////////////////////////////
