////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "resmgr.h"
#include "resourceapi.h"
#include "ziparchive.h"
#include "filepath.h"
#include "filespec.h"
#include "fileiter.h"
#include "readwriteapi.h"
#include "globalobj.h"

#include <cstdio>
#include <vector>
#include <string>
#include <map>
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
public:
   cResourceManager();
   virtual ~cResourceManager();

   virtual tResult Init();
   virtual tResult Term();

   // IResourceManager methods
   virtual IReader * Find(const char * pszName);
   virtual void AddSearchPath(const char * pszPath);

   // IResourceManager2 methods
   virtual tResult AddDirectory(const char * pszDir);
   virtual tResult AddDirectoryTreeFlattened(const char * pszDir);
   virtual tResult AddArchive(const char * pszArchive);
   virtual tResult Load(const tResKey & key, void * * ppData);
   virtual tResult Unload(const tResKey & key);
   virtual tResult RegisterFormat(eResourceClass rc,
                                  const char * pszExtension,
                                  tResourceLoad pfnLoad,
                                  tResourcePostload pfnPostload,
                                  tResourceUnload pfnUnload);

private:
   uint GetExtensionId(const char * pszExtension);

   typedef std::multimap<eResourceClass, cStr> tResClassExtMap;
   tResClassExtMap m_resClassExtMap;

   std::vector<cFilePath> m_searchPaths;
   std::vector<cZipArchive *> m_zipArchives;

   struct sFormat
   {
      eResourceClass rc;
      uint extensionId;
      tResourceLoad pfnLoad;
      tResourcePostload pfnPostload;
      tResourceUnload pfnUnload;
   };
   std::vector<sFormat> m_formats;

   std::vector<cStr> m_extensions;
};

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

   m_resClassExtMap.clear();

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
   Verify(AddDirectoryTreeFlattened(pszPath) == S_OK);
}

////////////////////////////////////////

tResult cResourceManager::AddDirectory(const char * pszDir)
{
   if (pszDir == NULL)
   {
      return E_POINTER;
   }

   // HACK: support the legacy behavior
   m_searchPaths.push_back(cFilePath(pszDir));

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
      // TODO: do something; cache information about the file
      if ((attribs & kFA_Directory) == kFA_Directory)
      {
         DebugMsg1("Dir: %s\n", szFile);
      }
      else
      {
         DebugMsg1("File: %s\n", szFile);
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

   std::vector<std::string> dirs;
   if (ListDirs(root, &dirs) > 0)
   {
      std::vector<std::string>::iterator iter;
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

tResult cResourceManager::Load(const tResKey & key, void * * ppData)
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

   cFileSpec file(key.GetName());

   std::vector<cFilePath>::iterator iter = m_searchPaths.begin();
   std::vector<cFilePath>::iterator end = m_searchPaths.end();
   for (; iter != end; iter++)
   {
      file.SetPath(*iter);
      if (file.Exists())
      {
         cAutoIPtr<IReader> pReader(FileCreateReader(file));
         if (!pReader)
         {
            return E_OUTOFMEMORY;
         }

         std::vector<sFormat>::iterator fIter = m_formats.begin();
         std::vector<sFormat>::iterator fEnd = m_formats.end();
         for (; fIter != fEnd; fIter++)
         {
            if (fIter->extensionId == extensionId && fIter->rc == key.GetClass())
            {
               if (fIter->pfnLoad != NULL)
               {
                  if ((*ppData = (*fIter->pfnLoad)(pReader)) != NULL)
                  {
                     return S_OK;
                  }
                  else
                  {
                     return E_FAIL;
                  }
               }
            }
         }
      }
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cResourceManager::Unload(const tResKey & key)
{
   return E_NOTIMPL;
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

void ResourceManagerCreate()
{
   cAutoIPtr<IResourceManager>(new cResourceManager);
}

////////////////////////////////////////////////////////////////////////////////
