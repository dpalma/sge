///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "filepath.h"
#include "filespec.h"

#include <cstring>
#include <cstdlib>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#endif
#include <locale>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

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

static char * CollapseDots(const char * pszPath, char * pszResult, int maxLen)
{
   const char * dirs[kMaxPath];
   int nDirs = 0;
   int iFirstCollapsible = 0;
   bool bHadLeadingSeparator = false;

   memset(dirs, 0, sizeof(dirs));

   if (IsPathSep(*pszPath))
      bHadLeadingSeparator = true;
   else
      dirs[nDirs++] = pszPath;

   const char * p;
   for (p = pszPath; *p != 0; p++)
   {
      if (IsPathSep(*p))
         dirs[nDirs++] = p;
   }

   // find the first directory that isn't two dots
   int i;
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
         int len = dirs[i + 1] - dirs[i] + 1;
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
            iRemove--;

         if (iRemove >= iFirstCollapsible)
            dirs[iRemove] = NULL;
      }
   }

   *pszResult = 0;

   for (i = 0; i < nDirs; i++)
   {
      if (dirs[i] != NULL)
      {
         int len = strcspn(dirs[i] + 1, szPathSeps) + 1;
         strncat(pszResult, dirs[i], len);
      }
   }

   if (!bHadLeadingSeparator && IsPathSep(*pszResult))
   {
      memmove(pszResult, pszResult + 1, strlen(pszResult));
   }

   return pszResult;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFilePath
//

///////////////////////////////////////

cFilePath::cFilePath()
{
   m_szPath[0] = 0;
}
   
///////////////////////////////////////

cFilePath::cFilePath(const cFilePath & other)
{
   strcpy(m_szPath, other.m_szPath);
}

///////////////////////////////////////

cFilePath::cFilePath(const char * pszPath)
{
   operator =(pszPath);
}

///////////////////////////////////////

cFilePath::cFilePath(const char * pszPath, int pathLen)
{
   if (pszPath != NULL && pathLen > 0)
   {
      int len = Min(pathLen, sizeof(m_szPath));
      strncpy(m_szPath, pszPath, len);
      m_szPath[len - 1] = 0;
   }
   else
   {
      m_szPath[0] = 0;
   }
}

///////////////////////////////////////

const cFilePath & cFilePath::operator =(const char * pszPath)
{
   if (pszPath != NULL)
   {
      strncpy(m_szPath, pszPath, sizeof(m_szPath));
      m_szPath[_countof(m_szPath) - 1] = 0;
   }
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

int cFilePath::Compare(const cFilePath & other) const
{
   char szTemp1[kMaxPath], szTemp2[kMaxPath];

   StrCpyExcl(szTemp1, GetPath(), szPathSeps);
   StrCpyExcl(szTemp2, other.GetPath(), szPathSeps);

   return stricmp(szTemp1, szTemp2);
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
   PathCat(m_szPath, pszDir);
}

///////////////////////////////////////

bool cFilePath::IsFullPath() const
{
   if (strlen(m_szPath) >= 3 &&
       isalpha(m_szPath[0]) &&
       m_szPath[1] == ':' &&
       IsPathSep(m_szPath[2]) &&
       strstr(m_szPath, "..") == NULL)
   {
      return true;
   }

   if (IsPathSep(m_szPath[0]) && strstr(m_szPath, "..") == NULL)
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
      char szFull[kMaxPath];

#ifdef _WIN32
      _getcwd(szFull, _countof(szFull));
#else
      getcwd(szFull, _countof(szFull));
#endif

      PathCat(szFull, GetPath());

      CollapseDots(szFull, m_szPath, _countof(m_szPath));
   }
}

///////////////////////////////////////

cFilePath cFilePath::GetCwd()
{
   char szCwd[kMaxPath];
#ifdef _WIN32
   _getcwd(szCwd, _countof(szCwd));
#else
   getcwd(szCwd, _countof(szCwd));
#endif
   return cFilePath(szCwd);
}

///////////////////////////////////////

int cFilePath::ListDirs(std::vector<std::string> * pDirs) const
{
   Assert(pDirs != NULL);
   if (pDirs == NULL)
      return -1;

   pDirs->clear();

#ifdef _WIN32
   cFileSpec wildcard(*this, "*");

   WIN32_FIND_DATA findData;
   HANDLE hFinder = FindFirstFile(wildcard.GetName(), &findData);
   if (hFinder != INVALID_HANDLE_VALUE)
   {
      do
      {
         if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
         {
            if (strcmp(findData.cFileName, ".") &&
                strcmp(findData.cFileName, ".."))
            {
               pDirs->push_back(findData.cFileName);
            }
         }
      }
      while (FindNextFile(hFinder, &findData));

      FindClose(hFinder);
   }
#else
   DIR * dir = opendir(GetPath());
   if (dir)
   {
      struct dirent * ent = readdir(dir);
      while (ent)
      {
         if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, ".."))
         {
            cFileSpec file(*this, ent->d_name);

            struct stat fstat;
            if (stat(file.GetName(), &fstat) == 0)
            {
               if (S_ISDIR(fstat.st_mode))
               {
                  pDirs->push_back(ent->d_name);
               }
            }
         }

         ent = readdir(dir);
      }

      closedir(dir);
   }
#endif

   return pDirs->size();
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cCollapseDotsTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cCollapseDotsTests);
      CPPUNIT_TEST(TestCollapseDots);
      CPPUNIT_TEST(TestCollapseDotsBufferSize);
   CPPUNIT_TEST_SUITE_END();

   static const char * gm_testStrings[];
   static const int gm_nTestStrings;

   void TestCollapseDots()
   {
      for (int i = 0; i < gm_nTestStrings; i += 2)
      {
         char szTemp[kMaxPath];
         CollapseDots(gm_testStrings[i], szTemp, kMaxPath);
         CPPUNIT_ASSERT(strcmp(gm_testStrings[i + 1], szTemp) == 0);
      }
   }

   void TestCollapseDotsBufferSize()
   {
      // @TODO: ensure no buffer over-runs and return value is appropriate
   }
};

const char * cCollapseDotsTests::gm_testStrings[] =
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

const int cCollapseDotsTests::gm_nTestStrings = _countof(gm_testStrings);

CPPUNIT_TEST_SUITE_REGISTRATION(cCollapseDotsTests);

///////////////////////////////////////////////////////////////////////////////

class cFilePathTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cFilePathTests);
      CPPUNIT_TEST(TestIsFullPath);
      CPPUNIT_TEST(TestCompare);
   CPPUNIT_TEST_SUITE_END();

   void TestIsFullPath()
   {
      CPPUNIT_ASSERT(cFilePath("C:\\p1\\p2").IsFullPath());
      CPPUNIT_ASSERT(!cFilePath("C:p1\\p2").IsFullPath());
      CPPUNIT_ASSERT(!cFilePath("C:\\p1\\p2\\..\\p3").IsFullPath());
      CPPUNIT_ASSERT(cFilePath("/p1/p2/p3").IsFullPath());
      CPPUNIT_ASSERT(!cFilePath("p1/p2/p3").IsFullPath());
      CPPUNIT_ASSERT(!cFilePath("/p1/p2/../p3").IsFullPath());
   }

   void TestCompare()
   {
      CPPUNIT_ASSERT(cFilePath("c:\\p1\\p2\\p3").Compare(cFilePath("c:/p1/p2/p3")) == 0);
      CPPUNIT_ASSERT(cFilePath("c:\\p1\\p2\\p3").Compare(cFilePath("c:\\p4\\p5\\p6")) < 0);
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(cFilePathTests);

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
