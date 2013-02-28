///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ms3dmodel/ms3dgroup.h"

#include "tech/matrix4.h"
#include "tech/vec3.h"

#include <map>

#include "tech/dbgalloc.h" // must be last header

using namespace std;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dGroup
//

///////////////////////////////////////

cMs3dGroup::cMs3dGroup()
 : materialIndex(~0)
{
}

///////////////////////////////////////

cMs3dGroup::cMs3dGroup(const cMs3dGroup & other)
 : materialIndex(other.materialIndex)
{
   memcpy(name, other.name, sizeof(name));
   triangleIndices.insert(triangleIndices.begin(), other.triangleIndices.begin(), other.triangleIndices.end());
}

///////////////////////////////////////

cMs3dGroup::~cMs3dGroup()
{
}

///////////////////////////////////////

const cMs3dGroup & cMs3dGroup::operator =(const cMs3dGroup & other)
{
   materialIndex = other.materialIndex;
   memcpy(name, other.name, sizeof(name));
   triangleIndices.clear();
   triangleIndices.insert(triangleIndices.begin(), other.triangleIndices.begin(), other.triangleIndices.end());
   return *this;
}

///////////////////////////////////////

tResult cReadWriteOps<cMs3dGroup>::Read(IReader * pReader, cMs3dGroup * pGroup)
{
   Assert(pReader != NULL);
   Assert(pGroup != NULL);

   tResult result = E_FAIL;

   do
   {
      byte ignoreFlags;
      if (pReader->Read(&ignoreFlags, sizeof(ignoreFlags)) != S_OK)
         break;

      if (pReader->Read(pGroup->name, sizeof(pGroup->name)) != S_OK)
         break;

      uint16 nTriangles;
      if (pReader->Read(&nTriangles, sizeof(nTriangles)) != S_OK)
         break;

      pGroup->triangleIndices.resize(nTriangles);

      if (pReader->Read(&pGroup->triangleIndices[0], pGroup->triangleIndices.size() * sizeof(uint16)) != S_OK)
         break;

      if (pReader->Read(&pGroup->materialIndex, sizeof(pGroup->materialIndex)) != S_OK)
         break;

      result = S_OK;
   }
   while (0);

   return result;
}


///////////////////////////////////////////////////////////////////////////////
