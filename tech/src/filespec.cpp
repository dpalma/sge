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

cFileSpec::cFileSpec(const char * pszFilePath)
{
   strcpy(m_szFullName, pszFilePath);
}

///////////////////////////////////////

cFileSpec::cFileSpec(const cFileSpec & other)
{
   operator=(other);
}

///////////////////////////////////////

cFileSpec::cFileSpec(const cFilePath & path, const char * pszFileName)
{
   strcpy(m_szFullName, pszFileName);
   SetPath(path);
}

///////////////////////////////////////

const cFileSpec & cFileSpec::operator=(const cFileSpec & other)
{
   strcpy(m_szFullName, other.m_szFullName);
   return *this;
}

///////////////////////////////////////

int cFileSpec::operator==(const cFileSpec & other) const
{
   return stricmp(m_szFullName, other.m_szFullName) == 0;
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

BOOL cFileSpec::SetFileExt(const char * pszExt)
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
      return TRUE;
   }
   return FALSE;
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
      CPPUNIT_TEST(TestAssignment);
      CPPUNIT_TEST(TestComparison);
      CPPUNIT_TEST(TestGetName);
      CPPUNIT_TEST(TestGetFileName);
      CPPUNIT_TEST(TestGetSetFileExt);
      CPPUNIT_TEST(TestSetPath);
      CPPUNIT_TEST(TestGetPath);
   CPPUNIT_TEST_SUITE_END();

public:
   void TestAssignment()
   {
      // @TODO
   }

   void TestComparison()
   {
      // @TODO
   }

   void TestGetName()
   {
      // @TODO
   }

   void TestGetFileName()
   {
      // @TODO
   }

   void TestGetSetFileExt()
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

   void TestSetPath()
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

   void TestGetPath()
   {
      CPPUNIT_ASSERT(strcmp(cFileSpec("c:\\p1\\p2\\p3\\p4\\file.ext").GetPath().GetPath(), "c:\\p1\\p2\\p3\\p4") == 0);
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(cFileSpecTests);

///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_CPPUNIT
