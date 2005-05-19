////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "resourcemanager.h"
#include "filespec.h"
#include "fileiter.h"
#include "readwriteapi.h"

#include <cstdio>
#include <vector>
#include <algorithm>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#endif

#include "dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(ResourceManager);

#define LocalMsg(msg)            DebugMsgEx(ResourceManager,(msg))
#define LocalMsg1(msg,a)         DebugMsgEx1(ResourceManager,(msg),(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(ResourceManager,(msg),(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(ResourceManager,(msg),(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)   DebugMsgEx4(ResourceManager,(msg),(a),(b),(c),(d))

static const int kUnzMaxPath = 260;

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
   HANDLE hFinder = FindFirstFile(wildcard.c_str(), &findData);
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

static bool SameType(tResourceType lhs, tResourceType rhs)
{
   if (lhs != NULL && rhs != NULL)
   {
      uint16 lhsHiWord = (uint16)(((ulong)lhs >> 16) & 0xFFFF);
      uint16 rhsHiWord = (uint16)(((ulong)rhs >> 16) & 0xFFFF);
      if (lhsHiWord == 0 && rhsHiWord == 0)
      {
         return lhs == rhs;
      }
      return strcmp(lhs, rhs) == 0;
   }
   else if (lhs == NULL && rhs == NULL)
   {
      return true;
   }
   else
   {
      return false;
   }
}

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceManager
//

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
   tResources::iterator resIter = m_resources.begin();
   tResources::iterator resEnd = m_resources.end();
   for (; resIter != resEnd; resIter++)
   {
      if (resIter->pData != NULL)
      {
         if (resIter->pFormat != NULL && resIter->pFormat->pfnUnload != NULL)
         {
            (*resIter->pFormat->pfnUnload)(resIter->pData);
            resIter->pData = NULL;
            resIter->dataSize = 0;
         }
      }
   }
   m_resources.clear();

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

   cFileSpec file;
   uint attribs;

   pFileIter->Begin(spec);
   while (pFileIter->Next(&file, &attribs))
   {
      if ((attribs & kFA_Directory) == kFA_Directory)
      {
         LocalMsg1("Dir: %s\n", file.c_str());
      }
      else
      {
         LocalMsg1("File: %s\n", file.c_str());
         sResource res;
         Verify(file.GetFileNameNoExt(&res.name));
         const char * pszExt = file.GetFileExt();
         if (pszExt != NULL && strlen(pszExt) > 0)
         {
            res.extensionId = GetExtensionId(pszExt);
         }
         res.dirId = GetDirectoryId(pszDir);
         m_resources.push_back(res);
      }
   }
   pFileIter->End();

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
   if (AddDirectory(root.c_str()) != S_OK)
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
         if (AddDirectoryTreeFlattened(p.c_str()) != S_OK)
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
      char szFile[kUnzMaxPath];
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
         m_resources.push_back(res);
      }
   }
   while (unzGoToNextFile(uf) == UNZ_OK);

   return S_OK;
}

////////////////////////////////////////

tResult cResourceManager::LoadUncached(const char * pszName, tResourceType type,
                                       void * param, void * * ppData, ulong * pDataSize)
{
   if (pszName == NULL || ppData == NULL)
   {
      return E_POINTER;
   }

   if (!type)
   {
      return E_INVALIDARG;
   }

   tResult result = E_FAIL;
   sFormat * pFormat = NULL;
   if (DeduceFormats(pszName, type, &pFormat, 1) == 1)
   {
      const sResource * pRes = FindResourceWithFormat(pszName, type, pFormat);

      // If no resource and format specifies a dependent type then it
      // may not have been loaded in AddDirectory or AddArchive. Do it now.
      if (pRes == NULL && pFormat->typeDepend)
      {
         sResource res;
         cFileSpec(pszName).GetFileNameNoExt(&res.name);
         res.extensionId = GetExtensionIdForName(pszName);
         res.pFormat = pFormat;
         m_resources.push_back(res);
         pRes = &m_resources[m_resources.size() - 1];
      }

      Assert(pRes != NULL);

      ulong dataSize = 0;
      void * pData = NULL;

      if (pFormat->typeDepend)
      {
         if (Load(pszName, pFormat->typeDepend, param, &pData) == S_OK)
         {
            pData = (*pFormat->pfnPostload)(pData, 0, param);
            Assert(pRes->pFormat == pFormat); // should have been set above
            result = S_OK;
         }
      }
      else
      {
         if (pRes->dirId != kNoIndex)
         {
            cFileSpec file(pszName);
            file.SetPath(m_dirs[pRes->dirId]);
            result = DoLoadFromFile(file, pFormat, param, &dataSize, &pData);
         }
         else if (pRes->archiveId != kNoIndex)
         {
            result = DoLoadFromArchive(pRes->archiveId, pRes->offset, pRes->index, pFormat, param, &dataSize, &pData);
         }
      }

      if (result == S_OK)
      {
         *ppData = pData;
         if (pDataSize != NULL)
         {
            *pDataSize = dataSize;
         }
      }
   }

   return result;
}

////////////////////////////////////////

tResult cResourceManager::Load(const char * pszName, tResourceType type,
                               void * param, void * * ppData)
{
   if (pszName == NULL || ppData == NULL)
   {
      return E_POINTER;
   }

   if (!type)
   {
      return E_INVALIDARG;
   }

   sFormat * pFormat = NULL;
   if (DeduceFormats(pszName, type, &pFormat, 1) == 1)
   {
      sResource * pRes = FindResourceWithFormat(pszName, type, pFormat);

      // If no resource and format specifies a dependent type then it
      // may not have been loaded in AddDirectory or AddArchive. Do it now.
      if (pRes == NULL && pFormat->typeDepend)
      {
         sResource res;
         cFileSpec(pszName).GetFileNameNoExt(&res.name);
         res.extensionId = GetExtensionIdForName(pszName);
         res.pFormat = pFormat;
         m_resources.push_back(res);
         pRes = &m_resources[m_resources.size() - 1];
      }

      if (pRes != NULL)
      {
         ulong dataSize = 0;
         void * pData = NULL;

         if (pFormat->typeDepend)
         {
            if (pRes->pData == NULL)
            {
               if (Load(pszName, pFormat->typeDepend, param, &pData) == S_OK)
               {
                  pRes->pData = (*pFormat->pfnPostload)(pData, 0, param);
                  Assert(pRes->pFormat == pFormat); // should have been set above
               }
            }
         }
         else
         {
            if (pRes->pData == NULL)
            {
               tResult result = E_FAIL;

               if (pRes->dirId != kNoIndex)
               {
                  cFileSpec file(pszName);
                  file.SetPath(m_dirs[pRes->dirId]);
                  result = DoLoadFromFile(file, pFormat, param, &dataSize, &pData);
               }
               else if (pRes->archiveId != kNoIndex)
               {
                  result = DoLoadFromArchive(pRes->archiveId, pRes->offset, pRes->index, pFormat, param, &dataSize, &pData);
               }

               if (result == S_OK)
               {
                  // Cache the resource data
                  pRes->pFormat = pFormat;
                  pRes->pData = pData;
                  pRes->dataSize = dataSize;
               }
            }
         }

         if (pRes->pData != NULL)
         {
            *ppData = pRes->pData;
            return S_OK;
         }
      }
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cResourceManager::Unload(const char * pszName, tResourceType type)
{
   // TODO: For now resources unloaded only on exit
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cResourceManager::Lock(const char * pszName, tResourceType type)
{
   // TODO
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cResourceManager::Unlock(const char * pszName, tResourceType type)
{
   // TODO
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cResourceManager::RegisterFormat(tResourceType type,
                                         tResourceType typeDepend,
                                         const char * pszExtension,
                                         tResourceLoad pfnLoad,
                                         tResourcePostload pfnPostload,
                                         tResourceUnload pfnUnload)
{
   if (!type)
   {
      return E_INVALIDARG;
   }

   if (typeDepend)
   {
      if (pfnLoad != NULL)
      {
         WarnMsg("Dependent resource type loader specifies a load \
            function which will never be called\n");
      }
   }

   if (pfnLoad == NULL && !typeDepend)
   {
      // Must have at least a load function
      WarnMsg1("No load function specified when registering \"%s\" resource format\n",
            pszExtension != NULL ? pszExtension : "<NONE>");
      return E_POINTER;
   }

   if (pfnUnload == NULL)
   {
      WarnMsg1("No unload function specified for \"%s\" resource format\n",
            pszExtension != NULL ? pszExtension : "<NONE>");
   }

   uint extensionId = kNoIndex;
   if (pszExtension != NULL)
   {
      extensionId = GetExtensionId(pszExtension);

      std::vector<sFormat>::const_iterator iter = m_formats.begin();
      std::vector<sFormat>::const_iterator end = m_formats.end();
      for (; iter != end; iter++)
      {
         if (iter->extensionId == extensionId)
         {
            WarnMsg1("Resource format with file extension \"%s\" already registered\n",
               pszExtension != NULL ? pszExtension : "<NONE>");
            return E_FAIL;
         }
      }
   }

   sFormat format;
   format.type = type;
   format.typeDepend = typeDepend;
   format.extensionId = extensionId;
   format.pfnLoad = pfnLoad;
   format.pfnPostload = pfnPostload;
   format.pfnUnload = pfnUnload;
   m_formats.push_back(format);

   return S_OK;
}

////////////////////////////////////////

cResourceManager::sResource * cResourceManager::FindResourceWithFormat(
   const char * pszName, tResourceType type, sFormat * pFormat)
{
   if (pszName == NULL)
   {
      return NULL;
   }

   uint extensionId = GetExtensionIdForName(pszName);
   if (extensionId == kNoIndex)
   {
      // TODO: Could deduce possible extensions using the type
      return NULL;
   }

   cStr name;
   cFileSpec(pszName).GetFileNameNoExt(&name);

   // Will point to an unloaded resource at the end of the loop
   sResource * pPotentialMatch = NULL;

   tResources::iterator resIter = m_resources.begin();
   tResources::iterator resEnd = m_resources.end();
   for (int index = 0; resIter != resEnd; index++, resIter++)
   {
      if (resIter->name == name)
      {
         if (resIter->pFormat == NULL && !pFormat->typeDepend
            && (resIter->archiveId != kNoIndex || resIter->dirId != kNoIndex))
         {
            pPotentialMatch = &m_resources[index];
         }
         else if (resIter->extensionId == extensionId && resIter->pFormat == pFormat)
         {
            return &m_resources[index];
         }
      }
   }

   return pPotentialMatch;
}

////////////////////////////////////////

tResult cResourceManager::DoLoadFromFile(const cFileSpec & file, sFormat * pFormat,
                                         void * param, ulong * pDataSize, void * * ppData)
{
   if (pDataSize == NULL || ppData == NULL)
   {
      return E_POINTER;
   }

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

   if (DoLoadFromReader(pReader, pFormat, dataSize, param, ppData) == S_OK)
   {
      *pDataSize = dataSize;
      return S_OK;
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cResourceManager::DoLoadFromArchive(uint archiveId, ulong offset, ulong index,
                                            sFormat * pFormat, void * param,
                                            ulong * pDataSize, void * * ppData)
{
   if (archiveId == kNoIndex || offset == kNoIndexL || index == kNoIndexL)
   {
      return E_INVALIDARG;
   }

   if (pDataSize == NULL || ppData == NULL)
   {
      return E_POINTER;
   }

   unzFile uf = m_archives[archiveId].handle;
   if (uf == NULL)
   {
      // Could actually reload the zip file on the fly using unzOpen(m_archives[...].archive)
      return E_FAIL;
   }

   unz_file_pos filePos;
   filePos.pos_in_zip_directory = offset;
   filePos.num_of_file = index;
   if (unzGoToFilePos(uf, &filePos) != UNZ_OK)
   {
      return E_FAIL;
   }

   tResult result = E_FAIL;

   unz_file_info fileInfo;
   char szFile[kUnzMaxPath];
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
            result = DoLoadFromReader(pReader, pFormat, fileInfo.uncompressed_size, param, ppData);
            if (result == S_OK)
            {
               *pDataSize = fileInfo.uncompressed_size;
            }
         }

         unzCloseCurrentFile(uf);
      }

      delete [] pBuffer;
   }

   return result;
}

////////////////////////////////////////

tResult cResourceManager::DoLoadFromReader(IReader * pReader, sFormat * pFormat, ulong dataSize,
                                           void * param, void * * ppData)
{
   if (pReader == NULL || ppData == NULL)
   {
      return E_POINTER;
   }

   if (pFormat->pfnLoad != NULL)
   {
      void * pData = (*pFormat->pfnLoad)(pReader);
      if (pData != NULL)
      {
         if (pFormat->pfnPostload != NULL)
         {
            // Assume the postload function cleans up pData or passes
            // it through (or returns NULL)
            pData = (*pFormat->pfnPostload)(pData, dataSize, param);
         }

         if (pData != NULL)
         {
            *ppData = pData;
            return S_OK;
         }
      }
   }

   return E_FAIL;
}

////////////////////////////////////////

uint cResourceManager::DeduceFormats(const char * pszName, tResourceType type,
                                     sFormat * * ppFormats, uint nMaxFormats)
{
   if (pszName == NULL || !type || ppFormats == NULL || nMaxFormats == 0)
   {
      return 0;
   }

   uint extensionId = GetExtensionIdForName(pszName);

   // if name has file extension, resource class plus extension determines format
   // plus, include all formats that can generate the resource class from a dependent type
   if (extensionId != kNoIndex)
   {
      tFormats::const_iterator fIter = m_formats.begin();
      tFormats::const_iterator fEnd = m_formats.end();
      uint iFormat = 0;
      for (; (fIter != fEnd) && (iFormat < nMaxFormats); fIter++)
      {
         if (SameType(fIter->type, type))
         {
            if ((fIter->extensionId == extensionId) || fIter->typeDepend)
            {
               ppFormats[iFormat++] = &m_formats[fIter - m_formats.begin()];
            }
         }
      }
      return iFormat;
   }
   // else resource class alone determines set of possible formats
   else
   {
      tFormats::const_iterator fIter = m_formats.begin();
      tFormats::const_iterator fEnd = m_formats.end();
      uint iFormat = 0;
      for (; (fIter != fEnd) && (iFormat < nMaxFormats); fIter++)
      {
         if (SameType(fIter->type, type))
         {
            ppFormats[iFormat++] = &m_formats[fIter - m_formats.begin()];
         }
      }
      return iFormat;
   }

   return 0;
}

////////////////////////////////////////

uint cResourceManager::GetExtensionId(const char * pszExtension)
{
   Assert(pszExtension != NULL);

   std::vector<cStr>::const_iterator f = std::find(m_extensions.begin(), m_extensions.end(), pszExtension);
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

uint cResourceManager::GetExtensionIdForName(const char * pszName)
{
   if (pszName == NULL)
   {
      return kNoIndex;
   }
   static const char kExtSep = '.';
   const char * pszExt = strrchr(pszName, kExtSep);
   if (pszExt != NULL)
   {
      return GetExtensionId(++pszExt);
   }
   return kNoIndex;
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
