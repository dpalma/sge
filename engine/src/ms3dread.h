///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MS3DREAD_H
#define INCLUDED_MS3DREAD_H

#include "readwriteapi.h"
#include "ms3d.h"

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

   byte GetFlags() const;
   const char * GetName() const;
   int GetNumTriangles() const;
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

inline int cMs3dGroup::GetNumTriangles() const
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
//
// CLASS: cMs3dJoint
//

class cMs3dJoint
{
   friend class cReadWriteOps<cMs3dJoint>;

public:
   cMs3dJoint();

#ifdef _DEBUG
   void DebugPrint();
#endif

   byte GetFlags() const;
   const char * GetName() const;
   const char * GetParentName() const;
   const float * GetRotation() const;
   const float * GetPosition() const;
   int GetNumRotationKeys() const;
   void GetRotationKey(int index, ms3d_keyframe_rot_t * pRotationKey) const;
   int GetNumPositionKeys() const;
   void GetPositionKey(int index, ms3d_keyframe_pos_t * pPositionKey) const;

private:
   byte flags;
   char name[32];
   char parentName[32];
   float rotation[3]; // local reference matrix
   float position[3];

   std::vector<ms3d_keyframe_rot_t> keyFramesRot; // local animation matrices
   std::vector<ms3d_keyframe_pos_t> keyFramesTrans; // local animation matrices
};

///////////////////////////////////////

inline byte cMs3dJoint::GetFlags() const
{
   return flags;
}

///////////////////////////////////////

inline const char * cMs3dJoint::GetName() const
{
   return name;
}

///////////////////////////////////////

inline const char * cMs3dJoint::GetParentName() const
{
   return parentName;
}

///////////////////////////////////////

inline const float * cMs3dJoint::GetRotation() const
{
   return rotation;
}

///////////////////////////////////////

inline const float * cMs3dJoint::GetPosition() const
{
   return position;
}

///////////////////////////////////////

inline int cMs3dJoint::GetNumRotationKeys() const
{
   return keyFramesRot.size();
}

///////////////////////////////////////

inline void cMs3dJoint::GetRotationKey(int index, ms3d_keyframe_rot_t * pRotationKey) const
{
   if (pRotationKey != NULL)
   {
      memcpy(pRotationKey, &keyFramesRot[index], sizeof(ms3d_keyframe_rot_t));
   }
}

///////////////////////////////////////

inline int cMs3dJoint::GetNumPositionKeys() const
{
   return keyFramesTrans.size();
}

///////////////////////////////////////

inline void cMs3dJoint::GetPositionKey(int index, ms3d_keyframe_pos_t * pPositionKey) const
{
   if (pPositionKey != NULL)
   {
      memcpy(pPositionKey, &keyFramesTrans[index], sizeof(ms3d_keyframe_pos_t));
   }
}

///////////////////////////////////////

template <>
class cReadWriteOps<cMs3dJoint>
{
public:
   static tResult Read(IReader * pReader, cMs3dJoint * pJoint);
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MS3DREAD_H
