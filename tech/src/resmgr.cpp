///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "resmgr.h"
#include "filepath.h"
#include "filespec.h"
#include "readwriteapi.h"
#include "globalobj.h"

#include <cstdio>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

static int ListDirs(const cFilePath & path, std::vector<std::string> * pDirs)
{
   Assert(pDirs != NULL);
   if (pDirs == NULL)
      return -1;

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

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceManager
//

class cResourceManager : public cGlobalObject<IMPLEMENTS(IResourceManager)>
{
public:
   cResourceManager();
   virtual ~cResourceManager();

   virtual IReader * Find(const char * pszName);
   virtual void AddSearchPath(const char * pszPath);

private:
   std::vector<cFilePath> m_searchPaths;
};

///////////////////////////////////////

cResourceManager::cResourceManager()
 : cGlobalObject<IMPLEMENTS(IResourceManager)>(kResourceManagerName)
{
}

///////////////////////////////////////

cResourceManager::~cResourceManager()
{
}

///////////////////////////////////////

IReader * cResourceManager::Find(const char * pszName)
{
   if (cFileSpec(pszName).Exists())
      return FileCreateReader(cFileSpec(pszName));

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

   return NULL;
}

///////////////////////////////////////

void cResourceManager::AddSearchPath(const char * pszPath)
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

///////////////////////////////////////

void ResourceManagerCreate()
{
   cAutoIPtr<IResourceManager>(new cResourceManager);
}

///////////////////////////////////////////////////////////////////////////////
