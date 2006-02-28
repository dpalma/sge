///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "filespec.h"
#include "filepath.h"

#ifdef HAVE_CPPUNITLITE2
#include "CppUnitLite2.h"
#endif

#include <cstring>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

static const tChar kExtensionSep = _T('.');

template <typename C>
inline bool IsPathSep(C c)
{
   return (c == _T('\\')) || (c == _T('/'));
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFileSpec
//

///////////////////////////////////////

cFileSpec::cFileSpec()
{
   memset(m_szFile, 0, sizeof(m_szFile));
}

///////////////////////////////////////

cFileSpec::cFileSpec(const tChar * pszFile)
{
   _tcsncpy(m_szFile, pszFile, _countof(m_szFile));
   m_szFile[_countof(m_szFile) - 1] = 0;
}

///////////////////////////////////////

cFileSpec::cFileSpec(const cFileSpec & other)
{
   _tcscpy(m_szFile, other.m_szFile);
}

///////////////////////////////////////

const cFileSpec & cFileSpec::operator =(const cFileSpec & other)
{
   _tcscpy(m_szFile, other.m_szFile);
   return *this;
}

///////////////////////////////////////

bool cFileSpec::operator ==(const cFileSpec & other)
{
   return FileSpecCompare(*this, other) == 0;
}

///////////////////////////////////////

bool cFileSpec::operator !=(const cFileSpec & other)
{
   return FileSpecCompare(*this, other) != 0;
}

///////////////////////////////////////

const tChar * cFileSpec::CStr() const
{
   return m_szFile;
}

///////////////////////////////////////

size_t cFileSpec::GetLength() const
{
   return _tcslen(m_szFile);
}

///////////////////////////////////////

bool cFileSpec::IsEmpty() const
{
   return (m_szFile[0] == 0);
}

///////////////////////////////////////

const tChar * cFileSpec::GetFileName() const
{
   const tChar * p = _tcsrchr(CStr(), _T('\\'));
   if (p == NULL)
   {
      p = _tcsrchr(CStr(), _T('/'));
   }
   if (p != NULL)
   {
      return (p + 1);
   }
   return CStr();
}

///////////////////////////////////////

bool cFileSpec::GetFileNameNoExt(std::string * pFileName) const
{
   if (pFileName != NULL)
   {
      const tChar * p = GetFileName();
      Assert(p != NULL);
      const tChar * pExt = _tcsrchr(p, kExtensionSep);
      if (pExt != NULL)
      {
         tChar * pTemp = reinterpret_cast<tChar*>(alloca(pExt - p + 1));
         _tcsncpy(pTemp, p, pExt - p);
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

const tChar * cFileSpec::GetFileExt() const
{
   const tChar * pszExt = _tcsrchr(CStr(), kExtensionSep);
   if (pszExt)
   {
      return _tcsinc(pszExt);
   }
   else
   {
      return _T("");
   }
}

///////////////////////////////////////

bool cFileSpec::SetFileExt(const tChar * pszExt)
{
   if ((pszExt != NULL) && !IsEmpty())
   {
      tChar * p = _tcsrchr(m_szFile, kExtensionSep);
      if (p == NULL)
      {
         p = m_szFile + GetLength();
      }

      if (static_cast<size_t>(&m_szFile[_countof(m_szFile)] - p - 1) >= _tcslen(pszExt))
      {
         *p++ = kExtensionSep;
         _tcscpy(p, pszExt);
         return true;
      }
   }
   return false;
}

///////////////////////////////////////

void cFileSpec::SetPath(const cFilePath & path)
{
   cFilePath temp(path);
   temp.AddRelative(GetFileName());
   *this = cFileSpec(temp.CStr());
}

///////////////////////////////////////

bool cFileSpec::GetPath(cFilePath * pPath) const
{
   if (pPath != NULL)
   {
      const tChar * pszFileName = GetFileName();
      if (pszFileName != NULL)
      {
         const tChar * pszFullName = CStr();
         *pPath = cFilePath(pszFullName, pszFileName - pszFullName);
         return true;
      }
   }
   return false;
}

////////////////////////////////////////

template <typename T>
static int FileSpecOrPathCompare(const T & f1, const T & f2, int (* pfn)(int))
{
   if (f1.IsEmpty() && !f2.IsEmpty())
   {
      return 1;
   }
   else if (!f1.IsEmpty() && f2.IsEmpty())
   {
      return -1;
   }
   else if (f1.IsEmpty() && f2.IsEmpty())
   {
      return 0;
   }
   const tChar * p1 = f1.CStr(), * p1end = p1 + f1.GetLength();
   const tChar * p2 = f2.CStr(), * p2end = p2 + f2.GetLength();
   for (; (p1 < p1end) && (p2 < p2end); p1++, p2++)
   {
      if (IsPathSep(*p1) && IsPathSep(*p2))
      {
         continue;
      }
      int diff = (pfn != NULL) ? ((*pfn)(*p1) - (*pfn)(*p2)) : (*p1 - *p2);
      if (diff != 0)
      {
         return (diff < 0) ? -1 : 1;
      }
   }
   return 0;
}

////////////////////////////////////////

int FileSpecCompare(const cFileSpec & f1, const cFileSpec & f2)
{
   return FileSpecOrPathCompare(f1, f2, NULL);
}

////////////////////////////////////////

int FileSpecCompareNoCase(const cFileSpec & f1, const cFileSpec & f2)
{
   return FileSpecOrPathCompare(f1, f2, tolower);
}

////////////////////////////////////////

int FilePathCompare(const cFilePath & f1, const cFilePath & f2)
{
   return FileSpecOrPathCompare(f1, f2, NULL);
}

////////////////////////////////////////

int FilePathCompareNoCase(const cFilePath & f1, const cFilePath & f2)
{
   return FileSpecOrPathCompare(f1, f2, tolower);
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNITLITE2

///////////////////////////////////////

TEST(TestFileSpecGetFileName)
{
   CHECK(strcmp(cFileSpec("c:\\p1\\p2\\p3.ext").GetFileName(), "p3.ext") == 0);
   CHECK(strcmp(cFileSpec("C:\\P1\\P2\\P3.EXT").GetFileName(), "P3.EXT") == 0);
   CHECK(strcmp(cFileSpec("c:\\p1\\p2.p3").GetFileName(), "p2.p3") == 0);
   CHECK(strcmp(cFileSpec("c:\\foo\\bar").GetFileName(), "bar") == 0);
}

///////////////////////////////////////

TEST(TestFileSpecGetSetFileExt)
{
   {
      cFileSpec fs("c:\\path1\\path2\\file");
      CHECK(strlen(fs.GetFileExt()) == 0);
      CHECK(fs.SetFileExt("ext"));
      CHECK(strcmp(fs.GetFileExt(), "ext") == 0);
      CHECK(strcmp(fs.CStr(), "c:\\path1\\path2\\file.ext") == 0);
   }

   {
      cFileSpec fs("c:\\path1\\path2\\file.TST");
      CHECK(strcmp(fs.GetFileExt(), "TST") == 0);
      CHECK(fs.SetFileExt("ext"));
      CHECK(strcmp(fs.GetFileExt(), "ext") == 0);
      CHECK(strcmp(fs.CStr(), "c:\\path1\\path2\\file.ext") == 0);
   }
}

///////////////////////////////////////

TEST(TestFileSpecSetPath)
{
   {
      cFileSpec fs1("c:\\p1\\p2\\p3\\p4\\file.ext");
      fs1.SetPath(cFilePath());
      CHECK(strcmp(fs1.CStr(), "file.ext") == 0);
   }

   {
      cFileSpec fs1("c:\\p1\\p2\\p3\\p4\\file.ext");
      fs1.SetPath(cFilePath("D:\\path"));
      CHECK(strcmp(fs1.CStr(), "D:\\path\\file.ext") == 0);
   }
}

///////////////////////////////////////

TEST(TestFileSpecGetPath)
{
   cFileSpec test("c:\\p1\\p2\\p3\\p4\\file.ext");
   cFilePath testPath;
   CHECK(test.GetPath(&testPath));
   CHECK(FilePathCompare(testPath, cFilePath("c:\\p1\\p2\\p3\\p4")) == 0);
}

///////////////////////////////////////

TEST(TestFileSpecOperatorEquals)
{
   CHECK(cFileSpec("c:\\p1\\p2\\p3.ext") == cFileSpec("c:/p1/p2/p3.ext"));
}

///////////////////////////////////////

TEST(TestFileSpecOperatorNotEquals)
{
   CHECK(cFileSpec("C:\\P1\\P2\\P3.EXT") != cFileSpec("c:/p1/p2/p3.ext"));
}

///////////////////////////////////////

TEST(TestFileSpecCompare)
{
   CHECK(FileSpecCompare(cFileSpec("c:\\p1\\p2\\p3"), cFileSpec("c:/p1/p2/p3")) == 0);
   CHECK(FileSpecCompare(cFileSpec("C:\\P1\\P2\\P3"), cFileSpec("c:/p1/p2/p3")) != 0);
   CHECK(FileSpecCompare(cFileSpec("c:\\p1\\p2\\p3"), cFileSpec("c:\\p4\\p5\\p6")) < 0);
   CHECK(FileSpecCompare(cFileSpec("c:\\P1\\P2\\P3.EXT"), cFileSpec("c:/p1/p2/p3.ext")) < 0);
   CHECK(FileSpecCompare(cFileSpec("c:\\p1\\p2.p3"), cFileSpec("c:\\p4\\p5.p6")) < 0);
}

///////////////////////////////////////

TEST(TestFileSpecCompareNoCase)
{
   CHECK(FileSpecCompareNoCase(cFileSpec("C:\\P1\\P2\\P3.EXT"), cFileSpec("c:/p1/p2/p3.ext")) == 0);
   CHECK(FileSpecCompareNoCase(cFileSpec("c:\\p1\\p2.p3"), cFileSpec("c:\\p4\\p5.p6")) < 0);
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

TEST(AttempFileSpecSetFileExtBufferOverrunAttack)
{
   static const char szTestFileName1[] = "foo.txt";
   static const char szTestFileName2[] = "foo";

   struct sAttack
   {
      char szExt[260 - 4]; // the 4 is the length of "foo." in szTestFileName
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
      Vulnerable(szTestFileName1, (char*)&attack);
      Vulnerable(szTestFileName2, (char*)&attack);
      CHECK(!g_bSetFileExtBufferOverrunSucceeded);
   }
   catch (...)
   {
      CHECK(!g_bSetFileExtBufferOverrunSucceeded);
      throw;
   }
}

///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_CPPUNIT
