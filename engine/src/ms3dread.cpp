///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ms3dread.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dGroup
//

///////////////////////////////////////

cMs3dGroup::cMs3dGroup()
 : flags(0), materialIndex(-1)
{
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
//
// CLASS: cMs3dJoint
//

///////////////////////////////////////

cMs3dJoint::cMs3dJoint()
 : flags(0)
{
   name[0] = 0;
   parentName[0] = 0;
}

///////////////////////////////////////

#ifdef _DEBUG
void cMs3dJoint::DebugPrint()
{
   DebugPrintf(NULL, 0,
      "Joint: %s\n"
      "   parent: %s\n"
      "   rotation: <%.3f, %.3f, %.3f>\n"
      "   position: <%.3f, %.3f, %.3f>\n"
      "   %d rotation keyframes\n"
      "   %d position keyframes\n",
      name, parentName, rotation[0], rotation[1], rotation[2],
      position[0], position[1], position[2],
      keyFramesRot.size(), keyFramesTrans.size());
}
#endif

///////////////////////////////////////

tResult cReadWriteOps<cMs3dJoint>::Read(IReader * pReader, cMs3dJoint * pJoint)
{
   Assert(pReader != NULL);
   Assert(pJoint != NULL);

   tResult result = E_FAIL;

   do
   {
      if (pReader->Read(&pJoint->flags, sizeof(pJoint->flags)) != S_OK)
         break;

      if (pReader->Read(pJoint->name, sizeof(pJoint->name)) != S_OK)
         break;

      if (pReader->Read(pJoint->parentName, sizeof(pJoint->parentName)) != S_OK)
         break;

      if (pReader->Read(pJoint->rotation, sizeof(pJoint->rotation)) != S_OK)
         break;

      if (pReader->Read(pJoint->position, sizeof(pJoint->position)) != S_OK)
         break;

      uint16 nKeyFramesRot, nKeyFramesTrans;

      if (pReader->Read(&nKeyFramesRot, sizeof(nKeyFramesRot)) != S_OK)
         break;
      if (pReader->Read(&nKeyFramesTrans, sizeof(nKeyFramesTrans)) != S_OK)
         break;

      pJoint->keyFramesRot.resize(nKeyFramesRot);
      pJoint->keyFramesTrans.resize(nKeyFramesTrans);

      if (pReader->Read(&pJoint->keyFramesRot[0], pJoint->keyFramesRot.size() * sizeof(ms3d_keyframe_rot_t)) != S_OK)
         break;
      if (pReader->Read(&pJoint->keyFramesTrans[0], pJoint->keyFramesTrans.size() * sizeof(ms3d_keyframe_pos_t)) != S_OK)
         break;

      result = S_OK;
   }
   while (0);

   return result;
}

///////////////////////////////////////////////////////////////////////////////
