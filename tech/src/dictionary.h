///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_DICTIONARY_H
#define INCLUDED_DICTIONARY_H

#include "dictionaryapi.h"
#include "str.h"

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
   cDictionary();
   virtual ~cDictionary();

   virtual tResult Get(const tChar * pszKey, tChar * pVal, int maxLength);
   virtual tResult Get(const tChar * pszKey, cStr * pVal);
   virtual tResult Get(const tChar * pszKey, int * pVal);
   virtual tResult Get(const tChar * pszKey, float * pVal);

   virtual tResult Set(const tChar * pszKey, const tChar * val);
   virtual tResult Set(const tChar * pszKey, int val);
   virtual tResult Set(const tChar * pszKey, float val);

   virtual tResult Delete(const tChar * pszKey);

   virtual tResult IsSet(const tChar * pszKey);

   virtual tResult GetKeys(std::list<cStr> * pKeys);

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

#endif // !INCLUDED_DICTIONARY_H
