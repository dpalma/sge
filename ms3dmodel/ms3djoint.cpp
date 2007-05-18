///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ms3djoint.h"

#include "tech/matrix4.h"
#include "tech/vec3.h"
#include "tech/techhash.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dJoint
//

///////////////////////////////////////

cMs3dJoint::cMs3dJoint()
 : flags(0)
{
}

///////////////////////////////////////

cMs3dJoint::cMs3dJoint(const cMs3dJoint & other)
 : flags(other.flags)
 , keyFramesRot(other.keyFramesRot.size())
 , keyFramesTrans(other.keyFramesTrans.size())
{
   memcpy(name, other.name, sizeof(name));
   memcpy(parentName, other.parentName, sizeof(parentName));
   memcpy(rotation, other.rotation, sizeof(rotation));
   memcpy(position, other.position, sizeof(position));
   std::copy(other.keyFramesRot.begin(), other.keyFramesRot.end(), keyFramesRot.begin());
   std::copy(other.keyFramesTrans.begin(), other.keyFramesTrans.end(), keyFramesTrans.begin());
}

///////////////////////////////////////

cMs3dJoint::~cMs3dJoint()
{
}

///////////////////////////////////////

tResult cReadWriteOps<cMs3dJoint>::Read(IReader * pReader, cMs3dJoint * pJoint)
{
   Assert(pReader != NULL);
   Assert(pJoint != NULL);

   tResult result = E_FAIL;

   do
   {
      uint16 nKeyFramesRot;
      uint16 nKeyFramesTrans;

      if (pReader->Read(&pJoint->flags, sizeof(pJoint->flags)) != S_OK
         || pReader->Read(pJoint->name, sizeof(pJoint->name)) != S_OK
         || pReader->Read(pJoint->parentName, sizeof(pJoint->parentName)) != S_OK
         || pReader->Read(pJoint->rotation, sizeof(pJoint->rotation)) != S_OK
         || pReader->Read(pJoint->position, sizeof(pJoint->position)) != S_OK
         || pReader->Read(&nKeyFramesRot) != S_OK
         || pReader->Read(&nKeyFramesTrans) != S_OK)
      {
         break;
      }

      if (nKeyFramesRot != nKeyFramesTrans)
      {
         break;
      }

      if (nKeyFramesRot == 0)
      {
         return S_OK;
      }

      pJoint->keyFramesRot.resize(nKeyFramesRot);
      pJoint->keyFramesTrans.resize(nKeyFramesTrans);

      if (pReader->Read(&pJoint->keyFramesRot[0], nKeyFramesRot * sizeof(ms3d_keyframe_rot_t)) != S_OK
         || pReader->Read(&pJoint->keyFramesTrans[0], nKeyFramesTrans * sizeof(ms3d_keyframe_pos_t)) != S_OK)
      {
         break;
      }

      result = S_OK;
   }
   while (0);

   return result;
}


///////////////////////////////////////////////////////////////////////////////
