////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "resourcestore.h"

#include "tech/fileenum.h"
#include "tech/filepath.h"
#include "tech/filespec.h"
#include "tech/readwriteapi.h"
#include "tech/techstring.h"

#define ZLIB_WINAPI
#include <unzip.h>

#include <map>

#include "tech/dbgalloc.h" // must be last header

using namespace std;

////////////////////////////////////////////////////////////////////////////////

LOG_EXTERN_CHANNEL(ResourceStore);

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

static const int kUnzMaxPath = 260;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUnzipIterator
//

class cUnzipIterator
{
public:
   cUnzipIterator(unzFile unzHandle);
   ~cUnzipIterator();

   bool Next(unz_file_pos * pFilePos, unz_file_info * pFileInfo, char * pszFile, size_t maxFile);

private:
   unzFile m_unzHandle;
   bool m_bAtEnd;
};

cUnzipIterator::cUnzipIterator(unzFile unzHandle)
 : m_unzHandle(unzHandle)
 , m_bAtEnd(false)
{
   if (m_unzHandle != NULL)
   {
      unzGoToFirstFile(m_unzHandle);
   }
}

cUnzipIterator::~cUnzipIterator()
{
}

bool cUnzipIterator::Next(unz_file_pos * pFilePos, unz_file_info * pFileInfo, char * pszFile, size_t maxFile)
{
   if (!m_bAtEnd)
   {
      if (unzGetFilePos(m_unzHandle, pFilePos) == UNZ_OK &&
         unzGetCurrentFileInfo(m_unzHandle, pFileInfo, pszFile, maxFile, NULL, 0, NULL, 0) == UNZ_OK)
      {
         m_bAtEnd = (unzGoToNextFile(m_unzHandle) == UNZ_END_OF_LIST_OF_FILE);
         return true;
      }
   }
   return false;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUnzipArchive
//

class cUnzipArchive
{
public:
   cUnzipArchive(unzFile unzHandle);
   ~cUnzipArchive();

   cUnzipIterator Iterate();

   tResult OpenMember(const unz_file_pos & filePos, IReader * * ppReader);

private:
   unzFile m_unzHandle;
};

////////////////////////////////////////

cUnzipArchive::cUnzipArchive(unzFile unzHandle)
 : m_unzHandle(unzHandle)
{
}

////////////////////////////////////////

cUnzipArchive::~cUnzipArchive()
{
   if (m_unzHandle != NULL)
   {
      unzClose(m_unzHandle);
      m_unzHandle = NULL;
   }
}

////////////////////////////////////////

cUnzipIterator cUnzipArchive::Iterate()
{
   return cUnzipIterator(m_unzHandle);
}

////////////////////////////////////////

tResult cUnzipArchive::OpenMember(const unz_file_pos & filePos, IReader * * ppReader)
{
   if (ppReader == NULL)
   {
      return E_POINTER;
   }

   Assert(m_unzHandle != NULL);

   if (unzGoToFilePos(m_unzHandle, const_cast<unz_file_pos*>(&filePos)) != UNZ_OK)
   {
      return E_FAIL;
   }

   tResult result = E_FAIL;

   unz_file_info fileInfo;
   char szFile[kUnzMaxPath];
   if (unzGetCurrentFileInfo(m_unzHandle, &fileInfo, szFile, _countof(szFile), NULL, 0, NULL, 0) == UNZ_OK)
   {
      if (unzOpenCurrentFile(m_unzHandle) == UNZ_OK)
      {
         byte * pBuffer = new byte[fileInfo.uncompressed_size];
         if (pBuffer == NULL)
         {
            result = E_OUTOFMEMORY;
         }
         else
         {
            cAutoIPtr<IReader> pReader;
            if (unzReadCurrentFile(m_unzHandle, pBuffer, fileInfo.uncompressed_size) >= 0
               && MemReaderCreate(pBuffer, fileInfo.uncompressed_size, true, &pReader) == S_OK)
            {
               *ppReader = CTAddRef(pReader);
               result = S_OK;
            }
            else
            {
               delete [] pBuffer;
               pBuffer = NULL;
            }
         }

         unzCloseCurrentFile(m_unzHandle);
      }
   }

   return result;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cZipResourceStore
//

class cZipResourceStore : public cComObject<IMPLEMENTS(IResourceStore)>
{
public:
   cZipResourceStore(unzFile unzHandle);
   virtual ~cZipResourceStore();

   virtual tResult CollectResourceNames(const tChar * pszMatch, vector<cStr> * pNames);
   virtual tResult OpenEntry(const tChar * pszName, IReader * * ppReader);

private:
   cUnzipArchive m_unzArchive;

   typedef map<cStr, unz_file_pos_s> tZipDirCache;
   tZipDirCache m_dirCache;
};

////////////////////////////////////////

cZipResourceStore::cZipResourceStore(unzFile unzHandle)
 : m_unzArchive(unzHandle)
{
}

////////////////////////////////////////

cZipResourceStore::~cZipResourceStore()
{
}

////////////////////////////////////////

tResult cZipResourceStore::CollectResourceNames(const tChar * pszMatch, vector<cStr> * pNames)
{
   cUnzipIterator iter(m_unzArchive.Iterate());

   unz_file_pos filePos;
   unz_file_info fileInfo;
   char szFile[kUnzMaxPath];

   while (iter.Next(&filePos, &fileInfo, szFile, _countof(szFile)))
   {
      LocalMsg2("[%d] %s\n", filePos.num_of_file, szFile);

#ifdef _UNICODE
      size_t size = mbstowcs(NULL, szFile, 0);
      wchar_t * pszTemp = reinterpret_cast<wchar_t*>(alloca(size));
      mbstowcs(pszTemp, szFile, size);
      cFileSpec file(pszTemp);
#else
      cFileSpec file(szFile);
#endif
      m_dirCache[cStr(file.CStr())] = filePos;

      if (pNames != NULL)
      {
         if (pszMatch != NULL)
         {
            if (WildCardMatch(pszMatch, file.CStr()))
            {
               pNames->push_back(file.CStr());
            }
         }
         else
         {
            pNames->push_back(file.CStr());
         }
      }
   }

   return S_OK;
}

////////////////////////////////////////

tResult cZipResourceStore::OpenEntry(const tChar * pszName, IReader * * ppReader)
{
   if (pszName == NULL || ppReader == NULL)
   {
      return E_POINTER;
   }

   // HACK
   if (m_dirCache.empty())
   {
      CollectResourceNames(NULL, NULL);
   }

   tZipDirCache::iterator f = m_dirCache.find(pszName);
   if (f == m_dirCache.end())
   {
      // TODO: do a slow look-up
      return E_NOTIMPL;
   }

   return m_unzArchive.OpenMember(f->second, ppReader);
}

////////////////////////////////////////

tResult ResourceStoreCreateZip(const tChar * pszArchive, IResourceStore * * ppStore)
{
   if (pszArchive == NULL || ppStore == NULL)
   {
      return E_POINTER;
   }

#ifdef _UNICODE
   size_t size = wcstombs(NULL, pszArchive, 0);
   char * pszTemp = reinterpret_cast<char*>(alloca(size));
   wcstombs(pszTemp, pszArchive, size);
   unzFile unzHandle = unzOpen(pszTemp);
#else
   unzFile unzHandle = unzOpen(pszArchive);
#endif

   if (unzHandle == NULL)
   {
      return S_FALSE;
   }

   IResourceStore * pStore = static_cast<IResourceStore *>(new cZipResourceStore(unzHandle));
   if (pStore == NULL)
   {
      unzClose(unzHandle);
      return E_OUTOFMEMORY;
   }

   *ppStore = pStore;
   return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
