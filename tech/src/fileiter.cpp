///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#endif

#include "filespec.h"
#include "filepath.h"
#include "fileiter.h"
#include "str.h"

#include "dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(FileIter);

///////////////////////////////////////////////////////////////////////////////

cFileIter::~cFileIter()
{
}

#ifdef _WIN32

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFileIterWin32
//

AssertOnce(kFA_ReadOnly == FILE_ATTRIBUTE_READONLY);
AssertOnce(kFA_Hidden == FILE_ATTRIBUTE_HIDDEN);
AssertOnce(kFA_System == FILE_ATTRIBUTE_SYSTEM);
AssertOnce(kFA_Directory == FILE_ATTRIBUTE_DIRECTORY);
AssertOnce(kFA_Archive == FILE_ATTRIBUTE_ARCHIVE);
AssertOnce(kFA_Normal == FILE_ATTRIBUTE_NORMAL);
AssertOnce(kFA_Temporary == FILE_ATTRIBUTE_TEMPORARY);
AssertOnce(kFA_Compressed == FILE_ATTRIBUTE_COMPRESSED);

#define kDefaultAttribs (kFA_Directory | kFA_Hidden | kFA_System)

class cFileIterWin32 : public cFileIter
{
public:
   cFileIterWin32();

   void IterBegin(const char * pszSpec);
   BOOL IterNext(char * pszResult, uint maxResult, uint * pAttribs);
   void IterEnd();

private:
   const cFilePath & GetPath() const { return m_path; }
   const char * GetSpec() const { return m_spec.c_str(); }

   cFilePath m_path;
   cStr m_spec;
   WIN32_FIND_DATA m_findData;
   HANDLE m_hFinder;
};

///////////////////////////////////////

cFileIterWin32::cFileIterWin32()
 : m_hFinder(NULL)
{
}

///////////////////////////////////////

void cFileIterWin32::IterBegin(const char * pszSpec)
{
   Assert(pszSpec != NULL);
   Assert(m_hFinder == NULL);

   m_spec = pszSpec;

   const char * p = strrchr(pszSpec, '\\');
   if (p != NULL)
   {
      int len = p - pszSpec;
      if (len < MAX_PATH)
      {
         char szPath[MAX_PATH];
         strncpy(szPath, pszSpec, len);
         szPath[len] = 0;
         m_path = szPath;
      }
   }
}

///////////////////////////////////////

BOOL cFileIterWin32::IterNext(char * pszResult, uint maxResult, uint * pAttribs)
{
   BOOL bFound = FALSE;

   do
   {
      if (m_hFinder == NULL)
      {
         m_hFinder = FindFirstFile(GetSpec(), &m_findData);
         if (m_hFinder != INVALID_HANDLE_VALUE)
            bFound = TRUE;
      }
      else
      {
         bFound = FindNextFile(m_hFinder, &m_findData);
      }

      if (bFound)
      {
         DebugMsgEx1(FileIter, "Found file %s\n", m_findData.cFileName);

         if (m_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
         {
            if (strcmp(m_findData.cFileName, ".") == 0 ||
                strcmp(m_findData.cFileName, "..") == 0)
               continue;
         }

         cFileSpec foundFile(m_findData.cFileName);
         foundFile.SetPath(GetPath());

         strncpy(pszResult, foundFile.GetName(), maxResult);
         pszResult[maxResult - 1] = 0;
         if (pAttribs != NULL)
            *pAttribs = m_findData.dwFileAttributes;
         return TRUE;
      }
   }
   while (bFound);

   return FALSE;
}

///////////////////////////////////////

void cFileIterWin32::IterEnd()
{
   if (m_hFinder != NULL && m_hFinder != INVALID_HANDLE_VALUE)
   {
      FindClose(m_hFinder);
      m_hFinder = NULL;
   }

   m_spec = "";
}

///////////////////////////////////////////////////////////////////////////////

cFileIter * FileIterCreate()
{
   return new cFileIterWin32;
}

#else

///////////////////////////////////////////////////////////////////////////////

cFileIter * FileIterCreate()
{
   Assert(!"Not supported"); // @TODO
   return NULL;
}

#endif

///////////////////////////////////////////////////////////////////////////////
