///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "filepath.h"
#include "filespec.h"

#include <string.h>
#include <stdlib.h>
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
         int len = strcspn(dirs[i] + 1, "/\\") + 1;
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
   memset(m_szPath, 0, sizeof(m_szPath));
}

///////////////////////////////////////

cFilePath::cFilePath(const char * pszPath)
{
   if (pszPath != NULL)
   {
      strncpy(m_szPath, pszPath, sizeof(m_szPath));
      m_szPath[sizeof(m_szPath) - 1] = '\0';
   }
}

///////////////////////////////////////

static void PathCat(char * pszDest, const char * pszSrc)
{
   int destLen = strlen(pszDest);

   if ((*pszSrc != '/') &&
       (*pszSrc != '\\') &&
       (pszDest[destLen - 1] != '/') &&
       (pszDest[destLen - 1] != '\\'))
   {
      strcat(pszDest, kPathSepStr);
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

int cFilePath::ListDirs(std::vector<std::string> * pDirs)
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

#define COLLAPSE_DOTS_COMPARE(path, collapsed) \
   do { \
      char sz[kMaxPath]; \
      CollapseDots((path), sz, kMaxPath); \
      CPPUNIT_ASSERT(strcmp((collapsed), sz) == 0); \
   } while (0)

#define COLLAPSE_DOTS_SAME(path) \
   COLLAPSE_DOTS_COMPARE(path, path)

class cCollapseDotsTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cCollapseDotsTests);
      CPPUNIT_TEST(testCollapseDots);
      CPPUNIT_TEST(testCollapseDotsBufferSize);
      CPPUNIT_TEST(testCollapseDotsDriveLetters);
   CPPUNIT_TEST_SUITE_END();

public:
   void testCollapseDots()
   {
      // nothing to do
      COLLAPSE_DOTS_SAME("/p1/p2/p3");

      // parent directory at end
      COLLAPSE_DOTS_COMPARE("/p1/p2/p3/..", "/p1/p2");

      // parent directory at begining, can't collapse
      COLLAPSE_DOTS_SAME("../p1/p2/p3");

      // parent directory mid-string
      COLLAPSE_DOTS_COMPARE("/p1/p2/../p3", "/p1/p3");

      // multiple parent directories at end
      COLLAPSE_DOTS_COMPARE("/p1/p2/p3/../..", "/p1");

      // multiple parent directories at begining, can't collapse
      COLLAPSE_DOTS_SAME("../../p1/p2/p3");

      // multiple parent directories mid-string
      COLLAPSE_DOTS_COMPARE("/p1/p2/../../p3", "/p3");

      // total collapse
      COLLAPSE_DOTS_COMPARE("/p1/p2/p3/../../..", "");

      // ensure relative path does not become absolute
      COLLAPSE_DOTS_COMPARE("p1/../p2/p3", "p2/p3");

      COLLAPSE_DOTS_COMPARE("../../p1/../p2/p3", "../../p2/p3");

      // too many levels
//      COLLAPSE_DOTS_SAME("/p1/p2/../../../p3");

#if 0
      CollapseDots("./p1/p2", sz);
      CPPUNIT_ASSERT(strcmp("p1/p2", sz) == 0);

      CollapseDots("./p1/p2/../../p3", sz);
      CPPUNIT_ASSERT(strcmp("p3", sz) == 0);
#endif
   }

   void testCollapseDotsBufferSize()
   {
      // @TODO: ensure no buffer over-runs and return value is appropriate
   }

   void testCollapseDotsDriveLetters()
   {
      // @TODO: test paths with drive letters
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(cCollapseDotsTests);

///////////////////////////////////////////////////////////////////////////////

class cFilePathTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cFilePathTests);
      CPPUNIT_TEST(testIsFullPath);
   CPPUNIT_TEST_SUITE_END();

public:
   void testIsFullPath()
   {
      CPPUNIT_ASSERT(cFilePath("C:\\p1\\p2").IsFullPath());
      CPPUNIT_ASSERT(!cFilePath("C:p1\\p2").IsFullPath());
      CPPUNIT_ASSERT(!cFilePath("C:\\p1\\p2\\..\\p3").IsFullPath());
      CPPUNIT_ASSERT(cFilePath("/p1/p2/p3").IsFullPath());
      CPPUNIT_ASSERT(!cFilePath("p1/p2/p3").IsFullPath());
      CPPUNIT_ASSERT(!cFilePath("/p1/p2/../p3").IsFullPath());
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(cFilePathTests);

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
