///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ms3dread.h"
#include "ms3d.h"

#include "animation.h"
#include "matrix4.h"
#include "vec3.h"
#include "str.h"

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

static tResult ReadKeyFrames(IReader * pReader, 
                             std::vector<sKeyFrameVec3> * pTranslationFrames,
                             std::vector<sKeyFrameQuat> * pRotationFrames)
{
   Assert(pReader != NULL);
   Assert(pTranslationFrames != NULL);
   Assert(pRotationFrames != NULL);

   tResult result = E_FAIL;

   do
   {
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

      std::vector<ms3d_keyframe_rot_t> rotationKeys(nKeyFramesRot);
      if (pReader->Read(&rotationKeys[0], rotationKeys.size() * sizeof(ms3d_keyframe_rot_t)) != S_OK)
         break;

      std::vector<ms3d_keyframe_pos_t> translationKeys(nKeyFramesTrans);
      if (pReader->Read(&translationKeys[0], translationKeys.size() * sizeof(ms3d_keyframe_pos_t)) != S_OK)
         break;

      pTranslationFrames->resize(nKeyFramesTrans);
      for (unsigned i = 0; i < nKeyFramesTrans; i++)
      {
         (*pTranslationFrames)[i].time = translationKeys[i].time;
         (*pTranslationFrames)[i].value = tVec3(translationKeys[i].position);
      }

      pRotationFrames->resize(nKeyFramesRot);
      for (i = 0; i < nKeyFramesRot; i++)
      {
         (*pRotationFrames)[i].time = rotationKeys[i].time;
         (*pRotationFrames)[i].value = QuatFromEulerAngles(tVec3(rotationKeys[i].rotation));
      }

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
   temp2 = temp1 * rotY;
   *pMatrix = temp2 * rotX;
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

      pInterpolators->resize(nJoints);

      uint i;
      for (i = 0; i < nJoints; i++)
      {
         if (pReader->Read(&boneInfo[i]) != S_OK)
            break;

         boneNames.insert(std::make_pair(cStr(boneInfo[i].name), i));

         std::vector<sKeyFrameVec3> translationFrames;
         std::vector<sKeyFrameQuat> rotationFrames;
         if (ReadKeyFrames(pReader, &translationFrames, &rotationFrames) != S_OK)
         {
            break;
         }

         IKeyFrameInterpolator * pInterpolator = NULL;
         if (KeyFrameInterpolatorCreate(boneInfo[i].name, NULL, 0,
            &rotationFrames[0], rotationFrames.size(),
            &translationFrames[0], rotationFrames.size(),
            &pInterpolator) != S_OK)
         {
            SafeRelease(pInterpolator);
            break;
         }

         (*pInterpolators)[i] = pInterpolator;
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

         tMatrix4 mt, mr;
         MatrixTranslate(boneInfo[i].position[0], boneInfo[i].position[1], boneInfo[i].position[2], &mt);
         MatrixFromAngles(tVec3(boneInfo[i].rotation), &mr);

         (*pBones)[i].localTransform = mt * mr;
      }

      result = S_OK;
   }
   while (0);

   return result;
}

///////////////////////////////////////////////////////////////////////////////
