///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "dictionarystore.h"
#include "readwriteapi.h"

#ifdef HAVE_CPPUNITLITE2
#include "CppUnitLite2.h"
#endif

#include <locale>
#include <list>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(DictionaryStore);

static const tChar kCommentChar = _T('#');
static const tChar kSepChar = _T('=');
static const tChar kWhitespace[] = _T(" \t\r\n");

///////////////////////////////////////////////////////////////////////////////

static bool SplitString(const tChar * psz, tChar split, cStr * pLeft, cStr * pRight)
{
   Assert(psz != NULL);
   const tChar * pszSplit = _tcsrchr(psz, split);
   if (pszSplit != NULL)
   {
      if (pLeft != NULL)
      {
         *pLeft = cStr(psz, pszSplit - psz).c_str();
      }
      if (pRight != NULL)
      {
         *pRight = pszSplit + 1;
      }
      return true;
   }
   return false;
}

inline void TrimSpace(cStr * pStr)
{
   TrimLeadingSpace(pStr);
   TrimTrailingSpace(pStr);
}

bool ParseDictionaryLine(const tChar * psz, cStr * pKey, cStr * pValue, cStr * pComment)
{
   Assert(psz != NULL);
   Assert(pKey != NULL);
   Assert(pValue != NULL);
   cStr temp;
   if (SplitString(psz, kCommentChar, &temp, pComment))
   {
      psz = temp.c_str();
      if (pComment != NULL)
      {
         TrimSpace(pComment);
      }
   }
   if (SplitString(psz, kSepChar, pKey, pValue))
   {
      TrimSpace(pKey);
      TrimSpace(pValue);
      return true;
   }
   else
   {
      *pKey = psz;
      cStr::size_type index = pKey->find_first_of(kWhitespace);
      if (index == cStr::npos)
      {
         return true;
      }
      pKey->clear();
   }
   return false;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cDictionaryTextStore
//

///////////////////////////////////////

cDictionaryTextStore::cDictionaryTextStore(const cFileSpec & file)
 : m_file(file)
{
}

///////////////////////////////////////

tResult cDictionaryTextStore::Load(IDictionary * pDictionary)
{
   Assert(pDictionary != NULL);

   cAutoIPtr<IReader> pReader;
   if (FileReaderCreate(m_file, &pReader) == S_OK)
   {
      cStr line;
      while (pReader->Read(&line, '\n') == S_OK)
      {
         cStr key, value, comment;
         if (ParseDictionaryLine(line.c_str(), &key, &value, &comment)
             && !key.empty() && !value.empty())
         {
            DebugMsgEx2(DictionaryStore, "Read dictionary entry '%s' = '%s'\n", key.c_str(), value.c_str());
            pDictionary->Set(key.c_str(), value.c_str());
         }
      }
      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cDictionaryTextStore::Save(IDictionary * pDictionary)
{
   FILE * fp = _tfopen(m_file.CStr(), _T("w"));
   if (fp == NULL)
   {
      return E_FAIL;
   }

   tResult result = S_FALSE;

   std::list<cStr> keys;
   if (pDictionary->GetKeys(&keys) == S_OK)
   {
      std::list<cStr>::iterator iter;
      for (iter = keys.begin(); iter != keys.end(); iter++)
      {
         cStr value;
         tPersistence persist;
         if (pDictionary->Get(iter->c_str(), &value, &persist) == S_OK)
         {
            if (persist == kPermanent)
            {
               fprintf(fp, "%s=%s\n", iter->c_str(), value.c_str());
            }
         }
      }

      result = S_OK;
   }

   fclose(fp);

   return result;
}

///////////////////////////////////////

tResult cDictionaryTextStore::MergeSave(IDictionary * pDictionary)
{
   return E_NOTIMPL; // TODO
}

///////////////////////////////////////

IDictionaryStore * DictionaryStoreCreate(const cFileSpec & file)
{
   return static_cast<IDictionaryStore *>(new cDictionaryTextStore(file));
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cDictionaryIniStore
//

///////////////////////////////////////

static bool ParseIniSectionLine(const tChar * pszBuffer, cStr * pSection, cStr * pComment)
{
   if (pszBuffer == NULL || pSection == NULL)
   {
      return false;
   }

   const tChar * pszCommentStart = _tcsrchr(pszBuffer, kCommentChar);
   if (pszCommentStart != NULL)
   {
      if (pComment != NULL)
      {
         *pComment = _tcsinc(pszCommentStart);
         TrimLeadingSpace(pComment);
         TrimTrailingSpace(pComment);
      }
   }

   const tChar * pszSectionStart = _tcschr(pszBuffer, _T('['));
   const tChar * pszSectionEnd = _tcschr(pszBuffer, _T(']'));

   if ((pszSectionStart != NULL) && (pszSectionEnd != NULL) && (pszSectionEnd > pszSectionStart))
   {
      pszSectionStart = _tcsinc(pszSectionStart);
      *pSection = cStr(pszSectionStart, pszSectionEnd - pszSectionStart);
      TrimLeadingSpace(pSection);
      TrimTrailingSpace(pSection);
      return true;
   }

   return false;
}

///////////////////////////////////////

cDictionaryIniStore::cDictionaryIniStore(const cFileSpec & file,
                                         const tChar * pszSection)
 : m_file(file)
{
   _tcsncpy(m_szSection, pszSection, _countof(m_szSection));
   m_szSection[_countof(m_szSection) - 1] = 0;
}

///////////////////////////////////////

tResult cDictionaryIniStore::Load(IDictionary * pDictionary)
{
   FILE * fp = _tfopen(m_file.CStr(), _T("r"));
   if (fp == NULL)
   {
      return E_FAIL;
   }

   tChar buffer[1024];

   bool bInSection = false;

   while (_fgetts(buffer, sizeof(buffer), fp))
   {
      cStr section;
      if (ParseIniSectionLine(buffer, &section, NULL))
      {
         if (_tcsicmp(section.c_str(), m_szSection) == 0)
         {
            bInSection = true;
         }
         else
         {
            bInSection = false;
         }
      }
      else if (bInSection)
      {
         cStr key, value, comment;
         if (ParseDictionaryLine(buffer, &key, &value, &comment)
             && !key.empty() && !value.empty())
         {
            DebugMsgEx2(DictionaryStore, "Read dictionary entry '%s' = '%s'\n", key.c_str(), value.c_str());
            pDictionary->Set(key.c_str(), value.c_str());
         }
      }
   }

   fclose(fp);

   return S_OK;
}

///////////////////////////////////////

tResult cDictionaryIniStore::Save(IDictionary * pDictionary)
{
   return E_NOTIMPL; // TODO
}

///////////////////////////////////////

tResult cDictionaryIniStore::MergeSave(IDictionary * pDictionary)
{
   return E_NOTIMPL; // TODO
}

///////////////////////////////////////

IDictionaryStore * DictionaryIniStoreCreate(const cFileSpec & file,
                                            const tChar * pszSection)
{
   return static_cast<IDictionaryStore *>(new cDictionaryIniStore(file, pszSection));
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNITLITE2

///////////////////////////////////////

TEST(SplitString)
{
   cStr l, r;
   CHECK(SplitString("key=value", '=', &l, &r));
   CHECK(l == "key");
   CHECK(r == "value");
}

///////////////////////////////////////

TEST(ParseDictionaryLine)
{
   cStr key, value, comment;

   key.clear();
   value.clear();
   comment.clear();
   CHECK(ParseDictionaryLine("key=value", &key, &value, &comment));
   CHECK(key == "key");
   CHECK(value == "value");
   CHECK(comment.empty());

   key.clear();
   value.clear();
   comment.clear();
   CHECK(ParseDictionaryLine("  key  =  value  # this is a comment   ", &key, &value, &comment));
   CHECK(key == "key");
   CHECK(value == "value");
   CHECK(comment == "this is a comment");

   key.clear();
   value.clear();
   comment.clear();
   CHECK(ParseDictionaryLine("definition", &key, &value, &comment));
   CHECK(key == "definition");
   CHECK(value.empty());
   CHECK(comment.empty());
}

///////////////////////////////////////

TEST(ParseIniSectionLine)
{
   // without comment
   {
      cStr section, comment;
      CHECK(ParseIniSectionLine(_T("[IniSection]"), &section, &comment));
      CHECK(section.compare(_T("IniSection")) == 0);
      CHECK(comment.empty());
   }

   // with comment
   {
      cStr section, comment;
      CHECK(ParseIniSectionLine(_T("[IniSection]   # the comment"), &section, &comment));
      CHECK(section.compare(_T("IniSection")) == 0);
      CHECK(comment.compare(_T("the comment")) == 0);
   }

   // with lots of extra whitespace
   {
      cStr section, comment;
      CHECK(ParseIniSectionLine(_T("  [   IniSection  ]   #   the comment   "), &section, &comment));
      CHECK(section.compare(_T("IniSection")) == 0);
      CHECK(comment.compare(_T("the comment")) == 0);
   }
}

///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_CPPUNIT
