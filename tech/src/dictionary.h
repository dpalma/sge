///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_DICTIONARY_H
#define INCLUDED_DICTIONARY_H

#include "dictionaryapi.h"
#include "techstring.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cDictionary
//

class cDictionary : public cComObject<IMPLEMENTS(IDictionary)>
{
public:
   cDictionary(tPersistence defaultPersist);
   virtual ~cDictionary();

   virtual tResult Get(const tChar * pszKey, tChar * pVal, int maxLength, tPersistence * pPersist = NULL);
   virtual tResult Get(const tChar * pszKey, cStr * pVal, tPersistence * pPersist = NULL);
   virtual tResult Get(const tChar * pszKey, int * pVal, tPersistence * pPersist = NULL);
   virtual tResult Get(const tChar * pszKey, float * pVal, tPersistence * pPersist = NULL);

   virtual tResult Set(const tChar * pszKey, const tChar * val, tPersistence persist = kPermanent);
   virtual tResult Set(const tChar * pszKey, int val, tPersistence persist = kPermanent);
   virtual tResult Set(const tChar * pszKey, float val, tPersistence persist = kPermanent);

   virtual tResult Delete(const tChar * pszKey);

   virtual tResult IsSet(const tChar * pszKey);

   virtual tResult GetKeys(std::list<cStr> * pKeys);

private:
   tResult GetPersistence(const tChar * pszKey, tPersistence * pPersist);

   typedef std::map<cStr, cStr, cStrLessNoCase> tMap;
   tMap m_vars;

   typedef std::map<cStr, tPersistence, cStrLessNoCase> tPersistenceMap;
   tPersistenceMap m_persistenceMap;

   tPersistence m_defaultPersist;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_DICTIONARY_H
