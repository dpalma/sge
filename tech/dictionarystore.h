///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_DICTIONARYSTORE_H
#define INCLUDED_DICTIONARYSTORE_H

#include "tech/dictionaryapi.h"
#include "tech/filespec.h"

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
   cDictionaryTextStore(IReader * pReader);

   virtual tResult Load(IDictionary * pDictionary);
   virtual tResult Save(IDictionary * pDictionary);
   virtual tResult MergeSave(IDictionary * pDictionary);

private:
   cFileSpec m_file;
   cAutoIPtr<IReader> m_pReader;
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
   cStr m_section;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_DICTIONARYSTORE_H
