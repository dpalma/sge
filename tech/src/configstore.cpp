///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"
#include "configstore.h"
#include "readwriteapi.h"
#include <locale>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(ConfigStore);

static const char kCommentChar = '#';
static const char kSepChar = '=';

///////////////////////////////////////////////////////////////////////////////

char * TrimLeadingSpace(char * p)
{
   while (isspace(*p))
      p++;
   return p;
}

char * TrimTrailingSpace(char * p)
{
   char * end = p + strlen(p) - 1;
   while (end >= p && isspace(*end))
      *end-- = '\0';
   return p;
}

///////////////////////////////////////////////////////////////////////////////

void ParseConfigLine(char * pBuffer, const char * * ppKey,
                     const char * * ppVal, const char * * ppComment)
{
   *ppKey = NULL;
   *ppVal = NULL;
   *ppComment = NULL;

   char * p = strrchr(pBuffer, kCommentChar);

   if (p)
   {
      *p++ = '\0';
      *ppComment = p;
   }
   TrimTrailingSpace(pBuffer);

   *ppKey = pBuffer;

   p = strchr(pBuffer, kSepChar);

   if (p != NULL)
   {
      *p++ = '\0';
      p = TrimLeadingSpace(p);
      TrimTrailingSpace(pBuffer);

      *ppVal = p;
   }
}

///////////////////////////////////////////////////////////////////////////////

static bool SplitString(const char * psz, char split, cStr * pLeft, cStr * pRight)
{
   Assert(psz != NULL);
   Assert(pLeft != NULL);
   Assert(pRight != NULL);
   const char * pszSplit = strrchr(psz, split);
   if (pszSplit != NULL)
   {
      *pLeft = std::string(psz, pszSplit - psz).c_str();
      *pRight = pszSplit + 1;
      return true;
   }
   return false;
}

static void TrimLeadingSpace(cStr * pStr)
{
   Assert(pStr != NULL);
   cStr::size_type index = pStr->find_first_not_of(" \t\r\n");
   if (index != cStr::npos)
      pStr->erase(0, index);
}

static void TrimTrailingSpace(cStr * pStr)
{
   Assert(pStr != NULL);
   cStr::size_type index = pStr->find_last_not_of(" \t\r\n");
   if (index != cStr::npos)
      pStr->erase(index + 1);
}

inline void TrimSpace(cStr * pStr)
{
   TrimLeadingSpace(pStr);
   TrimTrailingSpace(pStr);
}

static bool ParseConfigLine(const char * psz, cStr * pKey, cStr * pValue, cStr * pComment)
{
   Assert(psz != NULL);
   Assert(pKey != NULL);
   Assert(pValue != NULL);
   Assert(pComment != NULL);
   cStr temp;
   if (SplitString(psz, kCommentChar, &temp, pComment))
   {
      psz = temp.c_str();
   }
   if (SplitString(psz, kSepChar, pKey, pValue))
   {
      TrimSpace(pKey);
      TrimSpace(pValue);
      TrimSpace(pComment);
      return true;
   }
   return false;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTextConfigStore
//

cTextConfigStore::cTextConfigStore(const cFileSpec & file)
 : m_file(file)
{
}

///////////////////////////////////////

tResult cTextConfigStore::Load(IConfig * pConfig)
{
   cAutoIPtr<IReader> pReader = FileCreateReader(m_file);
   if (pReader != NULL)
   {
      cStr line;
      while (pReader->Read(&line, '\n') == S_OK)
      {
         cStr key, value, comment;
         if (ParseConfigLine(line.c_str(), &key, &value, &comment)
             && !key.empty() && !value.empty())
         {
            DebugMsgEx2(ConfigStore, "Read config entry '%s' = '%s'\n", key.c_str(), value.c_str());
            Assert(pConfig != NULL);
            pConfig->Set(key.c_str(), value.c_str());
         }
      }
      return S_OK;
   }
   return E_FAIL;
}

///////////////////////////////////////

tResult cTextConfigStore::Save(IConfig * pConfig)
{
   FILE * fp = fopen(m_file.GetName(), "w");

   if (fp == NULL)
      return E_FAIL;

   HANDLE hIter;
   pConfig->IterCfgVarBegin(&hIter);

   cStr name, value;
   while (pConfig->IterNextCfgVar(&hIter, &name, &value))
   {
      fprintf(fp, "%s = %s", (const char *)name, (const char *)value);
   }

   pConfig->IterCfgVarEnd(&hIter);

   fclose(fp);

   return S_OK;
}

///////////////////////////////////////

tResult cTextConfigStore::MergeSave(IConfig * pConfig)
{
   return E_NOTIMPL; // @TODO (dpalma 12/1/00): 
}

///////////////////////////////////////

IConfigStore * CreateTextConfigStore(const cFileSpec & file)
{
   return new cTextConfigStore(file);
}

///////////////////////////////////////////////////////////////////////////////

BOOL ParseIniSectionLine(char * pBuffer, const char ** ppSection,
                         const char ** ppComment)
{
   *ppSection = NULL;
   if (ppComment != NULL)
      *ppComment = NULL;

   char * p = strrchr(pBuffer, kCommentChar);

   if (p)
   {
      *p++ = '\0';
      if (ppComment != NULL)
         *ppComment = p;
   }
   TrimTrailingSpace(pBuffer);

   if (pBuffer[0] != '[') // do not allow whitespace before "[section name]"
      return FALSE;

   pBuffer++; // skip past the '['

   p = strchr(pBuffer, ']');

   if (p == NULL)
      return FALSE;

   *p = '\0';
   TrimTrailingSpace(pBuffer);

   *ppSection = pBuffer;

   return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cIniSectionConfigStore
//

cIniSectionConfigStore::cIniSectionConfigStore(const cFileSpec & file,
                                               const char * pszSection)
 : m_file(file), m_section(pszSection)
{
}

///////////////////////////////////////

tResult cIniSectionConfigStore::Load(IConfig * pConfig)
{
   FILE * fp = fopen(m_file.GetName(), "r");

   if (fp == NULL)
      return E_FAIL;

   char buffer[1024];

   BOOL bInSection = FALSE;

   while (fgets(buffer, sizeof(buffer), fp))
   {
      const char * pszSection;

      if (ParseIniSectionLine(buffer, &pszSection, NULL))
      {
         if (stricmp(pszSection, m_section) == 0)
         {
            bInSection = TRUE;
         }
         else
         {
            bInSection = FALSE;
         }
      }
      else if (bInSection)
      {
         const char * pszKey = NULL, * pszVal = NULL, * pszComment = NULL;

         ParseConfigLine(buffer, &pszKey, &pszVal, &pszComment);

         if (pszKey && pszVal)
         {
            DebugMsgEx2(ConfigStore, "Read config entry '%s' = '%s'\n", pszKey, pszVal);
            pConfig->Set(pszKey, pszVal);
         }
      }
   }

   fclose(fp);

   return S_OK;
}

///////////////////////////////////////

tResult cIniSectionConfigStore::Save(IConfig * pConfig)
{
   return E_NOTIMPL; // @TODO (dpalma 7/7/01)
}

///////////////////////////////////////

tResult cIniSectionConfigStore::MergeSave(IConfig * pConfig)
{
   return E_NOTIMPL; // @TODO (dpalma 7/7/01)
}

///////////////////////////////////////

IConfigStore * CreateIniSectionConfigStore(const cFileSpec & file,
                                           const char * pszSection)
{
   return new cIniSectionConfigStore(file, pszSection);
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cConfigStoreTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cConfigStoreTests);
      CPPUNIT_TEST(TestSplitString);
      CPPUNIT_TEST(TestParseConfigLine);
   CPPUNIT_TEST_SUITE_END();

public:
   void TestSplitString()
   {
      cStr l, r;
      CPPUNIT_ASSERT(SplitString("key=value", '=', &l, &r));
      CPPUNIT_ASSERT(l == "key");
      CPPUNIT_ASSERT(r == "value");
   }

   void TestParseConfigLine()
   {
      cStr key, value, comment;

      CPPUNIT_ASSERT(ParseConfigLine("key=value", &key, &value, &comment));
      CPPUNIT_ASSERT(key == "key");
      CPPUNIT_ASSERT(value == "value");
      CPPUNIT_ASSERT(comment.empty());

      CPPUNIT_ASSERT(ParseConfigLine("  key  =  value  # this is a comment   ", &key, &value, &comment));
      CPPUNIT_ASSERT(key == "key");
      CPPUNIT_ASSERT(value == "value");
      CPPUNIT_ASSERT(comment == "this is a comment");
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(cConfigStoreTests);

///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_CPPUNIT
