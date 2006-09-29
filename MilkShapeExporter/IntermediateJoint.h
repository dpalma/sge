///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_INTERMEDIATEJOINT_H
#define INCLUDED_INTERMEDIATEJOINT_H

#include "msLib.h"

#include "readwriteapi.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cIntermediateJoint
//

class cIntermediateJoint
{
public:
   cIntermediateJoint();
   cIntermediateJoint(msBone * pBone);
   cIntermediateJoint(const cIntermediateJoint & other);
   ~cIntermediateJoint();

   byte GetFlags() const;
   const char * GetName() const;
   const char * GetParentName() const;
   const float * GetRotation() const;
   const float * GetPosition() const;
   const std::vector<msRotationKey> & GetRotationKeys() const;
   const std::vector<msPositionKey> & GetPositionKeys() const;

private:
   byte flags;
   char name[MS_MAX_NAME];
   char parentName[MS_MAX_NAME];
   float rotation[3];
   float position[3];
   std::vector<msRotationKey> rotationKeys;
   std::vector<msPositionKey> positionKeys;
};

///////////////////////////////////////

inline byte cIntermediateJoint::GetFlags() const
{
   return flags;
}

///////////////////////////////////////

inline const char * cIntermediateJoint::GetName() const
{
   return name;
}

///////////////////////////////////////

inline const char * cIntermediateJoint::GetParentName() const
{
   return parentName;
}

///////////////////////////////////////

inline const float * cIntermediateJoint::GetRotation() const
{
   return rotation;
}

///////////////////////////////////////

inline const float * cIntermediateJoint::GetPosition() const
{
   return position;
}

///////////////////////////////////////

inline const std::vector<msRotationKey> & cIntermediateJoint::GetRotationKeys() const
{
   return rotationKeys;
}

///////////////////////////////////////

inline const std::vector<msPositionKey> & cIntermediateJoint::GetPositionKeys() const
{
   return positionKeys;
}


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_INTERMEDIATEJOINT_H
