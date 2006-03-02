///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "fileenum.h"
#include "filespec.h"
#include "filepath.h"
#include "techstring.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#endif

#include "dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(EnumFiles);

#ifdef _WIN32

///////////////////////////////////////////////////////////////////////////////

AssertOnce(kFA_ReadOnly == FILE_ATTRIBUTE_READONLY);
AssertOnce(kFA_Hidden == FILE_ATTRIBUTE_HIDDEN);
AssertOnce(kFA_System == FILE_ATTRIBUTE_SYSTEM);
AssertOnce(kFA_Directory == FILE_ATTRIBUTE_DIRECTORY);
AssertOnce(kFA_Archive == FILE_ATTRIBUTE_ARCHIVE);
AssertOnce(kFA_Normal == FILE_ATTRIBUTE_NORMAL);
AssertOnce(kFA_Temporary == FILE_ATTRIBUTE_TEMPORARY);
AssertOnce(kFA_Compressed == FILE_ATTRIBUTE_COMPRESSED);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEnumFilesWin32
//

class cEnumFilesWin32 : public cComObject<IMPLEMENTS(IEnumFiles)>
{
   cEnumFilesWin32(const cEnumFilesWin32 &);
   void operator =(const cEnumFilesWin32 &);
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

   bool bFound = false;
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
         if (FindNextFile(m_hFinder, &findData))
         {
            bFound = true;
         }
      }

      if (bFound)
      {
         DebugMsgEx1(EnumFiles, "Found file %s\n", findData.cFileName);

         if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
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

////////////////////////////////////////

tResult EnumFiles(const cFileSpec & spec, IEnumFiles * * ppEnumFiles)
{
   if (ppEnumFiles == NULL)
      return E_POINTER;

   cAutoIPtr<cEnumFilesWin32> pEnumFiles(new cEnumFilesWin32(spec));
   if (!pEnumFiles)
      return E_OUTOFMEMORY;

   *ppEnumFiles = static_cast<IEnumFiles*>(CTAddRef(pEnumFiles));
   return S_OK;
}

#else

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEnumFilesPosix
//

class cEnumFilesPosix : public cComObject<IMPLEMENTS(IEnumFiles)>
{
   cEnumFilesPosix(const cEnumFilesPosix &);
   void operator =(const cEnumFilesPosix &);
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
};

////////////////////////////////////////

cEnumFilesPosix::cEnumFilesPosix(const cFileSpec & spec)
 : m_spec(spec)
 , m_fileName(spec.GetFileName())
 , m_pDir(NULL)
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

tResult cEnumFilesPosix::Skip(ulong count)
{
   if (count == 0)
   {
      return E_INVALIDARG;
   }

   return GetNext(count, NULL, NULL, NULL);
}

////////////////////////////////////////

tResult cEnumFilesPosix::Reset()
{
   if (m_pDir != NULL)
   {
      rewinddir(m_pDir);
   }
   return S_OK;
}

////////////////////////////////////////

tResult cEnumFilesPosix::Clone(IEnumFiles * * ppEnum)
{
   return EnumFiles(GetSpec(), ppEnum);
}

////////////////////////////////////////

static bool WildcardMatch(const tChar * pszWildcard, const tChar * pszEntry)
{
   if (strlen(pszWildcard) == 1 && *pszWildcard == '*')
      return true;
   // TODO: implement actual wildcard matching
   return strcmp(pszWildcard, pszEntry);
}

////////////////////////////////////////

// TODO: THIS CODE IS UNTESTED.
tResult cEnumFilesPosix::GetNext(ulong count, cFileSpec * pFileSpecs, uint * pAttribs, ulong * pnElements)
{
   if (count == 0)
   {
      return E_INVALIDARG;
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
      if (strcmp(pEnt->d_name, ".") != 0 && strcmp(pEnt->d_name, "..") != 0
         && WildcardMatch(m_fileName.c_str(), pEnt->d_name))
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

   if (pnElements != NULL)
   {
      *pnElements = nFound;
   }

   return (nFound == count) ? S_OK : S_FALSE;
}

////////////////////////////////////////

tResult EnumFiles(const cFileSpec & spec, IEnumFiles * * ppEnumFiles)
{
   if (ppEnumFiles == NULL)
      return E_POINTER;

   cAutoIPtr<cEnumFilesPosix> pEnumFiles(new cEnumFilesPosix(spec));
   if (!pEnumFiles)
      return E_OUTOFMEMORY;

   *ppEnumFiles = static_cast<IEnumFiles*>(CTAddRef(pEnumFiles));
   return S_OK;
}

#endif

///////////////////////////////////////////////////////////////////////////////
