///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MS3DJOINT_H
#define INCLUDED_MS3DJOINT_H

#include "ms3d.h"

#include "tech/readwriteapi.h"
#include "tech/vec3.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dJoint
//

class cMs3dJoint
{
   friend class cReadWriteOps<cMs3dJoint>;

   void operator =(const cMs3dJoint &);

public:
   cMs3dJoint();
   cMs3dJoint(const cMs3dJoint & other);
   ~cMs3dJoint();

   byte GetFlags() const;
   const char * GetName() const;
   const char * GetParentName() const;
   const float * GetRotation() const;
   const float * GetPosition() const;
   const std::vector<ms3d_keyframe_rot_t> & GetKeyFramesRot() const;
   const std::vector<ms3d_keyframe_pos_t> & GetKeyFramesTrans() const;

private:
   byte flags;
   char name[32];
   char parentName[32];
   float rotation[3];
   float position[3];
   std::vector<ms3d_keyframe_rot_t> keyFramesRot;
   std::vector<ms3d_keyframe_pos_t> keyFramesTrans;
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

inline const std::vector<ms3d_keyframe_rot_t> & cMs3dJoint::GetKeyFramesRot() const
{
   return keyFramesRot;
}

///////////////////////////////////////

inline const std::vector<ms3d_keyframe_pos_t> & cMs3dJoint::GetKeyFramesTrans() const
{
   return keyFramesTrans;
}

///////////////////////////////////////

template <>
class cReadWriteOps<cMs3dJoint>
{
public:
   static tResult Read(IReader * pReader, cMs3dJoint * pJoint);
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MS3DJOINT_H
