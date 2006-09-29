///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "IntermediateJoint.h"

#include "matrix4.h"
#include "techhash.h"
#include "vec3.h"

#include <map>

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cIntermediateJoint
//

///////////////////////////////////////

cIntermediateJoint::cIntermediateJoint()
 : flags(0)
{
}

///////////////////////////////////////

cIntermediateJoint::cIntermediateJoint(msBone * pBone)
 : flags(0)
 , rotationKeys(pBone->nNumPositionKeys)
 , positionKeys(pBone->nNumRotationKeys)
{
   flags = pBone->nFlags;
   strcpy(name, pBone->szName);
   strcpy(parentName, pBone->szParentName);
   memcpy(rotation, pBone->Rotation, sizeof(rotation));
   memcpy(position, pBone->Position, sizeof(position));
   memcpy(&rotationKeys[0], pBone->pRotationKeys, rotationKeys.size() * sizeof(msRotationKey));
   memcpy(&positionKeys[0], pBone->pPositionKeys, positionKeys.size() * sizeof(msPositionKey));
}

///////////////////////////////////////

cIntermediateJoint::cIntermediateJoint(const cIntermediateJoint & other)
 : flags(other.flags)
 , rotationKeys(other.rotationKeys.size())
 , positionKeys(other.positionKeys.size())
{
   strcpy(name, other.name);
   strcpy(parentName, other.parentName);
   memcpy(rotation, other.rotation, sizeof(rotation));
   memcpy(position, other.position, sizeof(position));
   std::copy(other.rotationKeys.begin(), other.rotationKeys.end(), rotationKeys.begin());
   std::copy(other.positionKeys.begin(), other.positionKeys.end(), positionKeys.begin());
}

///////////////////////////////////////

cIntermediateJoint::~cIntermediateJoint()
{
}

///////////////////////////////////////////////////////////////////////////////
