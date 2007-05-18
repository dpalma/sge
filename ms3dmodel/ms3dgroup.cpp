///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ms3dgroup.h"

#include "tech/matrix4.h"
#include "tech/vec3.h"
#include "tech/techhash.h"

#include <map>

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dGroup
//

///////////////////////////////////////

cMs3dGroup::cMs3dGroup()
 : flags(0), materialIndex(~0)
{
}

///////////////////////////////////////

cMs3dGroup::cMs3dGroup(const cMs3dGroup & other)
 : flags(other.flags)
 , triangleIndices(other.triangleIndices.size())
 , materialIndex(other.materialIndex)
{
   memcpy(name, other.name, sizeof(name));
   std::copy(other.triangleIndices.begin(), other.triangleIndices.end(), triangleIndices.begin());
}

///////////////////////////////////////

cMs3dGroup::~cMs3dGroup()
{
}

///////////////////////////////////////

const cMs3dGroup & cMs3dGroup::operator =(const cMs3dGroup & other)
{
   flags = other.flags;
   materialIndex = other.materialIndex;
   memcpy(name, other.name, sizeof(name));
   triangleIndices.resize(other.triangleIndices.size()),
   std::copy(other.triangleIndices.begin(), other.triangleIndices.end(), triangleIndices.begin());
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
      if (pReader->Read(&pGroup->flags, sizeof(pGroup->flags)) != S_OK)
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
