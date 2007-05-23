///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MS3DGROUP_H
#define INCLUDED_MS3DGROUP_H

#include "tech/readwriteapi.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dGroup
//

class cMs3dGroup
{
   friend class cReadWriteOps<cMs3dGroup>;

public:
   cMs3dGroup();
   cMs3dGroup(const cMs3dGroup & other);
   ~cMs3dGroup();

   const cMs3dGroup & operator =(const cMs3dGroup & other);

   const char * GetName() const;
   size_t GetNumTriangles() const;
   uint GetTriangle(int index) const;
   const std::vector<uint16> & GetTriangles() const;
   uint8 GetMaterialIndex() const;

private:
   byte flags;
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

inline size_t cMs3dGroup::GetNumTriangles() const
{
   return triangleIndices.size();
}

///////////////////////////////////////

inline uint cMs3dGroup::GetTriangle(int index) const
{
   return triangleIndices[index];
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
class cReadWriteOps<cMs3dGroup>
{
public:
   static tResult Read(IReader * pReader, cMs3dGroup * pGroup);
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MS3DGROUP_H
