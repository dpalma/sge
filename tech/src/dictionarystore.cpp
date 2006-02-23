///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "dictionarystore.h"
#include "readwriteapi.h"
#include "techstring.h"

#include <locale>
#include <list>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

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

   cAutoIPtr<IReader> pReader = FileCreateReader(m_file);
   if (!!pReader)
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

bool ParseIniSectionLine(tChar * pszBuffer, cStr * pSection, cStr * pComment)
{
   *pSection = NULL;
   if (pComment != NULL)
   {
      *pComment = NULL;
   }

   tChar * p = _tcsrchr(pszBuffer, kCommentChar);

   if (p != NULL)
   {
      *p = 0;
      p = _tcsinc(p);
      if (pComment != NULL)
      {
         *pComment = p;
      }
   }
   TrimTrailingSpace(pComment);

   if (pszBuffer[0] != _T('[')) // do not allow whitespace before "[section name]"
   {
      return false;
   }

   pszBuffer = _tcsinc(pszBuffer); // skip past the '['

   p = _tcschr(pszBuffer, _T(']'));
   if (p == NULL)
   {
      return false;
   }

   *p = 0;

   *pSection = pszBuffer;
   TrimTrailingSpace(pSection);

   return true;
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

#ifdef HAVE_CPPUNIT

class cDictionaryStoreTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cDictionaryStoreTests);
      CPPUNIT_TEST(TestSplitString);
      CPPUNIT_TEST(TestParseDictionaryLine);
   CPPUNIT_TEST_SUITE_END();

   void TestSplitString();
   void TestParseDictionaryLine();
};

CPPUNIT_TEST_SUITE_REGISTRATION(cDictionaryStoreTests);

void cDictionaryStoreTests::TestSplitString()
{
   cStr l, r;
   CPPUNIT_ASSERT(SplitString("key=value", '=', &l, &r));
   CPPUNIT_ASSERT(l == "key");
   CPPUNIT_ASSERT(r == "value");
}

void cDictionaryStoreTests::TestParseDictionaryLine()
{
   cStr key, value, comment;

   key.clear();
   value.clear();
   comment.clear();
   CPPUNIT_ASSERT(ParseDictionaryLine("key=value", &key, &value, &comment));
   CPPUNIT_ASSERT(key == "key");
   CPPUNIT_ASSERT(value == "value");
   CPPUNIT_ASSERT(comment.empty());

   key.clear();
   value.clear();
   comment.clear();
   CPPUNIT_ASSERT(ParseDictionaryLine("  key  =  value  # this is a comment   ", &key, &value, &comment));
   CPPUNIT_ASSERT(key == "key");
   CPPUNIT_ASSERT(value == "value");
   CPPUNIT_ASSERT(comment == "this is a comment");

   key.clear();
   value.clear();
   comment.clear();
   CPPUNIT_ASSERT(ParseDictionaryLine("definition", &key, &value, &comment));
   CPPUNIT_ASSERT(key == "definition");
   CPPUNIT_ASSERT(value.empty());
   CPPUNIT_ASSERT(comment.empty());
}

///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_CPPUNIT
