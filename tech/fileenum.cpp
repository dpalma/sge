///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "tech/fileenum.h"
#include "tech/filespec.h"
#include "tech/filepath.h"
#include "tech/techstring.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "tech/dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(EnumFiles);

#define LocalMsg(msg)            DebugMsgEx(EnumFiles,msg)
#define LocalMsg1(msg,a)         DebugMsgEx1(EnumFiles,msg,(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(EnumFiles,msg,(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(EnumFiles,msg,(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)   DebugMsgEx4(EnumFiles,msg,(a),(b),(c),(d))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(EnumFiles,(cond),msg)
#define LocalMsgIf1(cond,msg,a)        DebugMsgIfEx1(EnumFiles,(cond),msg,(a))
#define LocalMsgIf2(cond,msg,a,b)      DebugMsgIfEx2(EnumFiles,(cond),msg,(a),(b))
#define LocalMsgIf3(cond,msg,a,b,c)    DebugMsgIfEx3(EnumFiles,(cond),msg,(a),(b),(c))
#define LocalMsgIf4(cond,msg,a,b,c,d)  DebugMsgIfEx4(EnumFiles,(cond),msg,(a),(b),(c),(d))

#ifdef _WIN32

///////////////////////////////////////////////////////////////////////////////

AssertAtCompileTime(kFA_ReadOnly == FILE_ATTRIBUTE_READONLY);
AssertAtCompileTime(kFA_Hidden == FILE_ATTRIBUTE_HIDDEN);
AssertAtCompileTime(kFA_System == FILE_ATTRIBUTE_SYSTEM);
AssertAtCompileTime(kFA_Directory == FILE_ATTRIBUTE_DIRECTORY);
AssertAtCompileTime(kFA_Archive == FILE_ATTRIBUTE_ARCHIVE);
AssertAtCompileTime(kFA_Normal == FILE_ATTRIBUTE_NORMAL);
AssertAtCompileTime(kFA_Temporary == FILE_ATTRIBUTE_TEMPORARY);
AssertAtCompileTime(kFA_Compressed == FILE_ATTRIBUTE_COMPRESSED);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEnumFilesWin32
//

class cEnumFilesWin32 : public cComObject<IMPLEMENTS(IEnumFiles)>
{
   cEnumFilesWin32(const cEnumFilesWin32 &);
   const cEnumFilesWin32 & operator =(const cEnumFilesWin32 &);

public:
   cEnumFilesWin32(const cFileSpec & spec);
   ~cEnumFilesWin32();

   virtual tResult Next(ulong count, cFileSpec * pFileSpecs, uint * pAttribs, ulong * pnElements);
   virtual tResult Skip(ulong count);
   virtual tResult Reset();
   virtual tResult Clone(IEnumFiles * * ppEnum);

private:
   // Internal function with more lenient arg checking for use by both Skip and Next
   tResult GetNext(ulong count, cFileSpec * pFileSpecs, uint * pAttribs, ulong * pnElements);
   const cFileSpec & GetSpec() const { return m_spec; }
   const cFilePath & GetPath() const { return m_path; }
   const cFileSpec m_spec;
   cFilePath m_path;
   HANDLE m_hFinder;
};

////////////////////////////////////////

cEnumFilesWin32::cEnumFilesWin32(const cFileSpec & spec)
 : m_spec(spec)
 , m_hFinder(NULL)
{
   spec.GetPath(&m_path);
}

////////////////////////////////////////

cEnumFilesWin32::~cEnumFilesWin32()
{
   Reset();
}

////////////////////////////////////////

tResult cEnumFilesWin32::Next(ulong count, cFileSpec * pFileSpecs, uint * pAttribs, ulong * pnElements)
{
   if (count == 0)
   {
      return E_INVALIDARG;
   }

   if (pFileSpecs == NULL || pAttribs == NULL)
   {
      return E_POINTER;
   }

   return GetNext(count, pFileSpecs, pAttribs, pnElements);
}

////////////////////////////////////////

tResult cEnumFilesWin32::Skip(ulong count)
{
   if (count == 0)
   {
      return E_INVALIDARG;
   }

   return GetNext(count, NULL, NULL, NULL);
}

////////////////////////////////////////

tResult cEnumFilesWin32::Reset()
{
   // Close finder handle to force FindFirstFile on next call to Next
   if ((m_hFinder != NULL) && (m_hFinder != INVALID_HANDLE_VALUE))
   {
      FindClose(m_hFinder);
      m_hFinder = NULL;
   }
   return S_OK;
}

////////////////////////////////////////

tResult cEnumFilesWin32::Clone(IEnumFiles * * ppEnum)
{
   return EnumFiles(GetSpec(), ppEnum);
}

////////////////////////////////////////

tResult cEnumFilesWin32::GetNext(ulong count, cFileSpec * pFileSpecs, uint * pAttribs, ulong * pnElements)
{
   if (count == 0)
   {
      return E_INVALIDARG;
   }

   bool bFirst = true, bFound = false;
   ulong nFound = 0;

   WIN32_FIND_DATA findData;

   do
   {
      if (m_hFinder == NULL)
      {
         m_hFinder = FindFirstFile(GetSpec().CStr(), &findData);
         if ((m_hFinder == NULL) || (m_hFinder == INVALID_HANDLE_VALUE))
         {
            return E_FAIL;
         }
         else
         {
            bFound = true;
         }
      }
      else
      {
         bFound = !!FindNextFile(m_hFinder, &findData);
      }

      // If first call failed, there is no data
      if (!bFound && bFirst)
      {
         return E_FAIL;
      }

      bFirst = false;

      if (bFound)
      {
         bool bIsDir = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;

         LocalMsgIf1(bIsDir, "Found directory %s\n", findData.cFileName);
         LocalMsgIf1(!bIsDir, "Found file %s\n", findData.cFileName);

         if (bIsDir)
         {
            if (_tcscmp(findData.cFileName, _T(".")) == 0 ||
                _tcscmp(findData.cFileName, _T("..")) == 0)
            {
               continue;
            }
         }

         if (pFileSpecs != NULL)
         {
            pFileSpecs[nFound] = cFileSpec(findData.cFileName);
            pFileSpecs[nFound].SetPath(GetPath());
         }

         if (pAttribs != NULL)
         {
            pAttribs[nFound] = findData.dwFileAttributes;
         }

         nFound++;
      }
   }
   while (bFound && (nFound < count));

   if (pnElements != NULL)
   {
      *pnElements = nFound;
   }

   return (nFound == count) ? S_OK : S_FALSE;
}

#else

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEnumFilesPosix
//

class cEnumFilesPosix : public cComObject<IMPLEMENTS(IEnumFiles)>
{
   cEnumFilesPosix(const cEnumFilesPosix &);
   const cEnumFilesPosix & operator =(const cEnumFilesPosix &);

public:
   cEnumFilesPosix(const cFileSpec & spec);
   ~cEnumFilesPosix();

   virtual tResult Next(ulong count, cFileSpec * pFileSpecs, uint * pAttribs, ulong * pnElements);
   virtual tResult Skip(ulong count);
   virtual tResult Reset();
   virtual tResult Clone(IEnumFiles * * ppEnum);

private:
   // Internal function with more lenient arg checking for use by both Skip and Next
   tResult GetNext(ulong count, cFileSpec * pFileSpecs, uint * pAttribs, ulong * pnElements);
   const cFileSpec & GetSpec() const { return m_spec; }
   const cFilePath & GetPath() const { return m_path; }
   const cFileSpec m_spec;
   cFilePath m_path;
   const cStr m_fileName;
   DIR * m_pDir;
   tResult m_lastResult;
};

////////////////////////////////////////

cEnumFilesPosix::cEnumFilesPosix(const cFileSpec & spec)
 : m_spec(spec)
 , m_fileName(spec.GetFileName())
 , m_pDir(NULL)
 , m_lastResult(E_FAIL)
{
   spec.GetPath(&m_path);
}

////////////////////////////////////////

cEnumFilesPosix::~cEnumFilesPosix()
{
   Reset();
}

////////////////////////////////////////

tResult cEnumFilesPosix::Next(ulong count, cFileSpec * pFileSpecs, uint * pAttribs, ulong * pnElements)
{
   return GetNext(count, pFileSpecs, pAttribs, pnElements);
}

////////////////////////////////////////

tResult cEnumFilesPosix::Skip(ulong count)
{
   return GetNext(count, NULL, NULL, NULL);
}

////////////////////////////////////////

tResult cEnumFilesPosix::Reset()
{
   if (m_pDir != NULL)
   {
      rewinddir(m_pDir);
      return S_OK;
   }
   else
   {
      return S_FALSE;
   }
}

////////////////////////////////////////

tResult cEnumFilesPosix::Clone(IEnumFiles * * ppEnum)
{
   return EnumFiles(GetSpec(), ppEnum);
}

////////////////////////////////////////

tResult cEnumFilesPosix::GetNext(ulong count, cFileSpec * pFileSpecs, uint * pAttribs, ulong * pnElements)
{
   if (count == 0)
   {
      return E_INVALIDARG;
   }

   if (pFileSpecs == NULL || pAttribs == NULL)
   {
      return E_POINTER;
   }

   if (m_pDir == NULL)
   {
      m_pDir = opendir(GetPath().CStr());
      if (m_pDir == NULL)
      {
         return E_FAIL;
      }
   }

   bool bFound = false;
   ulong nFound = 0;

   struct dirent * pEnt = readdir(m_pDir);
   while ((pEnt != NULL) && (nFound < count))
   {
      LocalMsg1("Found entry \"%s\"\n", pEnt->d_name);
      if (strcmp(pEnt->d_name, ".") != 0 && strcmp(pEnt->d_name, "..") != 0
         && WildCardMatch(m_fileName.c_str(), pEnt->d_name))
      {
         cFileSpec entry(pEnt->d_name);
         entry.SetPath(GetPath());

         ulong attribs = 0;

         struct stat fstat;
         if (stat(entry.CStr(), &fstat) == 0)
         {
            if (S_ISDIR(fstat.st_mode))
            {
               attribs |= kFA_Directory;
            }
         }

         if (pFileSpecs != NULL)
         {
            pFileSpecs[nFound] = entry;
         }

         if (pAttribs != NULL)
         {
            pAttribs[nFound] = attribs;
         }

         nFound++;
      }

      pEnt = readdir(m_pDir);
   }

   LocalMsg1("Found %d entries\n", nFound);

   if (pnElements != NULL)
   {
      *pnElements = nFound;
   }

   tResult result = (nFound == count) ? S_OK : S_FALSE;

   if ((result == S_FALSE) && (m_lastResult == S_FALSE))
   {
      result = E_FAIL;
   }

   m_lastResult = result;

   return result;
}

#endif

///////////////////////////////////////////////////////////////////////////////

tResult EnumFiles(const cFileSpec & spec, IEnumFiles * * ppEnumFiles)
{
   if (ppEnumFiles == NULL)
   {
      return E_POINTER;
   }

#ifdef _WIN32
   cAutoIPtr<IEnumFiles> pEnumFiles(static_cast<IEnumFiles*>(new cEnumFilesWin32(spec)));
#else
   cAutoIPtr<IEnumFiles> pEnumFiles(static_cast<IEnumFiles*>(new cEnumFilesPosix(spec)));
#endif

   if (!pEnumFiles)
   {
      return E_OUTOFMEMORY;
   }

   return pEnumFiles.GetPointer(ppEnumFiles);
}

///////////////////////////////////////////////////////////////////////////////
