///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ms3dread.h"

#include "animation.h"
#include "matrix4.h"
#include "vec3.h"
#include "techstring.h"

#include <map>

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

      pJoint->keyFramesRot.resize(nKeyFramesRot);
      pJoint->keyFramesTrans.resize(nKeyFramesTrans);

      if (pReader->Read(&pJoint->keyFramesRot[0], nKeyFramesRot * sizeof(uint16)) != S_OK
         || pReader->Read(&pJoint->keyFramesTrans[0], nKeyFramesTrans * sizeof(uint16)) != S_OK)
      {
         break;
      }

      result = S_OK;
   }
   while (0);

   return result;
}


///////////////////////////////////////////////////////////////////////////////

tResult cReadWriteOps<sMs3dBoneInfo>::Read(IReader * pReader, sMs3dBoneInfo * pBoneInfo)
{
   Assert(pReader != NULL);
   Assert(pBoneInfo != NULL);

   byte flags; // SELECTED | DIRTY

   if (pReader->Read(&flags, sizeof(flags)) != S_OK
      || pReader->Read(pBoneInfo->name, sizeof(pBoneInfo->name)) != S_OK
      || pReader->Read(pBoneInfo->parentName, sizeof(pBoneInfo->parentName)) != S_OK
      || pReader->Read(pBoneInfo->rotation, sizeof(pBoneInfo->rotation)) != S_OK
      || pReader->Read(pBoneInfo->position, sizeof(pBoneInfo->position)) != S_OK)
   {
      return E_FAIL;
   }

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

static tResult ReadKeyFrames(IReader * pReader, uint * pnKeyFrames, sKeyFrame * pKeyFrames)
{
   Assert(pReader != NULL);
   Assert(pnKeyFrames != NULL);
   Assert(pKeyFrames != NULL);

   tResult result = E_FAIL;

   do
   {
      uint i;
      uint16 nKeyFramesRot, nKeyFramesTrans;

      if (pReader->Read(&nKeyFramesRot, sizeof(nKeyFramesRot)) != S_OK)
         break;
      if (pReader->Read(&nKeyFramesTrans, sizeof(nKeyFramesTrans)) != S_OK)
         break;

      if (nKeyFramesRot != nKeyFramesTrans)
      {
         DebugMsg2("Have %d rotation and %d translation key frames\n", nKeyFramesRot, nKeyFramesTrans);
         break;
      }

      uint nKeyFrames = Min(*pnKeyFrames, nKeyFramesRot);

      ms3d_keyframe_rot_t rotationKeys[MAX_KEYFRAMES];
      if (pReader->Read(rotationKeys, nKeyFramesRot * sizeof(ms3d_keyframe_rot_t)) != S_OK)
         break;

      ms3d_keyframe_pos_t translationKeys[MAX_KEYFRAMES];
      if (pReader->Read(translationKeys, nKeyFramesTrans * sizeof(ms3d_keyframe_pos_t)) != S_OK)
         break;

      for (i = 0; i < nKeyFrames; i++)
      {
         pKeyFrames[i].time = translationKeys[i].time;
         pKeyFrames[i].translation = tVec3(translationKeys[i].position);
         pKeyFrames[i].rotation = QuatFromEulerAngles(tVec3(rotationKeys[i].rotation));
      }

      *pnKeyFrames = nKeyFrames;

      result = S_OK;
   }
   while (0);

   return result;
}

///////////////////////////////////////////////////////////////////////////////

static void MatrixFromAngles(tVec3 angles, tMatrix4 * pMatrix)
{
   tMatrix4 rotX, rotY, rotZ, temp1, temp2;
   MatrixRotateX(Rad2Deg(angles.x), &rotX);
   MatrixRotateY(Rad2Deg(angles.y), &rotY);
   MatrixRotateZ(Rad2Deg(angles.z), &rotZ);
   temp1 = rotZ;
   temp1.Multiply(rotY, &temp2);
   temp2.Multiply(rotX, pMatrix);
}

///////////////////////////////////////////////////////////////////////////////

tResult ReadSkeleton(IReader * pReader, 
                     std::vector<sBoneInfo> * pBones,
                     std::vector<IKeyFrameInterpolator *> * pInterpolators)
{
   Assert(pReader != NULL);
   Assert(pBones != NULL);
   Assert(pInterpolators != NULL);

   tResult result = E_FAIL;

   do
   {
      float animationFPS;
      float currentTime;
      int nTotalFrames;
      uint16 nJoints;
      if (pReader->Read(&animationFPS, sizeof(animationFPS)) != S_OK
         || pReader->Read(&currentTime, sizeof(currentTime)) != S_OK
         || pReader->Read(&nTotalFrames, sizeof(nTotalFrames)) != S_OK
         || pReader->Read(&nJoints, sizeof(nJoints)) != S_OK)
      {
         break;
      }

      std::map<cStr, int> boneNames; // map bone names to indices

      std::vector<sMs3dBoneInfo> boneInfo(nJoints);

      pInterpolators->clear();

      uint i;
      for (i = 0; i < nJoints; i++)
      {
         if (pReader->Read(&boneInfo[i]) != S_OK)
            break;

         boneNames.insert(std::make_pair(cStr(boneInfo[i].name), i));

         sKeyFrame keyFrames[MAX_KEYFRAMES];
         uint nKeyFrames = _countof(keyFrames);
         if (ReadKeyFrames(pReader, &nKeyFrames, keyFrames) != S_OK)
         {
            break;
         }

         IKeyFrameInterpolator * pInterpolator = NULL;
         if (KeyFrameInterpolatorCreate(boneInfo[i].name, keyFrames, nKeyFrames, &pInterpolator) != S_OK)
         {
            SafeRelease(pInterpolator);
            break;
         }
         else
         {
            pInterpolators->push_back(pInterpolator);
         }
      }

      if (i < nJoints)
         break;

      pBones->resize(nJoints);

      for (i = 0; i < nJoints; i++)
      {
         strcpy((*pBones)[i].name, boneInfo[i].name);

         if (strlen(boneInfo[i].parentName) > 0)
         {
            std::map<cStr, int>::iterator n = boneNames.find(boneInfo[i].parentName);
            if (n != boneNames.end())
            {
               Assert(strcmp(boneInfo[n->second].name, boneInfo[i].parentName) == 0);
               (*pBones)[i].parentIndex = n->second;
            }
         }
         else
         {
            (*pBones)[i].parentIndex = -1;
         }

         tMatrix4 mt, mr;
         MatrixTranslate(boneInfo[i].position[0], boneInfo[i].position[1], boneInfo[i].position[2], &mt);
         MatrixFromAngles(tVec3(boneInfo[i].rotation), &mr);

         mt.Multiply(mr, &(*pBones)[i].localTransform);
      }

      result = S_OK;
   }
   while (0);

   return result;
}

///////////////////////////////////////////////////////////////////////////////
