///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_DICTIONARYAPI_H
#define INCLUDED_DICTIONARYAPI_H

#include "techdll.h"
#include "comtools.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif

class cStr;
class cFileSpec;

F_DECLARE_INTERFACE(IDictionary);
F_DECLARE_INTERFACE(IDictionaryStore);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IDictionary
//

interface IDictionary : IUnknown
{
   virtual tResult Get(const tChar * pszKey, tChar * val, int maxLength) = 0;
   virtual tResult Get(const tChar * pszKey, cStr * val) = 0;
   virtual tResult Get(const tChar * pszKey, int * val) = 0;
   virtual tResult Get(const tChar * pszKey, float * val) = 0;

   virtual tResult Set(const tChar * pszKey, const tChar * val) = 0;
   virtual tResult Set(const tChar * pszKey, int val) = 0;
   virtual tResult Set(const tChar * pszKey, float val) = 0;

   virtual tResult Delete(const tChar * pszKey) = 0;

   virtual tResult IsSet(const tChar * pszKey) = 0;

   virtual tResult GetKeys(std::list<cStr> * pKeys) = 0;
};

///////////////////////////////////////

TECH_API IDictionary * DictionaryCreate();

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IDictionaryStore
//

interface IDictionaryStore : IUnknown
{
   virtual tResult Load(IDictionary * pDictionary) = 0;
   virtual tResult Save(IDictionary * pDictionary) = 0;
   virtual tResult MergeSave(IDictionary * pDictionary) = 0;
};

///////////////////////////////////////

TECH_API IDictionaryStore * DictionaryStoreCreate(const cFileSpec & file);
TECH_API IDictionaryStore * DictionaryIniStoreCreate(const cFileSpec & file,
                                                     const tChar * pszSection);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_DICTIONARYAPI_H
