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

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceManagerGlobalObj
//

class cResourceManagerGlobalObj : public cGlobalObject<IMPLEMENTS(IResourceManager)>
{
public:
   cResourceManagerGlobalObj();

   virtual IReader * Find(const char * pszName);
   virtual void AddSearchPath(const char * pszPath, bool bRecurse);

private:
   std::vector<cFilePath> m_searchPaths;
};

///////////////////////////////////////

cResourceManagerGlobalObj::cResourceManagerGlobalObj()
 : cGlobalObject<IMPLEMENTS(IResourceManager)>(kResourceManagerName)
{
}

///////////////////////////////////////

IReader * cResourceManagerGlobalObj::Find(const char * pszName)
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

void cResourceManagerGlobalObj::AddSearchPath(const char * pszPath, bool bRecurse)
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

void ResourceManagerCreate()
{
   cAutoIPtr<IResourceManager>(new cResourceManagerGlobalObj);
}

///////////////////////////////////////////////////////////////////////////////
