///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "fileiter.h"
#include "filespec.h"
#include "filepath.h"
#include "techstring.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#endif

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

   void Begin(const cFileSpec & spec);
   bool Next(cFileSpec * pFileSpec, uint * pAttribs);
   void End();

private:
   const cFilePath & GetPath() const { return m_path; }
   const tChar * GetSpec() const { return m_spec.c_str(); }

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

void cFileIterWin32::Begin(const cFileSpec & spec)
{
   Assert(m_hFinder == NULL);
   m_spec = spec;
   m_path = spec.GetPath();
}

///////////////////////////////////////

bool cFileIterWin32::Next(cFileSpec * pFileSpec, uint * pAttribs)
{
   bool bFound = false;

   do
   {
      if (m_hFinder == NULL)
      {
         m_hFinder = FindFirstFile(GetSpec(), &m_findData);
         if (m_hFinder != INVALID_HANDLE_VALUE)
         {
            bFound = true;
         }
      }
      else
      {
         if (FindNextFile(m_hFinder, &m_findData))
         {
            bFound = true;
         }
      }

      if (bFound)
      {
         DebugMsgEx1(FileIter, "Found file %s\n", m_findData.cFileName);

         if (m_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
         {
            if (_tcscmp(m_findData.cFileName, _T(".")) == 0 ||
                _tcscmp(m_findData.cFileName, _T("..")) == 0)
            {
               continue;
            }
         }

         if (pFileSpec != NULL)
         {
            *pFileSpec = cFileSpec(m_findData.cFileName);
            pFileSpec->SetPath(GetPath());
         }

         if (pAttribs != NULL)
         {
            *pAttribs = m_findData.dwFileAttributes;
         }

         return true;
      }
   }
   while (bFound);

   return false;
}

///////////////////////////////////////

void cFileIterWin32::End()
{
   if (m_hFinder != NULL && m_hFinder != INVALID_HANDLE_VALUE)
   {
      FindClose(m_hFinder);
      m_hFinder = NULL;
   }

   m_spec.erase();
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
