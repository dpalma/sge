///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "filepath.h"
#include "filespec.h"
#include "techstring.h"

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
}

///////////////////////////////////////

cFileSpec::cFileSpec(const char * pszFile)
 : cStr(pszFile)
{
}

///////////////////////////////////////

cFileSpec::cFileSpec(const cFileSpec & other)
{
   operator =(other);
}

///////////////////////////////////////

const cFileSpec & cFileSpec::operator =(const cFileSpec & other)
{
   assign(other.c_str());
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

   StrCpyExcl(szTemp1, c_str(), kPathSeps);
   StrCpyExcl(szTemp2, other.c_str(), kPathSeps);

   return strcmp(szTemp1, szTemp2);
}

///////////////////////////////////////

int cFileSpec::CompareNoCase(const cFileSpec & other) const
{
   char szTemp1[kMaxPath], szTemp2[kMaxPath];

   StrCpyExcl(szTemp1, c_str(), kPathSeps);
   StrCpyExcl(szTemp2, other.c_str(), kPathSeps);

   return stricmp(szTemp1, szTemp2);
}

///////////////////////////////////////

const char * cFileSpec::GetFileName() const
{
   size_type i = find_last_of(kPathSeps);
   if (i != npos)
   {
      return c_str() + i + 1;
   }
   else
   {
      return c_str();
   }
}

///////////////////////////////////////

bool cFileSpec::GetFileNameNoExt(cStr * pFileName) const
{
   if (pFileName != NULL)
   {
      const char * p = GetFileName();
      Assert(p != NULL);
      const char * pExt = strrchr(p, kExtensionSep);
      if (pExt != NULL)
      {
         char * pTemp = reinterpret_cast<char*>(alloca(pExt - p + 1));
         strncpy(pTemp, p, pExt - p);
         pTemp[pExt - p] = 0;
         *pFileName = pTemp;
      }
      else
      {
         *pFileName = p;
      }
      return true;
   }
   return false;
}

///////////////////////////////////////

const char * cFileSpec::GetFileExt() const
{
   const char * pszExt = strrchr(c_str(), kExtensionSep);
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
   size_t len = length();
   if (len > 0)
   {
      size_type i = rfind(kExtensionSep);
      if (i != npos)
      {
         erase(i);
      }
      append(szExtensionSep);
      append(pszExt);
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
      const char * pszFullName = c_str();
      return cFilePath(pszFullName, pszFileName - pszFullName);
   }
   else
   {
      return cFilePath(c_str());
   }
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
      CPPUNIT_TEST(TestSetFileExtBufferOverrunAttack);
   CPPUNIT_TEST_SUITE_END();

   void TestGetFileName();
   void TestGetSetFileExt();
   void TestSetPath();
   void TestGetPath();
   void TestCompare();
   void TestSetFileExtBufferOverrunAttack();
};

///////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cFileSpecTests);

///////////////////////////////////////

void cFileSpecTests::TestGetFileName()
{
   CPPUNIT_ASSERT(strcmp(cFileSpec("c:\\p1\\p2\\p3.ext").GetFileName(), "p3.ext") == 0);
   CPPUNIT_ASSERT(strcmp(cFileSpec("C:\\P1\\P2\\P3.EXT").GetFileName(), "P3.EXT") == 0);
   CPPUNIT_ASSERT(strcmp(cFileSpec("c:\\p1\\p2.p3").GetFileName(), "p2.p3") == 0);
   CPPUNIT_ASSERT(strcmp(cFileSpec("c:\\foo\\bar").GetFileName(), "bar") == 0);
}

///////////////////////////////////////

void cFileSpecTests::TestGetSetFileExt()
{
   {
      cFileSpec fs("c:\\path1\\path2\\file");
      CPPUNIT_ASSERT(strlen(fs.GetFileExt()) == 0);
      CPPUNIT_ASSERT(fs.SetFileExt("ext"));
      CPPUNIT_ASSERT(strcmp(fs.GetFileExt(), "ext") == 0);
      CPPUNIT_ASSERT(strcmp(fs.c_str(), "c:\\path1\\path2\\file.ext") == 0);
   }

   {
      cFileSpec fs("c:\\path1\\path2\\file.TST");
      CPPUNIT_ASSERT(strcmp(fs.GetFileExt(), "TST") == 0);
      CPPUNIT_ASSERT(fs.SetFileExt("ext"));
      CPPUNIT_ASSERT(strcmp(fs.GetFileExt(), "ext") == 0);
      CPPUNIT_ASSERT(strcmp(fs.c_str(), "c:\\path1\\path2\\file.ext") == 0);
   }
}

///////////////////////////////////////

void cFileSpecTests::TestSetPath()
{
   {
      cFileSpec fs1("c:\\p1\\p2\\p3\\p4\\file.ext");
      fs1.SetPath(cFilePath());
      CPPUNIT_ASSERT(strcmp(fs1.c_str(), "file.ext") == 0);
   }

   {
      cFileSpec fs1("c:\\p1\\p2\\p3\\p4\\file.ext");
      fs1.SetPath(cFilePath("D:\\path"));
      CPPUNIT_ASSERT(strcmp(fs1.c_str(), "D:\\path\\file.ext") == 0);
   }
}

///////////////////////////////////////

void cFileSpecTests::TestGetPath()
{
   CPPUNIT_ASSERT(strcmp(cFileSpec("c:\\p1\\p2\\p3\\p4\\file.ext").GetPath().GetPath(), "c:\\p1\\p2\\p3\\p4") == 0);
}

///////////////////////////////////////

void cFileSpecTests::TestCompare()
{
   CPPUNIT_ASSERT(cFileSpec("c:\\p1\\p2\\p3.ext").Compare(cFileSpec("c:/p1/p2/p3.ext")) == 0);
   CPPUNIT_ASSERT(cFileSpec("C:\\P1\\P2\\P3.EXT").Compare(cFileSpec("c:/p1/p2/p3.ext")) != 0);
   CPPUNIT_ASSERT(cFileSpec("C:\\P1\\P2\\P3.EXT").CompareNoCase(cFileSpec("c:/p1/p2/p3.ext")) == 0);
   CPPUNIT_ASSERT(cFileSpec("c:\\p1\\p2.p3").Compare(cFileSpec("c:\\p4\\p5.p6")) < 0);
}

///////////////////////////////////////

static bool g_bSetFileExtBufferOverrunSucceeded = false;

static void SetFileExtAttackFunction()
{
   g_bSetFileExtBufferOverrunSucceeded = true;
   DebugMsg("BUFFER OVERRUN ATTACK SUCCEEDED ON cFileSpec::SetFileExt()!!!\n");
}

static bool Vulnerable(const char * pszTestFileName, char * psz)
{
   cFileSpec f(pszTestFileName);
   return f.SetFileExt(psz);
}

void cFileSpecTests::TestSetFileExtBufferOverrunAttack()
{
   static const char szTestFileName1[] = "foo.txt";
   static const char szTestFileName2[] = "foo";

   struct sAttack
   {
      char szExt[kMaxPath - 4]; // the 4 is the length of "foo." in szTestFileName
      void * pStackFrame;
      void * pReturnAddress;
   };

   struct sAttack attack;

   static const unsigned char NOP = 0x90;

   memset(attack.szExt, NOP, sizeof(attack.szExt));
   attack.pStackFrame = (void *)0xDEADBEEF;
   attack.pReturnAddress = (void *)SetFileExtAttackFunction;

   try
   {
      CPPUNIT_ASSERT(Vulnerable(szTestFileName1, (char*)&attack));
      CPPUNIT_ASSERT(Vulnerable(szTestFileName2, (char*)&attack));
      CPPUNIT_ASSERT(!g_bSetFileExtBufferOverrunSucceeded);
   }
   catch (...)
   {
      CPPUNIT_ASSERT(!g_bSetFileExtBufferOverrunSucceeded);
      throw;
   }
}

///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_CPPUNIT
