///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "mesh.h"
#include "ReadWriteAPI.h"
#include "FileSpec.h"
#include "resmgr.h"

#include <cstring>

#include "dbgalloc.h" // must be last header

IMesh * Load3ds(IReader * pReader);
IMesh * LoadMs3d(IReader * pReader);

///////////////////////////////////////////////////////////////////////////////

IMesh * MeshLoad(IResourceManager * pResMgr, const char * pszMesh)
{
   typedef IMesh * (* tMeshLoadFn)(IReader *);

   static const struct
   {
      const char * ext;
      tMeshLoadFn pfn;
   }
   meshFileLoaders[] =
   {
      { "3ds", Load3ds },
      { "ms3d", LoadMs3d },
   };

   cFileSpec meshFile(pszMesh);

   for (int i = 0; i < _countof(meshFileLoaders); i++)
   {
      if (stricmp(meshFileLoaders[i].ext, meshFile.GetFileExt()) == 0)
      {
         cAutoIPtr<IReader> pReader = pResMgr->Find(meshFile.GetName());
         if (!pReader)
         {
            return NULL;
         }

         IMesh * pMesh = (*meshFileLoaders[i].pfn)(pReader);
         if (pMesh != NULL)
         {
            return pMesh;
         }
      }
   }

   DebugMsg1("Unsupported mesh file format: \"%s\"\n", meshFile.GetFileExt());
   return NULL;
}

///////////////////////////////////////////////////////////////////////////////
