////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "resourceapi.h"
#include "filepath.h"
#include "filespec.h"
#include "fileiter.h"
#include "readwriteapi.h"
#include "globalobj.h"
#include "techtime.h"

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

#include "dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(ResourceManager);

#define LocalMsg(msg)            DebugMsgEx(ResourceManager,(msg))
#define LocalMsg1(msg,a)         DebugMsgEx1(ResourceManager,(msg),(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(ResourceManager,(msg),(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(ResourceManager,(msg),(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)   DebugMsgEx4(ResourceManager,(msg),(a),(b),(c),(d))

const uint kNoIndex = ~0;

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
   virtual tResult RegisterFormat(eResourceClass rc,
                                  const char * pszExtension,
                                  tResourceLoad pfnLoad,
                                  tResourcePostload pfnPostload,
                                  tResourceUnload pfnUnload);

private:
   struct sResource;
   sResource * InternalFindResource(const tResKey & key);
   tResult DoLoadFromFile(const cFileSpec & file, eResourceClass rc, void * param, sResource * pResource);

   uint GetExtensionId(const char * pszExtension);
   uint GetDirectoryId(const char * pszDir);

   std::vector<cStr> m_extensions;
   std::vector<cFilePath> m_dirs;

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
      uint extensionId;
      uint dirId;
      cStr name;
      sFormat * pFormat;
      ulong refCount;
      void * pData;
      ulong dataSize;
      double timeLastUsed;
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
         res.extensionId = kNoIndex;
         const char * pExt = file.GetFileExt();
         if (pExt && strlen(pExt) > 0)
         {
            res.extensionId = GetExtensionId(pExt);
         }
         res.dirId = GetDirectoryId(pszDir);
         Verify(file.GetFileNameNoExt(&res.name));
         res.pFormat = NULL;
         res.refCount = 0;
         res.pData = 0;
         res.dataSize = 0;
         res.timeLastUsed = 0;
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
   // TODO
#if 0
   cFileSpec f(pszPath);
   if (strcmp(f.GetFileExt(), "zip") == 0)
   {
      cZipArchive * pZip = new cZipArchive;
      if (pZip != NULL)
      {
         if (pZip->Open(f) == S_OK)
         {
            m_zipArchives.push_back(pZip);
         }
         else
         {
            delete pZip;
         }
      }
   }
#endif
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cResourceManager::Load(const tResKey & key, void * param, void * * ppData)
{
   if (ppData == NULL)
   {
      return E_POINTER;
   }

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
      return E_FAIL;
   }

   cStr name;
   cFileSpec(key.GetName()).GetFileNameNoExt(&name);

   sResource * pRes = InternalFindResource(key);
   if (pRes != NULL)
   {
      if (pRes->pData == NULL)
      {
         cFileSpec file(key.GetName());
         file.SetPath(m_dirs[pRes->dirId]);
         if (DoLoadFromFile(file, key.GetClass(), param, pRes) != S_OK)
         {
            return E_FAIL;
         }
      }

      if (pRes->pData != NULL)
      {
         pRes->refCount += 1;
         pRes->timeLastUsed = TimeGetSecs();
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
      if (pRes->refCount == 0)
      {
         return S_FALSE;
      }
      else
      {
         pRes->refCount -= 1;
         if (pRes->refCount == 0)
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
   }

   return E_FAIL;
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

   std::vector<sFormat>::iterator fIter = m_formats.begin();
   std::vector<sFormat>::iterator fEnd = m_formats.end();
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
      return m_extensions.size() - 1;
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

void ResourceManagerCreate()
{
   cAutoIPtr<IResourceManager>(new cResourceManager);
}

////////////////////////////////////////////////////////////////////////////////
