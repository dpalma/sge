////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "resmgr.h"
#include "resourceapi.h"
#include "ziparchive.h"
#include "filepath.h"
#include "filespec.h"
#include "readwriteapi.h"
#include "globalobj.h"

#include <cstdio>
#include <vector>
#include <string>
#include <map>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#endif

#include "dbgalloc.h" // must be last header

////////////////////////////////////////////////////////////////////////////////

static size_t ListDirs(const cFilePath & path, std::vector<std::string> * pDirs)
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

class cResourceManager : public cComObject3<IMPLEMENTS(IResourceManager),
                                            IMPLEMENTS(IResourceManager2),
                                            cGlobalObjectBase, &IID_IGlobalObject>
{
   friend tResult RegisterResourceFormat(eResourceClass resClass,
                                         const char * pszExtension,
                                         IResourceFormat * pResFormat);

public:
   cResourceManager();
   virtual ~cResourceManager();

   virtual tResult Init();
   virtual tResult Term();

   // IResourceManager methods
   virtual IReader * Find(const char * pszName);
   virtual void AddSearchPath(const char * pszPath);

   // IResourceManager2 methods
   virtual tResult AddResourceStore(const char * pszName, eResourceStorePriority priority);
   virtual tResult Load(const tResKey & key, IResource * * ppResource);
   virtual tResult RegisterResourceFormat(eResourceClass resClass,
                                          const char * pszExtension,
                                          IResourceFormat * pResFormat);

private:
   void ConsumeDeferredResourceFormats();
   static void DiscardDeferredResourceFormats();

   struct sDeferredResourceFormat
   {
      eResourceClass resClass;
      cStr extension;
      IResourceFormat * pResFormat;
      sDeferredResourceFormat * pNext;
   };

   static struct sDeferredResourceFormat * gm_pDeferredResourceFormats;

   static bool gm_bInitialized;

   typedef std::multimap<eResourceClass, cStr> tResClassExtMap;
   typedef std::map<cStr, IResourceFormat *> tResExtFormatMap;

   tResClassExtMap m_resClassExtMap;
   tResExtFormatMap m_resExtFormatMap;

   std::vector<cFilePath> m_searchPaths;
   std::vector<cZipArchive *> m_zipArchives;
};

////////////////////////////////////////

cResourceManager::sDeferredResourceFormat * cResourceManager::gm_pDeferredResourceFormats = NULL;

////////////////////////////////////////

bool cResourceManager::gm_bInitialized = false;

////////////////////////////////////////

cResourceManager::cResourceManager()
{
   Construct(IID_IResourceManager, kResourceManagerName, NULL, 0);

   if (AccessGlobalObjectRegistry() != NULL)
   {
      Verify(SUCCEEDED(AccessGlobalObjectRegistry()->Register(IID_IResourceManager, static_cast<IResourceManager *>(this))));
      Verify(SUCCEEDED(AccessGlobalObjectRegistry()->Register(IID_IResourceManager2, static_cast<IResourceManager2 *>(this))));
   }
}

////////////////////////////////////////

cResourceManager::~cResourceManager()
{
   Assert(m_zipArchives.empty());
}

////////////////////////////////////////

tResult cResourceManager::Init()
{
   gm_bInitialized = true;

   ConsumeDeferredResourceFormats();

   return S_OK;
}

////////////////////////////////////////

tResult cResourceManager::Term()
{
   std::vector<cZipArchive *>::iterator iter;
   for (iter = m_zipArchives.begin(); iter != m_zipArchives.end(); iter++)
   {
      delete (*iter);
   }
   m_zipArchives.clear();

   tResExtFormatMap::iterator iter2;
   for (iter2 = m_resExtFormatMap.begin(); iter2 != m_resExtFormatMap.end(); iter2++)
   {
      SafeRelease(iter2->second);
   }
   m_resExtFormatMap.clear();

   m_resClassExtMap.clear();

   DiscardDeferredResourceFormats();

   gm_bInitialized = false;

   return S_OK;
}

////////////////////////////////////////

IReader * cResourceManager::Find(const char * pszName)
{
   if (!m_zipArchives.empty())
   {
      std::vector<cZipArchive *>::iterator iter;
      for (iter = m_zipArchives.begin(); iter != m_zipArchives.end(); iter++)
      {
         IReader * pReader = NULL;
         if ((*iter)->OpenMember(pszName, &pReader) == S_OK)
         {
            return pReader;
         }
      }
   }
   else
   {
      if (cFileSpec(pszName).Exists())
      {
         return FileCreateReader(cFileSpec(pszName));
      }

      std::vector<cFilePath>::iterator iter;
      for (iter = m_searchPaths.begin(); iter != m_searchPaths.end(); iter++)
      {
         cFileSpec file(pszName);
         file.SetPath(*iter);
         if (file.Exists())
         {
            return FileCreateReader(file);
         }
      }
   }

   return NULL;
}

////////////////////////////////////////

void cResourceManager::AddSearchPath(const char * pszPath)
{
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
   else
   {
      cFilePath path(pszPath);
      path.MakeFullPath();

      m_searchPaths.push_back(path);

      std::vector<std::string> dirs;
      if (ListDirs(path, &dirs) > 0)
      {
         std::vector<std::string>::iterator iter;
         for (iter = dirs.begin(); iter != dirs.end(); iter++)
         {
            cFilePath searchPath(path);
            searchPath.AddRelative(iter->c_str());

            AddSearchPath(searchPath.GetPath());
         }
      }
   }
}

////////////////////////////////////////

tResult cResourceManager::AddResourceStore(const char * pszName, eResourceStorePriority priority)
{
   // TODO
   return E_NOTIMPL;
}

////////////////////////////////////////
// Given key no file extension
//    Get all possible extensions and corresponding IResourceFormat interfaces
//    Try each
// Given key with extension
//    Get specific IResourceFormat for that extension

tResult cResourceManager::Load(const tResKey & key, IResource * * ppResource)
{
   if (ppResource == NULL)
   {
      return E_POINTER;
   }

   static const char kExtSep = '.';

   const char * pszExt = strrchr(key.GetName(), kExtSep);
   if (pszExt != NULL)
   {
      cStr ext(++pszExt);

      tResExtFormatMap::iterator f = m_resExtFormatMap.find(ext);
      if (f != m_resExtFormatMap.end())
      {
         cAutoIPtr<IReader> pReader(Find(key.GetName()));

         if (!!pReader)
         {
            return f->second->Load(key, pReader, ppResource);
         }
      }
   }
   else
   {
      tResClassExtMap::iterator iter;
      for (iter = m_resClassExtMap.lower_bound(key.GetClass());
         iter != m_resClassExtMap.upper_bound(key.GetClass()); iter++)
      {
         cStr name(key.GetName());
         name += kExtSep;
         name += iter->second;

         cAutoIPtr<IReader> pReader(Find(name.c_str()));

         if (!!pReader)
         {
            tResExtFormatMap::iterator f = m_resExtFormatMap.find(iter->second);
            if (f != m_resExtFormatMap.end())
            {
               return f->second->Load(tResKey(name, key.GetClass()), pReader, ppResource);
            }
         }
      }
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cResourceManager::RegisterResourceFormat(eResourceClass resClass,
                                                 const char * pszExtension,
                                                 IResourceFormat * pResFormat)
{
   if (pszExtension == NULL || pResFormat == NULL)
   {
      return E_POINTER;
   }

   cStr extension(pszExtension);

   tResExtFormatMap::iterator;
   if (m_resExtFormatMap.find(extension) != m_resExtFormatMap.end())
   {
      return S_FALSE;
   }

   tResClassExtMap::iterator iter;
   for (iter = m_resClassExtMap.lower_bound(resClass);
      iter != m_resClassExtMap.upper_bound(resClass); iter++)
   {
      if (iter->second == extension)
      {
         return S_FALSE;
      }
   }

   m_resClassExtMap.insert(std::make_pair(resClass, extension));
   m_resExtFormatMap.insert(std::make_pair(extension, CTAddRef(pResFormat)));

   return S_OK;
}

////////////////////////////////////////

tResult RegisterResourceFormat(eResourceClass resClass,
                               const char * pszExtension,
                               IResourceFormat * pResFormat)
{
   if (pszExtension == NULL || pResFormat == NULL)
   {
      return E_POINTER;
   }

   // If resource manager is already up, register it directly.
   // Otherwise, add it to the deferred list.
   if (cResourceManager::gm_bInitialized)
   {
      UseGlobal(ResourceManager2);
      return pResourceManager2->RegisterResourceFormat(resClass, pszExtension, pResFormat);
   }
   else
   {
      cResourceManager::sDeferredResourceFormat * p = new cResourceManager::sDeferredResourceFormat;
      if (p == NULL)
      {
         return E_OUTOFMEMORY;
      }

      p->resClass = resClass;
      p->extension = pszExtension;
      p->pResFormat = CTAddRef(pResFormat);
      p->pNext = cResourceManager::gm_pDeferredResourceFormats;
      cResourceManager::gm_pDeferredResourceFormats = p;
      return S_OK;
   }

   return E_FAIL;
}

////////////////////////////////////////

void cResourceManager::ConsumeDeferredResourceFormats()
{
   while (gm_pDeferredResourceFormats != NULL)
   {
      RegisterResourceFormat(
         gm_pDeferredResourceFormats->resClass, 
         gm_pDeferredResourceFormats->extension.c_str(), 
         gm_pDeferredResourceFormats->pResFormat);

      SafeRelease(gm_pDeferredResourceFormats->pResFormat);

      sDeferredResourceFormat * p = gm_pDeferredResourceFormats;
      gm_pDeferredResourceFormats = gm_pDeferredResourceFormats->pNext;
      delete p;
   }
}

////////////////////////////////////////

void cResourceManager::DiscardDeferredResourceFormats()
{
   sDeferredResourceFormat * p = gm_pDeferredResourceFormats;
   while (p != NULL)
   {
      gm_pDeferredResourceFormats = gm_pDeferredResourceFormats->pNext;
      delete p;
      p = gm_pDeferredResourceFormats;
   }
}

////////////////////////////////////////

void ResourceManagerCreate()
{
   cAutoIPtr<IResourceManager>(new cResourceManager);
}

////////////////////////////////////////////////////////////////////////////////
