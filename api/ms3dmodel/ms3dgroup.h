///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MS3DGROUP_H
#define INCLUDED_MS3DGROUP_H

#include "ms3dmodeldll.h"

#include "tech/readwriteapi.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

template class MS3DMODEL_API std::allocator<uint16>;

template class MS3DMODEL_API std::vector<uint16>;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dGroup
//

class MS3DMODEL_API cMs3dGroup
{
   friend class cReadWriteOps<cMs3dGroup>;

public:
   cMs3dGroup();
   cMs3dGroup(const cMs3dGroup & other);
   ~cMs3dGroup();

   const cMs3dGroup & operator =(const cMs3dGroup & other);

   const char * GetName() const;
   const std::vector<uint16> & GetTriangles() const;
   uint8 GetMaterialIndex() const;

private:
   char name[32];
   std::vector<uint16> triangleIndices;
   uint8 materialIndex;
};

///////////////////////////////////////

inline const char * cMs3dGroup::GetName() const
{
   return name;
}

///////////////////////////////////////

inline const std::vector<uint16> & cMs3dGroup::GetTriangles() const
{
   return triangleIndices;
}

///////////////////////////////////////

inline uint8 cMs3dGroup::GetMaterialIndex() const
{
   return materialIndex;
}

///////////////////////////////////////

template <>
class MS3DMODEL_API cReadWriteOps<cMs3dGroup>
{
public:
   static tResult Read(IReader * pReader, cMs3dGroup * pGroup);
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MS3DGROUP_H
