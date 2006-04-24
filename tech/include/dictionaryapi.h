///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_DICTIONARYAPI_H
#define INCLUDED_DICTIONARYAPI_H

#include "techdll.h"
#include "comtools.h"
#include "techstring.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif

class cFileSpec;
class cMultiVar;

F_DECLARE_INTERFACE(IDictionary);
F_DECLARE_INTERFACE(IDictionaryStore);

F_DECLARE_HANDLE(HKEY);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IDictionary
//

enum ePersistence
{
   kUseDefault,
   kPermanent,
   kTransitory,
};

typedef ePersistence tPersistence;

interface IDictionary : IUnknown
{
   virtual tResult Get(const tChar * pszKey, tChar * val, int maxLength, tPersistence * pPersist = NULL) = 0;
   virtual tResult Get(const tChar * pszKey, cStr * val, tPersistence * pPersist = NULL) = 0;
   virtual tResult Get(const tChar * pszKey, int * val, tPersistence * pPersist = NULL) = 0;
   virtual tResult Get(const tChar * pszKey, float * val, tPersistence * pPersist = NULL) = 0;
   virtual tResult Get(const tChar * pszKey, double * pVal, tPersistence * pPersist = NULL) = 0;
   virtual tResult Get(const tChar * pszKey, cMultiVar * pVal, tPersistence * pPersist = NULL) = 0;

   virtual tResult Set(const tChar * pszKey, const char * val, tPersistence persist = kUseDefault) = 0;
   virtual tResult Set(const tChar * pszKey, const wchar_t * val, tPersistence persist = kUseDefault) = 0;
   virtual tResult Set(const tChar * pszKey, int val, tPersistence persist = kUseDefault) = 0;
   virtual tResult Set(const tChar * pszKey, float val, tPersistence persist = kUseDefault) = 0;
   virtual tResult Set(const tChar * pszKey, double val, tPersistence persist = kUseDefault) = 0;

   virtual tResult Delete(const tChar * pszKey) = 0;

   virtual tResult IsSet(const tChar * pszKey) = 0;

   virtual tResult GetKeys(std::list<cStr> * pKeys) = 0;

   virtual void Clear() = 0;

   virtual tResult Clone(IDictionary * * ppDictionary) const = 0;
};

///////////////////////////////////////

TECH_API IDictionary * DictionaryCreate(tPersistence defaultPersist = kPermanent);
TECH_API IUnknown * DictionaryCreate(tPersistence defaultPersist, IUnknown * pUnkOuter);

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
TECH_API tResult DictionaryStoreCreate(HKEY hKey, const tChar * pszSubKey, bool bReadOnly, IDictionaryStore * * ppStore);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_DICTIONARYAPI_H
