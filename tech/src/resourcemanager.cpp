///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"
#include "resmgr.h"
#include "filepath.h"
#include "filespec.h"
#include "readwriteapi.h"

#include <stdio.h>
#include <vector>

#include "dbgalloc.h" // must be last header

using namespace std;

///////////////////////////////////////////////////////////////////////////////

vector<cFilePath> g_searchPaths;

///////////////////////////////////////////////////////////////////////////////

IReader * ResourceFind(const char * pszName)
{
   if (cFileSpec(pszName).Exists())
      return FileCreateReader(cFileSpec(pszName));

   vector<cFilePath>::iterator iter;
   for (iter = g_searchPaths.begin(); iter != g_searchPaths.end(); iter++)
   {
      cFileSpec file(*iter, pszName);
      if (file.Exists())
      {
         return FileCreateReader(file);
      }
   }

   return NULL;
}

///////////////////////////////////////////////////////////////////////////////

void ResourceAddSearchPath(const char * pszPath, bool bRecurse)
{
   cFilePath path(pszPath);
   path.MakeFullPath();

   g_searchPaths.push_back(path);

   if (bRecurse)
   {
      vector<string> dirs;
      if (path.ListDirs(&dirs) > 0)
      {
         vector<string>::iterator iter;
         for (iter = dirs.begin(); iter != dirs.end(); iter++)
         {
            cFilePath searchPath(path);
            searchPath.AddRelative(iter->c_str());

            ResourceAddSearchPath(searchPath.GetPath(), bRecurse);
         }
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceManager
//

class cResourceManager : public cComObject<IMPLEMENTS(IResourceManager)>
{
public:
   virtual IReader * Find(const char * pszName);
   virtual void AddSearchPath(const char * pszPath, bool bRecurse);

private:
   std::vector<cFilePath> m_searchPaths;
};

///////////////////////////////////////

IReader * cResourceManager::Find(const char * pszName)
{
   if (cFileSpec(pszName).Exists())
      return FileCreateReader(cFileSpec(pszName));

   std::vector<cFilePath>::iterator iter;
   for (iter = m_searchPaths.begin(); iter != m_searchPaths.end(); iter++)
   {
      cFileSpec file(*iter, pszName);
      if (file.Exists())
      {
         return FileCreateReader(file);
      }
   }

   return NULL;
}

///////////////////////////////////////

void cResourceManager::AddSearchPath(const char * pszPath, bool bRecurse)
{
   cFilePath path(pszPath);
   path.MakeFullPath();

   m_searchPaths.push_back(path);

   if (bRecurse)
   {
      std::vector<std::string> dirs;
      if (path.ListDirs(&dirs) > 0)
      {
         std::vector<std::string>::iterator iter;
         for (iter = dirs.begin(); iter != dirs.end(); iter++)
         {
            cFilePath searchPath(path);
            searchPath.AddRelative(iter->c_str());

            AddSearchPath(searchPath.GetPath(), bRecurse);
         }
      }
   }
}

///////////////////////////////////////

IResourceManager * ResourceManagerCreate()
{
   return static_cast<IResourceManager *>(new cResourceManager);
}

///////////////////////////////////////////////////////////////////////////////
