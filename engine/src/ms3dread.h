///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MS3DREAD_H
#define INCLUDED_MS3DREAD_H

#include "readwriteapi.h"
#include "skeleton.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IReader);
F_DECLARE_INTERFACE(IKeyFrameInterpolator);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dGroup
//

class cMs3dGroup
{
   friend class cReadWriteOps<cMs3dGroup>;

public:
   cMs3dGroup();

   byte GetFlags() const;
   const char * GetName() const;
   size_t GetNumTriangles() const;
   uint GetTriangle(int index) const;
   int GetMaterialIndex() const;

private:
   byte flags;
   char name[32];
   std::vector<uint16> triangleIndices;
   char materialIndex;
};

///////////////////////////////////////

inline byte cMs3dGroup::GetFlags() const
{
   return flags;
}

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

inline int cMs3dGroup::GetMaterialIndex() const
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

struct sMs3dBoneInfo
{
   char name[kMaxBoneName];
   char parentName[kMaxBoneName];
   float rotation[3];
   float position[3];
};

template <>
class cReadWriteOps<sMs3dBoneInfo>
{
public:
   static tResult Read(IReader * pReader, sMs3dBoneInfo * pBoneInfo);
};

///////////////////////////////////////////////////////////////////////////////

tResult ReadSkeleton(IReader * pReader, 
                     std::vector<sBoneInfo> * pBones,
                     std::vector<IKeyFrameInterpolator *> * pInterpolators);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MS3DREAD_H
