////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "resourceapi.h"
#include "filepath.h"
#include "filespec.h"
#include "fileiter.h"
#include "readwriteapi.h"
#include "globalobj.h"

#include <cstdio>
#include <vector>
#include <string>
#include <algorithm>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#endif

#define ZLIB_WINAPI
#include <unzip.h>

#include "dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(ResourceManager);

#define LocalMsg(msg)            DebugMsgEx(ResourceManager,(msg))
#define LocalMsg1(msg,a)         DebugMsgEx1(ResourceManager,(msg),(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(ResourceManager,(msg),(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(ResourceManager,(msg),(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)   DebugMsgEx4(ResourceManager,(msg),(a),(b),(c),(d))

const uint kNoIndex = ~0;
const ulong kNoIndexL = ~0;

// REFERENCES
// "Game Developer Magazine", February 2005, "Inner Product" column

////////////////////////////////////////////////////////////////////////////////

typedef std::vector<std::string> tStrings;
static size_t ListDirs(const cFilePath & path, tStrings * pDirs)
{
   Assert(pDirs != NULL);
   if (pDirs == NULL)
      return ~0;

   pDirs->clear();

#ifdef _WIN32
   cFileSpec wildcard("*");
   wildcard.SetPath(path);

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
   DIR * dir = opendir(path.GetPath());
   if (dir)
   {
      struct dirent * ent = readdir(dir);
      while (ent)
      {
         if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, ".."))
         {
            cFileSpec file(ent->d_name);
            file.SetPath(path);

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

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceManager
//

class cResourceManager : public cGlobalObject<IMPLEMENTS(IResourceManager)>
{
public:
   cResourceManager();
   virtual ~cResourceManager();

   virtual tResult Init();
   virtual tResult Term();

   // IResourceManager methods
   virtual tResult AddDirectory(const char * pszDir);
   virtual tResult AddDirectoryTreeFlattened(const char * pszDir);
   virtual tResult AddArchive(const char * pszArchive);
   virtual tResult Load(const tResKey & key, void * param, void * * ppData);
   virtual tResult Unload(const tResKey & key);
   virtual tResult Lock(const tResKey & key);
   virtual tResult Unlock(const tResKey & key);
   virtual tResult RegisterFormat(eResourceClass rc,
                                  const char * pszExtension,
                                  tResourceLoad pfnLoad,
                                  tResourcePostload pfnPostload,
                                  tResourceUnload pfnUnload);

private:
   struct sResource;
   sResource * InternalFindResource(const tResKey & key);
   tResult DoLoadFromFile(const cFileSpec & file, eResourceClass rc, void * param, sResource * pResource);
   tResult DoLoadFromArchive(const tResKey & key, void * param, sResource * pResource);
   tResult DoLoadFromReader(IReader * pReader, eResourceClass rc, ulong dataSize, void * param, sResource * pResource);

   uint GetExtensionId(const char * pszExtension);
   uint GetDirectoryId(const char * pszDir);
   uint GetArchiveId(const char * pszArchive);

   std::vector<cStr> m_extensions;
   std::vector<cFilePath> m_dirs;

   struct sArchiveInfo
   {
      cStr archive;
      unzFile handle;
   };
   typedef std::vector<sArchiveInfo> tArchives;
   tArchives m_archives;

   struct sFormat
   {
      eResourceClass rc;
      uint extensionId;
      tResourceLoad pfnLoad;
      tResourcePostload pfnPostload;
      tResourceUnload pfnUnload;
   };
   std::vector<sFormat> m_formats;

   struct sResource
   {
      sResource()
      {
         extensionId = kNoIndex;
         pFormat = NULL;
         dirId = archiveId = kNoIndex;
         offset = index = kNoIndexL;
         lockCount = 0;
         pData = NULL;
         dataSize = 0;
      }

      cStr name;
      uint extensionId;
      sFormat * pFormat;
      uint dirId;
      uint archiveId;
      ulong offset;
      ulong index;
      ulong lockCount;
      void * pData;
      ulong dataSize;
   };
   typedef std::vector<sResource> tCache;
   tCache m_cache;
};

////////////////////////////////////////

cResourceManager::cResourceManager()
 : cGlobalObject<IMPLEMENTS(IResourceManager)>(kResourceManagerName)
{
}

////////////////////////////////////////

cResourceManager::~cResourceManager()
{
}

////////////////////////////////////////

tResult cResourceManager::Init()
{
   return S_OK;
}

////////////////////////////////////////

tResult cResourceManager::Term()
{
   tCache::iterator cacheIter = m_cache.begin();
   tCache::iterator cacheEnd = m_cache.end();
   for (; cacheIter != cacheEnd; cacheIter++)
   {
      if (cacheIter->pFormat != NULL && cacheIter->pFormat->pfnUnload != NULL)
      {
         (*cacheIter->pFormat->pfnUnload)(cacheIter->pData);
         cacheIter->pData = NULL;
         cacheIter->dataSize = 0;
      }
   }
   m_cache.clear();

   tArchives::iterator arIter = m_archives.begin();
   tArchives::iterator arEnd = m_archives.end();
   for (; arIter != arEnd; arIter++)
   {
      if (arIter->handle != NULL)
      {
         unzClose(arIter->handle);
         arIter->handle = NULL;
      }
   }
   m_archives.clear();

   return S_OK;
}

////////////////////////////////////////

tResult cResourceManager::AddDirectory(const char * pszDir)
{
   if (pszDir == NULL)
   {
      return E_POINTER;
   }

   cFileIter * pFileIter = FileIterCreate();
   if (pFileIter == NULL)
   {
      return E_OUTOFMEMORY;
   }

   cFileSpec spec("*.*");
   spec.SetPath(cFilePath(pszDir));

   char szFile[kMaxPath];
   uint attribs;

   pFileIter->IterBegin(spec.GetName());
   while (pFileIter->IterNext(szFile, _countof(szFile), &attribs))
   {
      if ((attribs & kFA_Directory) == kFA_Directory)
      {
         LocalMsg1("Dir: %s\n", szFile);
      }
      else
      {
         LocalMsg1("File: %s\n", szFile);
         cFileSpec file(szFile);
         sResource res;
         Verify(file.GetFileNameNoExt(&res.name));
         const char * pszExt = file.GetFileExt();
         if (pszExt != NULL && strlen(pszExt) > 0)
         {
            res.extensionId = GetExtensionId(pszExt);
         }
         res.dirId = GetDirectoryId(pszDir);
         m_cache.push_back(res);
      }
   }
   pFileIter->IterEnd();

   delete pFileIter;

   return S_OK;
}

////////////////////////////////////////

tResult cResourceManager::AddDirectoryTreeFlattened(const char * pszDir)
{
   if (pszDir == NULL)
   {
      return E_POINTER;
   }

   cFilePath root(pszDir);
   root.MakeFullPath();
   if (AddDirectory(root.GetPath()) != S_OK)
   {
      return E_FAIL;
   }

   tStrings dirs;
   if (ListDirs(root, &dirs) > 0)
   {
      tStrings::iterator iter;
      for (iter = dirs.begin(); iter != dirs.end(); iter++)
      {
         cFilePath p(root);
         p.AddRelative(iter->c_str());
         if (AddDirectoryTreeFlattened(p.GetPath()) != S_OK)
         {
            return E_FAIL;
         }
      }
   }

   return S_OK;
}

////////////////////////////////////////

tResult cResourceManager::AddArchive(const char * pszArchive)
{
   uint archiveId = GetArchiveId(pszArchive);

   unzFile uf = NULL;
   if (m_archives[archiveId].handle != NULL)
   {
      uf = m_archives[archiveId].handle;
   }
   else
   {
      uf = m_archives[archiveId].handle = unzOpen(pszArchive);
      if (uf == NULL)
      {
         return E_FAIL;
      }
   }

   do
   {
      unz_file_pos filePos;
      unz_file_info fileInfo;
      char szFile[kMaxPath];
      if (unzGetFilePos(uf, &filePos) == UNZ_OK &&
         unzGetCurrentFileInfo(uf, &fileInfo, szFile, _countof(szFile), NULL, 0, NULL, 0) == UNZ_OK)
      {
         LocalMsg3("%s(%d): %s\n", pszArchive, filePos.num_of_file, szFile);
         cFileSpec file(szFile);
         sResource res;
         Verify(file.GetFileNameNoExt(&res.name));
         const char * pszExt = file.GetFileExt();
         if (pszExt != NULL && strlen(pszExt) > 0)
         {
            res.extensionId = GetExtensionId(pszExt);
         }
         res.archiveId = archiveId;
         res.offset = filePos.pos_in_zip_directory;
         res.index = filePos.num_of_file;
         m_cache.push_back(res);
      }
   }
   while (unzGoToNextFile(uf) == UNZ_OK);

   return S_OK;
}

////////////////////////////////////////

tResult cResourceManager::Load(const tResKey & key, void * param, void * * ppData)
{
   if (ppData == NULL)
   {
      return E_POINTER;
   }

   cStr name;
   cFileSpec(key.GetName()).GetFileNameNoExt(&name);

   sResource * pRes = InternalFindResource(key);
   if (pRes != NULL)
   {
      if (pRes->pData == NULL)
      {
         if (pRes->dirId != kNoIndex)
         {
            cFileSpec file(key.GetName());
            file.SetPath(m_dirs[pRes->dirId]);
            if (DoLoadFromFile(file, key.GetClass(), param, pRes) != S_OK)
            {
               return E_FAIL;
            }
         }
         else if (pRes->archiveId != kNoIndex)
         {
            if (DoLoadFromArchive(key, param, pRes) != S_OK)
            {
               return E_FAIL;
            }
         }
      }

      if (pRes->pData != NULL)
      {
         *ppData = pRes->pData;
         return S_OK;
      }
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cResourceManager::Unload(const tResKey & key)
{
   sResource * pRes = InternalFindResource(key);
   if (pRes != NULL)
   {
      if (pRes->lockCount > 0)
      {
         // Locked. Cannot unload.
         return S_FALSE;
      }
      else
      {
         if (pRes->pFormat != NULL && pRes->pFormat->pfnUnload != NULL)
         {
            (*pRes->pFormat->pfnUnload)(pRes->pData);
            pRes->pData = NULL;
            pRes->dataSize = 0;
            return S_OK;
         }
      }
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cResourceManager::Lock(const tResKey & key)
{
   sResource * pRes = InternalFindResource(key);
   if (pRes == NULL)
   {
      return E_FAIL;
   }

   if (pRes->pData == NULL)
   {
      return E_FAIL;
   }

   pRes->lockCount += 1;

   return S_OK;
}

////////////////////////////////////////

tResult cResourceManager::Unlock(const tResKey & key)
{
   sResource * pRes = InternalFindResource(key);
   if (pRes == NULL)
   {
      return E_FAIL;
   }

   if (pRes->pData == NULL)
   {
      return E_FAIL;
   }

   if (pRes->lockCount == 0)
   {
      WarnMsg("Too many resource unlock calls\n");
      return S_FALSE;
   }

   pRes->lockCount -= 1;

   return S_OK;
}

////////////////////////////////////////

AssertOnce(kNUMRESOURCECLASSES == 5); // If this fails, GetResourceClassName may need to be updated
static const char * GetResourceClassName(eResourceClass rc)
{
   switch (rc)
   {
   case kRC_Image: return "Image";
   case kRC_Texture: return "Texture";
   case kRC_Mesh: return "Mesh";
   case kRC_Text: return "Text";
   case kRC_Font: return "Font";
   default: return "Unknown";
   }
}

////////////////////////////////////////

tResult cResourceManager::RegisterFormat(eResourceClass rc,
                                         const char * pszExtension,
                                         tResourceLoad pfnLoad,
                                         tResourcePostload pfnPostload,
                                         tResourceUnload pfnUnload)
{
   if (pfnLoad == NULL)
   {
      // Must have at least a load function
      WarnMsg1("No load function specified when registering resource %s\n",
            pszExtension != NULL ? pszExtension : "<NONE>");
      return E_POINTER;
   }

   if (pfnUnload == NULL)
   {
      WarnMsg1("No unload function specified for resource %s\n",
            pszExtension != NULL ? pszExtension : "<NONE>");
   }

   uint extensionId = kNoIndex;
   if (pszExtension != NULL)
   {
      extensionId = GetExtensionId(pszExtension);
   }

   std::vector<sFormat>::iterator iter = m_formats.begin();
   std::vector<sFormat>::iterator end = m_formats.end();
   for (; iter != end; iter++)
   {
      if (iter->extensionId == extensionId)
      {
         WarnMsg1("Resource format with file extension %s already registered\n",
            pszExtension != NULL ? pszExtension : "<NONE>");
         return E_FAIL;
      }
   }

   sFormat format;
   format.rc = rc;
   format.extensionId = extensionId;
   format.pfnLoad = pfnLoad;
   format.pfnPostload = pfnPostload;
   format.pfnUnload = pfnUnload;
   m_formats.push_back(format);

   return S_OK;
}

////////////////////////////////////////

cResourceManager::sResource * cResourceManager::InternalFindResource(const tResKey & key)
{
   uint extensionId = kNoIndex;
   static const char kExtSep = '.';
   const char * pszExt = strrchr(key.GetName(), kExtSep);
   if (pszExt != NULL)
   {
      extensionId = GetExtensionId(++pszExt);
   }

   if (extensionId == kNoIndex)
   {
      // TODO: look for possible extensions for the resource class
      return NULL;
   }

   cStr name;
   cFileSpec(key.GetName()).GetFileNameNoExt(&name);

   tCache::iterator cacheIter =  m_cache.begin();
   tCache::iterator cacheEnd =  m_cache.end();
   for (; cacheIter != cacheEnd; cacheIter++)
   {
      if (cacheIter->extensionId == extensionId && cacheIter->name.compare(name) == 0)
      {
         return &m_cache[cacheIter - m_cache.begin()];
      }
   }

   return NULL;
}

////////////////////////////////////////

tResult cResourceManager::DoLoadFromFile(const cFileSpec & file, eResourceClass rc,
                                         void * param, sResource * pResource)
{
   if (pResource == NULL)
   {
      return E_POINTER;
   }

   Assert(pResource->pData == NULL);
   Assert(pResource->dataSize == 0);

   cAutoIPtr<IReader> pReader(FileCreateReader(file));
   if (!pReader)
   {
      return E_OUTOFMEMORY;
   }

   pReader->Seek(0, kSO_End);
   ulong dataSize;
   if (pReader->Tell(&dataSize) != S_OK)
   {
      return E_FAIL;
   }
   pReader->Seek(0, kSO_Set);

   return DoLoadFromReader(pReader, rc, dataSize, param, pResource);
}

////////////////////////////////////////

tResult cResourceManager::DoLoadFromArchive(const tResKey & key, void * param, sResource * pResource)
{
   if (pResource == NULL)
   {
      return E_POINTER;
   }

   Assert(pResource->pData == NULL);
   Assert(pResource->dataSize == 0);

   unzFile uf = m_archives[pResource->archiveId].handle;
   if (uf == NULL)
   {
      // Could actually reload the zip file on the fly using unzOpen(m_archives[...].archive)
      return E_FAIL;
   }

   unz_file_pos filePos;
   filePos.pos_in_zip_directory = pResource->offset;
   filePos.num_of_file = pResource->index;
   if (unzGoToFilePos(uf, &filePos) != UNZ_OK)
   {
      return E_FAIL;
   }

   tResult result = E_FAIL;

   unz_file_info fileInfo;
   char szFile[kMaxPath];
   if (unzGetCurrentFileInfo(uf, &fileInfo, szFile, _countof(szFile), NULL, 0, NULL, 0) == UNZ_OK)
   {
      byte * pBuffer = new byte[fileInfo.uncompressed_size];
      if (pBuffer == NULL)
      {
         return E_OUTOFMEMORY;
      }

      if (unzOpenCurrentFile(uf) == UNZ_OK)
      {
         cAutoIPtr<IReader> pReader;
         if (unzReadCurrentFile(uf, pBuffer, fileInfo.uncompressed_size) >= 0
            && ReaderCreateMem(pBuffer, fileInfo.uncompressed_size, false, &pReader) == S_OK)
         {
            result = DoLoadFromReader(pReader, key.GetClass(), fileInfo.uncompressed_size, param, pResource);
         }

         unzCloseCurrentFile(uf);
      }

      delete [] pBuffer;
   }

   return result;
}

////////////////////////////////////////

tResult cResourceManager::DoLoadFromReader(IReader * pReader, eResourceClass rc, ulong dataSize,
                                           void * param, sResource * pResource)
{
   if (pReader == NULL || pResource == NULL)
   {
      return E_POINTER;
   }

   std::vector<sFormat>::const_iterator fIter = m_formats.begin();
   std::vector<sFormat>::const_iterator fEnd = m_formats.end();
   for (; fIter != fEnd; fIter++)
   {
      if (fIter->extensionId == pResource->extensionId && fIter->rc == rc)
      {
         if (fIter->pfnLoad != NULL)
         {
            void * pData = (*fIter->pfnLoad)(pReader);
            if (pData != NULL)
            {
               if (fIter->pfnPostload != NULL)
               {
                  void * pNewData = (*fIter->pfnPostload)(pData, dataSize, param);
                  if (pNewData == NULL)
                  {
                     // Postload must succeed in order to cache anything about
                     // the resource
                     return E_FAIL;
                  }

                  // Assume the postload function cleaned up pData
                  // (or just passed it through to pNewData)
                  pData = pNewData;
               }

               // Cache the resource data
               pResource->pFormat = &m_formats[fIter - m_formats.begin()];
               pResource->pData = pData;
               pResource->dataSize = dataSize;
               return S_OK;
            }
         }
      }
   }

   return E_FAIL;
}

////////////////////////////////////////

uint cResourceManager::GetExtensionId(const char * pszExtension)
{
   Assert(pszExtension != NULL);

   std::vector<cStr>::iterator f = std::find(m_extensions.begin(), m_extensions.end(), pszExtension);
   if (f == m_extensions.end())
   {
      m_extensions.push_back(pszExtension);
      uint index = m_extensions.size() - 1;
      LocalMsg2("File extension %s has id %d\n", pszExtension, index);
      return index;
   }

   return f - m_extensions.begin();
}

////////////////////////////////////////

uint cResourceManager::GetDirectoryId(const char * pszDir)
{
   Assert(pszDir != NULL);

   cFilePath dir(pszDir);

   std::vector<cFilePath>::iterator f = std::find(m_dirs.begin(), m_dirs.end(), dir);
   if (f == m_dirs.end())
   {
      m_dirs.push_back(dir);
      return m_dirs.size() - 1;
   }

   return f - m_dirs.begin();
}

////////////////////////////////////////

uint cResourceManager::GetArchiveId(const char * pszArchive)
{
   Assert(pszArchive != NULL);

   tArchives::iterator iter = m_archives.begin();
   tArchives::iterator end = m_archives.end();
   for (uint index = 0; iter != end; iter++)
   {
      if (stricmp(pszArchive, iter->archive.c_str()) == 0)
      {
         return index;
      }
   }

   sArchiveInfo archiveInfo;
   archiveInfo.archive = pszArchive;
   archiveInfo.handle = NULL;
   m_archives.push_back(archiveInfo);
   return m_archives.size() - 1;
}

////////////////////////////////////////

void ResourceManagerCreate()
{
   cAutoIPtr<IResourceManager>(new cResourceManager);
}

////////////////////////////////////////////////////////////////////////////////
