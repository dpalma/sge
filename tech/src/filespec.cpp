///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "filepath.h"
#include "filespec.h"

#include <cstdio>
#include <cstring>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

static const char kExtensionSep = '.';
static const char szExtensionSep[] = { kExtensionSep, 0 };
static const char kPathSeps[] = "\\/";

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFileSpec
//

///////////////////////////////////////

cFileSpec::cFileSpec()
{
   memset(m_szFullName, 0, sizeof(m_szFullName));
}

///////////////////////////////////////

cFileSpec::cFileSpec(const char * pszFile)
{
   strncpy(m_szFullName, pszFile, _countof(m_szFullName));
   m_szFullName[_countof(m_szFullName) - 1] = 0;
}

///////////////////////////////////////

cFileSpec::cFileSpec(const cFileSpec & other)
{
   operator =(other);
}

///////////////////////////////////////

const cFileSpec & cFileSpec::operator =(const char * pszFile)
{
   strncpy(m_szFullName, pszFile, _countof(m_szFullName));
   m_szFullName[_countof(m_szFullName) - 1] = 0;
   return *this;
}

///////////////////////////////////////

static void StrCpyExcl(char * pDest, const char * pSrc, const char * pExcl)
{
   for (; *pSrc; pSrc++)
   {
      if (!strchr(pExcl, *pSrc))
      {
         *pDest++ = *pSrc;
      }
   }
   *pDest = 0;
}

///////////////////////////////////////

int cFileSpec::Compare(const cFileSpec & other) const
{
   char szTemp1[kMaxPath], szTemp2[kMaxPath];

   StrCpyExcl(szTemp1, GetName(), kPathSeps);
   StrCpyExcl(szTemp2, other.GetName(), kPathSeps);

   return strcmp(szTemp1, szTemp2);
}

///////////////////////////////////////

int cFileSpec::CompareNoCase(const cFileSpec & other) const
{
   char szTemp1[kMaxPath], szTemp2[kMaxPath];

   StrCpyExcl(szTemp1, GetName(), kPathSeps);
   StrCpyExcl(szTemp2, other.GetName(), kPathSeps);

   return stricmp(szTemp1, szTemp2);
}

///////////////////////////////////////

const char * ReverseFindOneOf(const char * string, const char * strCharSet)
{
   const char * start = string;

   string += strlen(string);

   while (--string != start)
   {
      if (strchr(strCharSet, *string))
      {
         return string;
      }
   }

   return NULL;
}

const char * cFileSpec::GetFileName() const
{
   const char * pszLastPathSep = ReverseFindOneOf(GetName(), kPathSeps);
   if (pszLastPathSep != NULL)
   {
      return ++pszLastPathSep;
   }
   else
   {
      return GetName();
   }
}

///////////////////////////////////////

const char * cFileSpec::GetFileExt() const
{
   const char * pszExt = strrchr(GetName(), kExtensionSep);
   if (pszExt)
   {
      return ++pszExt;
   }
   else
   {
      return "";
   }
}

///////////////////////////////////////

bool cFileSpec::SetFileExt(const char * pszExt)
{
   if (strlen(GetName()) > 0)
   {
      char * pszDest = strrchr(GetName(), kExtensionSep);
      if (pszDest != NULL)
      {
         strcpy(++pszDest, pszExt);
      }
      else
      {
         strcat(m_szFullName, szExtensionSep);
         strcat(m_szFullName, pszExt);
      }
      return true;
   }
   return false;
}

///////////////////////////////////////

void cFileSpec::SetPath(const cFilePath & path)
{
   cFilePath temp(path);
   temp.AddRelative(GetFileName());
   *this = cFileSpec(temp.GetPath());
}

///////////////////////////////////////

cFilePath cFileSpec::GetPath() const
{
   const char * pszFileName = GetFileName();
   if (pszFileName != NULL)
   {
      const char * pszFullName = GetName();
      return cFilePath(pszFullName, pszFileName - pszFullName);
   }
   else
   {
      return cFilePath(GetName());
   }
}

///////////////////////////////////////

bool cFileSpec::Exists() const
{
   FILE * fp = fopen(GetName(), "r");
   if (fp != NULL)
   {
      fclose(fp);
      return true;
   }
   return false;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////

class cFileSpecTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cFileSpecTests);
      CPPUNIT_TEST(TestGetFileName);
      CPPUNIT_TEST(TestGetSetFileExt);
      CPPUNIT_TEST(TestSetPath);
      CPPUNIT_TEST(TestGetPath);
      CPPUNIT_TEST(TestCompare);
   CPPUNIT_TEST_SUITE_END();

   void TestGetFileName();
   void TestGetSetFileExt();
   void TestSetPath();
   void TestGetPath();
   void TestCompare();
};

CPPUNIT_TEST_SUITE_REGISTRATION(cFileSpecTests);

void cFileSpecTests::TestGetFileName()
{
   CPPUNIT_ASSERT(strcmp(cFileSpec("c:\\p1\\p2\\p3.ext").GetFileName(), "p3.ext") == 0);
   CPPUNIT_ASSERT(strcmp(cFileSpec("C:\\P1\\P2\\P3.EXT").GetFileName(), "P3.EXT") == 0);
   CPPUNIT_ASSERT(strcmp(cFileSpec("c:\\p1\\p2.p3").GetFileName(), "p2.p3") == 0);
   CPPUNIT_ASSERT(strcmp(cFileSpec("c:\\foo\\bar").GetFileName(), "bar") == 0);
}

void cFileSpecTests::TestGetSetFileExt()
{
   {
      cFileSpec fs("c:\\path1\\path2\\file");
      CPPUNIT_ASSERT(strlen(fs.GetFileExt()) == 0);
      CPPUNIT_ASSERT(fs.SetFileExt("ext"));
      CPPUNIT_ASSERT(strcmp(fs.GetFileExt(), "ext") == 0);
      CPPUNIT_ASSERT(strcmp(fs.GetName(), "c:\\path1\\path2\\file.ext") == 0);
   }

   {
      cFileSpec fs("c:\\path1\\path2\\file.TST");
      CPPUNIT_ASSERT(strcmp(fs.GetFileExt(), "TST") == 0);
      CPPUNIT_ASSERT(fs.SetFileExt("ext"));
      CPPUNIT_ASSERT(strcmp(fs.GetFileExt(), "ext") == 0);
      CPPUNIT_ASSERT(strcmp(fs.GetName(), "c:\\path1\\path2\\file.ext") == 0);
   }
}

void cFileSpecTests::TestSetPath()
{
   {
      cFileSpec fs1("c:\\p1\\p2\\p3\\p4\\file.ext");
      fs1.SetPath(cFilePath());
      CPPUNIT_ASSERT(strcmp(fs1.GetName(), "file.ext") == 0);
   }

   {
      cFileSpec fs1("c:\\p1\\p2\\p3\\p4\\file.ext");
      fs1.SetPath(cFilePath("D:\\path"));
      CPPUNIT_ASSERT(strcmp(fs1.GetName(), "D:\\path\\file.ext") == 0);
   }
}

void cFileSpecTests::TestGetPath()
{
   CPPUNIT_ASSERT(strcmp(cFileSpec("c:\\p1\\p2\\p3\\p4\\file.ext").GetPath().GetPath(), "c:\\p1\\p2\\p3\\p4") == 0);
}

void cFileSpecTests::TestCompare()
{
   CPPUNIT_ASSERT(cFileSpec("c:\\p1\\p2\\p3.ext").Compare(cFileSpec("c:/p1/p2/p3.ext")) == 0);
   CPPUNIT_ASSERT(cFileSpec("C:\\P1\\P2\\P3.EXT").Compare(cFileSpec("c:/p1/p2/p3.ext")) != 0);
   CPPUNIT_ASSERT(cFileSpec("C:\\P1\\P2\\P3.EXT").CompareNoCase(cFileSpec("c:/p1/p2/p3.ext")) == 0);
   CPPUNIT_ASSERT(cFileSpec("c:\\p1\\p2.p3").Compare(cFileSpec("c:\\p4\\p5.p6")) < 0);
}

///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_CPPUNIT
