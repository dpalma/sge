///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "filepath.h"
#include "fileconst.h"

#include <cstring>
#include <cstdlib>
#include <locale>

#ifndef _WIN32
#include <unistd.h>
#endif

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
extern "C"
{
   __declspec(dllimport) uint STDCALL GetCurrentDirectoryA(uint, char *);
   __declspec(dllimport) uint STDCALL GetCurrentDirectoryW(uint, wchar_t *);
}
#ifdef UNICODE
#define GetCurrentDirectory  GetCurrentDirectoryW
#else
#define GetCurrentDirectory  GetCurrentDirectoryA
#endif // !UNICODE
#endif // _WIN32

///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
static const char kPathSep = '\\';
#else
static const char kPathSep = '/';
#endif
static const char szPathSep[] = { kPathSep, 0 };
static const char szPathSeps[] = "\\/";

///////////////////////////////////////////////////////////////////////////////

inline bool IsPathSep(char c)
{
   if (c == '/' || c == '\\')
      return true;
   else
      return false;
}

inline bool IsTwoDots(const char * psz)
{
   if (strncmp("/..", psz, 3) == 0 ||
       strncmp("\\..", psz, 3) == 0 ||
       strncmp("..", psz, 2) == 0)
      return true;
   else
      return false;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFilePath
//

///////////////////////////////////////

cFilePath::cFilePath()
{
}
   
///////////////////////////////////////

cFilePath::cFilePath(const cFilePath & other)
 : cStr(other.c_str())
{
}

///////////////////////////////////////

cFilePath::cFilePath(const char * pszPath)
 : cStr(pszPath)
{
}

///////////////////////////////////////

cFilePath::cFilePath(const char * pszPath, size_t pathLen)
 : cStr(pszPath, pathLen)
{
}

///////////////////////////////////////

const cFilePath & cFilePath::operator =(const cFilePath & other)
{
   assign(other.c_str());
   return *this;
}

///////////////////////////////////////

static void PathCat(char * pszDest, const char * pszSrc)
{
   int destLen = strlen(pszDest);

   if (destLen > 0
      && (*pszSrc != '/')
      && (*pszSrc != '\\')
      && (pszDest[destLen - 1] != '/')
      && (pszDest[destLen - 1] != '\\'))
   {
      strcat(pszDest, szPathSep);
   }

   strcat(pszDest, pszSrc);
}

///////////////////////////////////////

void cFilePath::AddRelative(const char * pszDir)
{
   if (!IsEmpty()
      && (*pszDir != '/')
      && (*pszDir != '\\')
      && (at(length() - 1) != '/')
      && (at(length() - 1) != '\\'))
   {
      append(szPathSep);
   }
   append(pszDir);
}

///////////////////////////////////////

bool cFilePath::IsFullPath() const
{
   if (length() >= 3 &&
       isalpha(at(0)) &&
       at(1) == ':' &&
       IsPathSep(at(2)) &&
       strstr(c_str(), "..") == NULL)
   {
      return true;
   }

   if (IsPathSep(at(0)) && strstr(c_str(), "..") == NULL)
   {
      return true;
   }

   return false;
}

///////////////////////////////////////

void cFilePath::MakeFullPath()
{
   if (!IsFullPath())
   {
      cFilePath temp(GetCwd());
      temp.AddRelative(c_str());
      temp.CollapseDots();
      *this = temp;
   }
}

///////////////////////////////////////

cFilePath cFilePath::CollapseDots()
{
   int nDirs = 0;
   const char * * dirs = static_cast<const char**>(alloca(length() * sizeof(const char *)));
   memset(dirs, 0, sizeof(dirs));

   bool bHadLeadingSeparator = false;
   if (IsPathSep(at(0)))
   {
      bHadLeadingSeparator = true;
   }

   const char * p = c_str();
   dirs[nDirs++] = p++;
   for (; *p != 0; p++)
   {
      if (IsPathSep(*p))
      {
         dirs[nDirs++] = p;
      }
   }

   // find the first directory that isn't two dots
   int i, iFirstCollapsible = 0;
   for (i = 0; i < nDirs; i++)
   {
      if (IsTwoDots(dirs[i]))
      {
         iFirstCollapsible++;
      }
      else
      {
         break;
      }
   }

   for (i = 0; i < nDirs; i++)
   {
      char szDir[kMaxPath];
      if (i < (nDirs - 1))
      {
         size_t len = dirs[i + 1] - dirs[i] + 1;
         strncpy(szDir, dirs[i], len);
         szDir[len - 1] = 0;
      }
      else
      {
         strcpy(szDir, dirs[i]);
      }

      if (IsTwoDots(szDir) && i > iFirstCollapsible)
      {
         dirs[i] = NULL; // remove this ".." from the result

         // remove the previous directory from the result
         int iRemove = i - 1;
         while (dirs[iRemove] == NULL && iRemove > 0)
         {
            iRemove--;
         }

         if (iRemove >= iFirstCollapsible)
         {
            dirs[iRemove] = NULL;
         }
      }
   }

   cStr result;

   for (i = 0; i < nDirs; i++)
   {
      if (dirs[i] != NULL)
      {
         size_t len = strcspn(dirs[i] + 1, szPathSeps) + 1;
         result.Append(cStr(dirs[i], len).c_str());
      }
   }

   if (!bHadLeadingSeparator && IsPathSep(result[0]))
   {
      result.erase(0, 1);
   }

   return cFilePath(result.c_str());
}

///////////////////////////////////////

cFilePath cFilePath::GetCwd()
{
   char szCwd[kMaxPath];
#ifdef _WIN32
   GetCurrentDirectory(_countof(szCwd), szCwd);
#else
   getcwd(szCwd, _countof(szCwd));
#endif
   return cFilePath(szCwd);
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cFilePathTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cFilePathTests);
      CPPUNIT_TEST(TestCollapseDots);
      CPPUNIT_TEST(TestIsFullPath);
   CPPUNIT_TEST_SUITE_END();

   static const char * gm_testStrings[];

   void TestCollapseDots();
   void TestIsFullPath();
};

////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cFilePathTests);

////////////////////////////////////////

const char * cFilePathTests::gm_testStrings[] =
{
   "/p1/p2/p3",            "/p1/p2/p3",         // nothing to do
   "/p1/p2/p3/..",         "/p1/p2",            // parent directory at end
   "../p1/p2/p3",          "../p1/p2/p3",       // parent directory at begining, can't collapse
   "/p1/p2/../p3",         "/p1/p3",            // parent directory mid-string
   "/p1/p2/p3/../..",      "/p1",               // multiple parent directories at end
   "../../p1/p2/p3",       "../../p1/p2/p3",    // multiple parent directories at begining, can't collapse
   "/p1/p2/../../p3",      "/p3",               // multiple parent directories mid-string
   "/p1/p2/p3/../../..",   "",                  // total collapse
   "p1/../p2/p3",          "p2/p3",             // ensure relative path does not become absolute
   "../../p1/../p2/p3",    "../../p2/p3",       // collapse middle but not beginning
#if 0
   "/p1/p2/../../../p3",   "/p1/p2/../../../p3", // too many levels
   "./p1/p2",              "p1/p2",
   "./p1/p2/../../p3",     "p3",
#endif
};

////////////////////////////////////////

void cFilePathTests::TestCollapseDots()
{
   for (int i = 0; i < _countof(gm_testStrings); i += 2)
   {
      cFilePath temp(cFilePath(gm_testStrings[i]).CollapseDots());
      CPPUNIT_ASSERT(filepathcmp(gm_testStrings[i + 1], temp.c_str()) == 0);
   }
}

////////////////////////////////////////

void cFilePathTests::TestIsFullPath()
{
   CPPUNIT_ASSERT(cFilePath("C:\\p1\\p2").IsFullPath());
   CPPUNIT_ASSERT(!cFilePath("C:p1\\p2").IsFullPath());
   CPPUNIT_ASSERT(!cFilePath("C:\\p1\\p2\\..\\p3").IsFullPath());
   CPPUNIT_ASSERT(cFilePath("/p1/p2/p3").IsFullPath());
   CPPUNIT_ASSERT(!cFilePath("p1/p2/p3").IsFullPath());
   CPPUNIT_ASSERT(!cFilePath("/p1/p2/../p3").IsFullPath());
}

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
