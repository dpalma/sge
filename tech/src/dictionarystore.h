///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_DICTIONARYSTORE_H
#define INCLUDED_DICTIONARYSTORE_H

#include "dictionaryapi.h"
#include "filespec.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cDictionaryTextStore
//

class cDictionaryTextStore : public cComObject<IMPLEMENTS(IDictionaryStore)>
{
public:
   cDictionaryTextStore(const cFileSpec & file);

   virtual tResult Load(IDictionary * pDictionary);
   virtual tResult Save(IDictionary * pDictionary);
   virtual tResult MergeSave(IDictionary * pDictionary);

private:
   cFileSpec m_file;
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cDictionaryIniStore
//

class cDictionaryIniStore : public cComObject<IMPLEMENTS(IDictionaryStore)>
{
public:
   cDictionaryIniStore(const cFileSpec & file, const tChar * pszSection);

   virtual tResult Load(IDictionary * pDictionary);
   virtual tResult Save(IDictionary * pDictionary);
   virtual tResult MergeSave(IDictionary * pDictionary);

private:
   cFileSpec m_file;
   tChar m_szSection[256];
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_DICTIONARYSTORE_H
