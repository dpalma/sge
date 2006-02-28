///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "filepath.h"

#ifdef HAVE_CPPUNITLITE2
#include "CppUnitLite2.h"
#endif

#include <cstring>
#include <cstdlib>
#include <locale>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
extern "C"
{
   __declspec(dllimport) uint STDCALL GetCurrentDirectoryA(uint, char *);
   __declspec(dllimport) uint STDCALL GetCurrentDirectoryW(uint, wchar_t *);
}
#ifdef _UNICODE
#define GetCurrentDirectory  GetCurrentDirectoryW
#else
#define GetCurrentDirectory  GetCurrentDirectoryA
#endif // !UNICODE
#endif // _WIN32

///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
static const tChar kPathSep = _T('\\');
#else
static const tChar kPathSep = _T('/');
#endif
static const tChar szPathSeps[] = _T("\\/");

///////////////////////////////////////////////////////////////////////////////

inline bool IsPathSep(tChar c)
{
   return (c == _T('/') || c == _T('\\'));
}

inline bool IsTwoDots(const tChar * psz)
{
   return (_tcsncmp(_T("/.."), psz, 3) == 0 ||
      _tcsncmp(_T("\\.."), psz, 3) == 0 ||
      _tcsncmp(_T(".."), psz, 2) == 0);
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFilePath
//

////////////////////////////////////////

cFilePath::cFilePath()
{
   memset(m_szPath, 0, sizeof(m_szPath));
}
   
////////////////////////////////////////

cFilePath::cFilePath(const cFilePath & other)
{
   _tcscpy(m_szPath, other.m_szPath);
}

////////////////////////////////////////

cFilePath::cFilePath(const tChar * pszPath)
{
   _tcsncpy(m_szPath, pszPath, _countof(m_szPath));
   m_szPath[_countof(m_szPath) - 1] = 0;
}

////////////////////////////////////////

cFilePath::cFilePath(const tChar * pszPath, size_t pathLen)
{
   if (pathLen > _countof(m_szPath))
   {
      pathLen = _countof(m_szPath);
   }
   _tcsncpy(m_szPath, pszPath, pathLen);
   m_szPath[pathLen - 1] = 0;
}

////////////////////////////////////////

const cFilePath & cFilePath::operator =(const cFilePath & other)
{
   _tcscpy(m_szPath, other.m_szPath);
   return *this;
}

////////////////////////////////////////

bool cFilePath::operator ==(const cFilePath & other)
{
   return FilePathCompare(*this, other) == 0;
}

////////////////////////////////////////

bool cFilePath::operator !=(const cFilePath & other)
{
   return FilePathCompare(*this, other) != 0;
}

////////////////////////////////////////

const tChar * cFilePath::CStr() const
{
   return m_szPath;
}

////////////////////////////////////////

size_t cFilePath::GetLength() const
{
   return _tcslen(m_szPath);
}

////////////////////////////////////////

bool cFilePath::IsEmpty() const
{
   return (m_szPath[0] == 0);
}

////////////////////////////////////////

bool cFilePath::AddRelative(const tChar * pszDir)
{
   if (pszDir == NULL)
   {
      return false;
   }
   size_t dirLength = _tcslen(pszDir);
   size_t thisLength = GetLength();
   bool bDirStartsWithSep = (*pszDir == _T('/')) || (*pszDir == _T('\\'));
   bool bThisEndsWithSep = (m_szPath[thisLength - 1] == _T('/')) || (m_szPath[thisLength - 1] == _T('\\'));
   if (!IsEmpty() && !bDirStartsWithSep && !bThisEndsWithSep)
   {
      if (thisLength < (_countof(m_szPath) - 1))
      {
         m_szPath[thisLength] = kPathSep;
         m_szPath[thisLength+1] = 0;
      }
      else
      {
         return false;
      }
   }
   else if (bDirStartsWithSep && bThisEndsWithSep)
   {
      pszDir++;
      dirLength--;
   }
   _tcsncat(m_szPath, pszDir, Min(_countof(m_szPath) - thisLength - 1, dirLength));
   m_szPath[_countof(m_szPath) - 1] = 0;
   return true;
}

////////////////////////////////////////

bool cFilePath::IsFullPath() const
{
   if (GetLength() >= 3 &&
       _istalpha(m_szPath[0]) &&
       m_szPath[1] == _T(':') &&
       IsPathSep(m_szPath[2]) &&
       _tcsstr(CStr(), _T("..")) == NULL)
   {
      return true;
   }

   if (IsPathSep(m_szPath[0]) && _tcsstr(CStr(), _T("..")) == NULL)
   {
      return true;
   }

   return false;
}

////////////////////////////////////////

void cFilePath::MakeFullPath()
{
   if (!IsFullPath())
   {
      cFilePath temp(GetCwd());
      temp.AddRelative(CStr());
      temp.CollapseDots();
      *this = temp;
   }
}

////////////////////////////////////////

cFilePath cFilePath::CollapseDots()
{
   int nDirs = 0;
   const tChar * * dirs = static_cast<const tChar**>(alloca(GetLength() * sizeof(const tChar *)));
   memset(dirs, 0, sizeof(dirs));

   bool bHadLeadingSeparator = false;
   if (IsPathSep(m_szPath[0]))
   {
      bHadLeadingSeparator = true;
   }

   const tChar * p = CStr();
   dirs[nDirs++] = p;
   p = _tcsinc(p);
   for (; *p != 0; p = _tcsinc(p))
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
      tChar szDir[kMaxPath];
      if (i < (nDirs - 1))
      {
         size_t len = dirs[i + 1] - dirs[i] + 1;
         _tcsncpy(szDir, dirs[i], len);
         szDir[len - 1] = 0;
      }
      else
      {
         _tcscpy(szDir, dirs[i]);
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

   std::string result;

   for (i = 0; i < nDirs; i++)
   {
      if (dirs[i] != NULL)
      {
         size_t len = _tcscspn(dirs[i] + 1, szPathSeps) + 1;
         result.append(std::string(dirs[i], len).c_str());
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
   tChar szCwd[kMaxPath];
#ifdef _WIN32
   GetCurrentDirectory(_countof(szCwd), szCwd);
#else
   getcwd(szCwd, _countof(szCwd));
#endif
   return cFilePath(szCwd);
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNITLITE2

////////////////////////////////////////

TEST(TestFilePathCollapseDots)
{
   static const tChar * collapseDotsTestStrings[] =
   {
      _T("/p1/p2/p3"),           _T("/p1/p2/p3"),        // nothing to do
      _T("/p1/p2/p3/.."),        _T("/p1/p2"),           // parent directory at end
      _T("../p1/p2/p3"),         _T("../p1/p2/p3"),      // parent directory at begining, can't collapse
      _T("/p1/p2/../p3"),        _T("/p1/p3"),           // parent directory mid-string
      _T("/p1/p2/p3/../.."),     _T("/p1"),              // multiple parent directories at end
      _T("../../p1/p2/p3"),      _T("../../p1/p2/p3"),   // multiple parent directories at begining, can't collapse
      _T("/p1/p2/../../p3"),     _T("/p3"),              // multiple parent directories mid-string
      _T("/p1/p2/p3/../../.."),  _T(""),                 // total collapse
      _T("p1/../p2/p3"),         _T("p2/p3"),            // ensure relative path does not become absolute
      _T("../../p1/../p2/p3"),   _T("../../p2/p3"),      // collapse middle but not beginning
   };

   for (int i = 0; i < _countof(collapseDotsTestStrings); i += 2)
   {
      cFilePath temp(cFilePath(collapseDotsTestStrings[i]).CollapseDots());
      CHECK_EQUAL(FilePathCompare(cFilePath(collapseDotsTestStrings[i + 1]), temp), 0);
   }
}

////////////////////////////////////////

TEST(TestFilePathIsFullPath)
{
   CHECK(cFilePath("C:\\p1\\p2").IsFullPath());
   CHECK(!cFilePath("C:p1\\p2").IsFullPath());
   CHECK(!cFilePath("C:\\p1\\p2\\..\\p3").IsFullPath());
   CHECK(cFilePath("/p1/p2/p3").IsFullPath());
   CHECK(!cFilePath("p1/p2/p3").IsFullPath());
   CHECK(!cFilePath("/p1/p2/../p3").IsFullPath());
}

////////////////////////////////////////

TEST(TestFilePathAddRelative)
{
   {
      cFilePath path("c:\\p1\\p2\\");
      CHECK(path.AddRelative("\\p3"));
      CHECK_EQUAL(_tcscmp(path.CStr(), _T("c:\\p1\\p2\\p3")), 0);
   }

   {
      cFilePath path("c:\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory");
      CHECK(path.AddRelative("\\theend"));
      CHECK_EQUAL(_tcscmp(path.CStr(), _T("c:\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\directory\\direct")), 0);
   }
}

#endif // HAVE_CPPUNITLITE2

///////////////////////////////////////////////////////////////////////////////
