///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_DICTIONARY_H
#define INCLUDED_DICTIONARY_H

#include "tech/dictionaryapi.h"
#include "tech/multivar.h"
#include "tech/techstring.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cStrLessNoCase
//

class cStrLessNoCase
{
public:
   bool operator()(const cStr & lhs, const cStr & rhs) const;
};

///////////////////////////////////////

inline bool cStrLessNoCase::operator()(const cStr & lhs, const cStr & rhs) const
{
   return (_tcsicmp(lhs.c_str(), rhs.c_str()) < 0) ? true : false;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cDictionary
//

class cDictionary : public cComAggregableObject<IMPLEMENTS(IDictionary)>
{
   cDictionary(const cDictionary &);
   const cDictionary & operator =(const cDictionary &);

   friend tResult DictionaryCreate(tPersistence persist, IDictionary * * ppDictionary);
   friend IUnknown * DictionaryCreate(tPersistence defaultPersist, IUnknown * pUnkOuter);

protected:
   cDictionary(tPersistence defaultPersist, IUnknown * pUnkOuter = NULL);
   virtual ~cDictionary();

public:
   virtual tResult Get(const tChar * pszKey, tChar * pVal, int maxLength, tPersistence * pPersist = NULL);
   virtual tResult Get(const tChar * pszKey, cStr * pVal, tPersistence * pPersist = NULL);
   virtual tResult Get(const tChar * pszKey, int * pVal, tPersistence * pPersist = NULL);
   virtual tResult Get(const tChar * pszKey, float * pVal, tPersistence * pPersist = NULL);
   virtual tResult Get(const tChar * pszKey, double * pVal, tPersistence * pPersist = NULL);
   virtual tResult Get(const tChar * pszKey, cMultiVar * pVal, tPersistence * pPersist = NULL);

   virtual tResult Set(const tChar * pszKey, const char * val, tPersistence persist = kUseDefault);
   virtual tResult Set(const tChar * pszKey, const wchar_t * val, tPersistence persist = kUseDefault);
   virtual tResult Set(const tChar * pszKey, int val, tPersistence persist = kUseDefault);
   virtual tResult Set(const tChar * pszKey, float val, tPersistence persist = kUseDefault);
   virtual tResult Set(const tChar * pszKey, double val, tPersistence persist = kUseDefault);

   virtual tResult Delete(const tChar * pszKey);

   virtual tResult IsSet(const tChar * pszKey);

   virtual tResult GetKeys(std::list<cStr> * pKeys);

   virtual void Clear();

   virtual tResult Clone(IDictionary * * ppDictionary) const;

private:
   tResult GetPersistence(const tChar * pszKey, tPersistence * pPersist);

   typedef std::map<cStr, cMultiVar, cStrLessNoCase> tMap;
   tMap m_vars;

   typedef std::map<cStr, tPersistence, cStrLessNoCase> tPersistenceMap;
   tPersistenceMap m_persistenceMap;

   tPersistence m_defaultPersist;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_DICTIONARY_H
