///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_DICTREGSTORE_H
#define INCLUDED_DICTREGSTORE_H

#include "tech/dictionaryapi.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cDictionaryRegistryStore
//

class cDictionaryRegistryStore : public cComObject<IMPLEMENTS(IDictionaryStore)>
{
public:
   cDictionaryRegistryStore(HKEY hKey, bool bTakeOwnership = true);
   ~cDictionaryRegistryStore();

   virtual tResult Load(IDictionary * pDictionary);
   virtual tResult Save(IDictionary * pDictionary);
   virtual tResult MergeSave(IDictionary * pDictionary);

private:
   HKEY m_hKey;
   bool m_bCloseKeyOnDestroy;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_DICTREGSTORE_H
