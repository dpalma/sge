////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "resourcestore.h"
#include "fileenum.h"
#include "filepath.h"
#include "filespec.h"
#include "readwriteapi.h"

#include "dbgalloc.h" // must be last header

////////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(ResourceStore);

#define LocalMsg(msg)            DebugMsgEx(ResourceStore,msg)
#define LocalMsg1(msg,a)         DebugMsgEx1(ResourceStore,msg,(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(ResourceStore,msg,(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(ResourceStore,msg,(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)   DebugMsgEx4(ResourceStore,msg,(a),(b),(c),(d))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(ResourceStore,(cond),msg)
#define LocalMsgIf1(cond,msg,a)        DebugMsgIfEx1(ResourceStore,(cond),msg,(a))
#define LocalMsgIf2(cond,msg,a,b)      DebugMsgIfEx2(ResourceStore,(cond),msg,(a),(b))
#define LocalMsgIf3(cond,msg,a,b,c)    DebugMsgIfEx3(ResourceStore,(cond),msg,(a),(b),(c))
#define LocalMsgIf4(cond,msg,a,b,c,d)  DebugMsgIfEx4(ResourceStore,(cond),msg,(a),(b),(c),(d))

////////////////////////////////////////////////////////////////////////////////

// REFERENCES
// "Game Developer Magazine", February 2005, "Inner Product" column

static const int kUnzMaxPath = 260;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceStore
//

////////////////////////////////////////

cResourceStore::~cResourceStore()
{
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cDirectoryResourceStore
//

////////////////////////////////////////

cDirectoryResourceStore::cDirectoryResourceStore(const tChar * pszDir)
 : m_dir((pszDir != NULL) ? pszDir : _T(""))
{
}

////////////////////////////////////////

cDirectoryResourceStore::~cDirectoryResourceStore()
{
}

////////////////////////////////////////

tResult cDirectoryResourceStore::FillCache(cResourceCache * pCache)
{
   if (pCache == NULL)
   {
      return E_POINTER;
   }

   if (m_dir.empty())
   {
      return E_FAIL;
   }

   cFileSpec wildcard(_T("*.*"));
   wildcard.SetPath(cFilePath(m_dir.c_str()));
   cAutoIPtr<IEnumFiles> pEnumFiles;
   if (EnumFiles(wildcard, &pEnumFiles) == S_OK)
   {
      cFileSpec files[10];
      uint attribs[10];
      ulong nFiles = 0;
      while (SUCCEEDED(pEnumFiles->Next(_countof(files), files, attribs, &nFiles)))
      {
         for (ulong i = 0; i < nFiles; i++)
         {
            if ((attribs[i] & kFA_Directory) == kFA_Directory)
            {
               LocalMsg1("Directory: %s\n", files[i].CStr());
            }
            else if ((attribs[i] & kFA_Hidden) == kFA_Hidden)
            {
               InfoMsg1("Skipping hidden file \"%s\"\n", files[i].CStr());
            }
            else
            {
               LocalMsg1("File: %s\n", files[i].CStr());
               pCache->AddCacheEntry(cResourceCacheEntryHeader(files[i].CStr(),
                  static_cast<cResourceStore *>(this)));
            }
         }
      }
   }

   return S_OK;
}

////////////////////////////////////////

tResult cDirectoryResourceStore::OpenEntry(const tChar * pszName, IReader * * ppReader)
{
   if (pszName == NULL || ppReader == NULL)
   {
      return E_POINTER;
   }

   cFileSpec file(pszName);
   file.SetPath(cFilePath(m_dir.c_str()));

   tResult result = E_FAIL;
   cAutoIPtr<IReader> pReader;
   if ((result = FileReaderCreate(file, &pReader)) != S_OK)
   {
      return result;
   }

   *ppReader = CTAddRef(pReader);
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cZipResourceStore
//

////////////////////////////////////////

cZipResourceStore::cZipResourceStore(const tChar * pszArchive)
 : m_archive((pszArchive != NULL) ? pszArchive : _T(""))
 , m_handle(NULL)
{
}

////////////////////////////////////////

cZipResourceStore::~cZipResourceStore()
{
   if (m_handle != NULL)
   {
      unzClose(m_handle);
      m_handle = NULL;
   }
}

////////////////////////////////////////

tResult cZipResourceStore::FillCache(cResourceCache * pCache)
{
   if (pCache == NULL)
   {
      return E_POINTER;
   }

   if (m_archive.empty())
   {
      return E_FAIL;
   }

   if (m_handle == NULL)
   {
#ifdef _UNICODE
      size_t size = wcstombs(NULL, m_archive.c_str(), 0);
      char * pszTemp = reinterpret_cast<char*>(alloca(size));
      wcstombs(pszTemp, m_archive.c_str(), size);
      m_handle = unzOpen(pszTemp);
#else
      m_handle = unzOpen(m_archive.c_str());
#endif
      if (m_handle == NULL)
      {
         ErrorMsg1("Failed to open zip archive \"%s\"\n", m_archive.c_str());
         return E_FAIL;
      }
   }

   do
   {
      unz_file_pos filePos;
      unz_file_info fileInfo;
      char szFile[kUnzMaxPath];
      if (unzGetFilePos(m_handle, &filePos) == UNZ_OK &&
         unzGetCurrentFileInfo(m_handle, &fileInfo, szFile, _countof(szFile), NULL, 0, NULL, 0) == UNZ_OK)
      {
         LocalMsg3("%s(%d): %s\n", m_archive.c_str(), filePos.num_of_file, szFile);
#ifdef _UNICODE
         size_t size = mbstowcs(NULL, szFile, 0);
         wchar_t * pszTemp = reinterpret_cast<wchar_t*>(alloca(size));
         mbstowcs(pszTemp, szFile, size);
         cFileSpec file(pszTemp);
#else
         cFileSpec file(szFile);
#endif
         m_dirCache[cStr(file.CStr())] = filePos;
         pCache->AddCacheEntry(cResourceCacheEntryHeader(file.CStr(),
            static_cast<cResourceStore *>(this)));
      }
   }
   while (unzGoToNextFile(m_handle) == UNZ_OK);

   return S_OK;
}

////////////////////////////////////////

tResult cZipResourceStore::OpenEntry(const tChar * pszName, IReader * * ppReader)
{
   if (pszName == NULL || ppReader == NULL)
   {
      return E_POINTER;
   }

   if (m_handle == NULL)
   {
      ErrorMsg("Zip archive not open\n");
      return E_FAIL;
   }

   tZipDirCache::iterator f = m_dirCache.find(pszName);
   if (f == m_dirCache.end())
   {
      // TODO: do a slow look-up
      return E_NOTIMPL;
   }

   if (unzGoToFilePos(m_handle, &(f->second)) != UNZ_OK)
   {
      return E_FAIL;
   }

   tResult result = E_FAIL;

   unz_file_info fileInfo;
   char szFile[kUnzMaxPath];
   if (unzGetCurrentFileInfo(m_handle, &fileInfo, szFile, _countof(szFile), NULL, 0, NULL, 0) == UNZ_OK)
   {
      byte * pBuffer = new byte[fileInfo.uncompressed_size];
      if (pBuffer == NULL)
      {
         return E_OUTOFMEMORY;
      }

      if (unzOpenCurrentFile(m_handle) == UNZ_OK)
      {
         cAutoIPtr<IReader> pReader;
         if (unzReadCurrentFile(m_handle, pBuffer, fileInfo.uncompressed_size) >= 0
            && MemReaderCreate(pBuffer, fileInfo.uncompressed_size, true, &pReader) == S_OK)
         {
            *ppReader = CTAddRef(pReader);
            result = S_OK;
         }

         unzCloseCurrentFile(m_handle);
      }
   }

   return result;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceCache
//

////////////////////////////////////////

cResourceCache::~cResourceCache()
{
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceCacheEntryHeader
//
////////////////////////////////////////

cResourceCacheEntryHeader::cResourceCacheEntryHeader(const tChar * pszName, cResourceStore * pStore)
 : m_name(pszName)
 , m_pStore(pStore)
{
}

////////////////////////////////////////

cResourceCacheEntryHeader::cResourceCacheEntryHeader(const cResourceCacheEntryHeader & other)
 : m_name(other.m_name)
 , m_pStore(other.m_pStore)
{
}

////////////////////////////////////////

cResourceCacheEntryHeader::~cResourceCacheEntryHeader()
{
}

////////////////////////////////////////

const cResourceCacheEntryHeader & cResourceCacheEntryHeader::operator =(const cResourceCacheEntryHeader & other)
{
   m_name = other.m_name;
   m_pStore = other.m_pStore;
   return *this;
}


////////////////////////////////////////////////////////////////////////////////
