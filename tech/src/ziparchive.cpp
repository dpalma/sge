///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ziparchive.h"

#include "readwriteapi.h"
#include "filespec.h"

#define ZLIB_WINAPI
#include <unzip.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cZipArchive
//

///////////////////////////////////////

cZipArchive::cZipArchive()
 : m_unzFile(NULL)
{
}

///////////////////////////////////////

cZipArchive::~cZipArchive()
{
   Close();
}

///////////////////////////////////////

tResult cZipArchive::Open(const cFileSpec & zipFileSpec)
{
   if (m_unzFile != NULL)
   {
      return S_FALSE;
   }

   m_unzFile = unzOpen(zipFileSpec.GetName());

   if (m_unzFile != NULL)
   {
      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cZipArchive::Close()
{
   if (m_unzFile != NULL)
   {
      if (unzClose(m_unzFile) == UNZ_OK)
      {
         m_unzFile = NULL;
         return S_OK;
      }
      else
      {
         return E_FAIL;
      }
   }
   return S_FALSE;
}

///////////////////////////////////////

tResult cZipArchive::IsOpen() const
{
   return (m_unzFile != NULL) ? S_OK : S_FALSE;
}

///////////////////////////////////////

tResult cZipArchive::OpenMember(const char * pszFile, IReader * * ppReader)
{
   if (pszFile == NULL || ppReader == NULL)
   {
      return E_POINTER;
   }

   tResult result = E_FAIL;

   if (m_unzFile != NULL)
   {
      if (unzLocateFile(m_unzFile, pszFile, 0) == UNZ_OK)
      {
         unz_file_info fileInfo;
         char szFileName[kMaxPath];
         char szExtra[256];
         char szComment[256];

         if (unzGetCurrentFileInfo(m_unzFile, &fileInfo,
                                   szFileName, sizeof(szFileName),
                                   szExtra, sizeof(szExtra),
                                   szComment, sizeof(szComment)) == UNZ_OK)
         {
            byte * pMem = new byte[fileInfo.uncompressed_size];
            if (pMem != NULL)
            {
               if (unzOpenCurrentFile(m_unzFile) == UNZ_OK)
               {
                  if (unzReadCurrentFile(m_unzFile, pMem, fileInfo.uncompressed_size) >= 0)
                  {
                     result = ReaderCreateMem(pMem, fileInfo.uncompressed_size, true, ppReader);
                  }

                  unzCloseCurrentFile(m_unzFile);
               }

               if (FAILED(result))
               {
                  delete [] pMem;
               }
            }
         }
      }
   }

   return result;
}

///////////////////////////////////////////////////////////////////////////////
