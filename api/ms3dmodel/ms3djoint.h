///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MS3DJOINT_H
#define INCLUDED_MS3DJOINT_H

#include "ms3dmodeldll.h"

#include "tech/readwriteapi.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

typedef struct msBone msBone;

#pragma pack(push,1)

struct sMs3dRotationKeyframe
{
   float time;
   float rotation[3];
};

struct sMs3dPositionKeyframe
{
   float time;
   float position[3];
};

#pragma pack(pop)

template class MS3DMODEL_API std::allocator<sMs3dRotationKeyframe>;
template class MS3DMODEL_API std::allocator<sMs3dPositionKeyframe>;

template class MS3DMODEL_API std::vector<sMs3dRotationKeyframe>;
template class MS3DMODEL_API std::vector<sMs3dPositionKeyframe>;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dJoint
//

class MS3DMODEL_API cMs3dJoint
{
   friend class cReadWriteOps<cMs3dJoint>;

public:
   cMs3dJoint();
   cMs3dJoint(msBone * pBone);
   cMs3dJoint(const cMs3dJoint & other);
   ~cMs3dJoint();

   const cMs3dJoint & operator =(const cMs3dJoint &);

   const char * GetName() const;
   const char * GetParentName() const;
   const float * GetRotation() const;
   const float * GetPosition() const;
   const std::vector<sMs3dRotationKeyframe> & GetRotationKeys() const;
   const std::vector<sMs3dPositionKeyframe> & GetPositionKeys() const;

private:
   byte m_flags;
   char m_name[32];
   char m_parentName[32];
   float rotation[3];
   float position[3];
   std::vector<sMs3dRotationKeyframe> keyFramesRot;
   std::vector<sMs3dPositionKeyframe> keyFramesTrans;
};

///////////////////////////////////////

inline const char * cMs3dJoint::GetName() const
{
   return m_name;
}

///////////////////////////////////////

inline const char * cMs3dJoint::GetParentName() const
{
   return m_parentName;
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

inline const std::vector<sMs3dRotationKeyframe> & cMs3dJoint::GetRotationKeys() const
{
   return keyFramesRot;
}

///////////////////////////////////////

inline const std::vector<sMs3dPositionKeyframe> & cMs3dJoint::GetPositionKeys() const
{
   return keyFramesTrans;
}

///////////////////////////////////////

template <>
class MS3DMODEL_API cReadWriteOps<cMs3dJoint>
{
public:
   static tResult Read(IReader * pReader, cMs3dJoint * pJoint);
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MS3DJOINT_H
