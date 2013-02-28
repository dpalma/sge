///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ms3dmodel/ms3djoint.h"

#include "msLib.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dJoint
//

///////////////////////////////////////

cMs3dJoint::cMs3dJoint()
 : m_flags(0)
{
}

///////////////////////////////////////
// TODO: UNTESTED

cMs3dJoint::cMs3dJoint(msBone * pBone)
 : m_flags(pBone->nFlags)
{
   memcpy(m_name, pBone->szName, sizeof(m_name));
   memcpy(m_parentName, pBone->szParentName, sizeof(m_parentName));
   memcpy(rotation, pBone->Rotation, sizeof(rotation));
   memcpy(position, pBone->Position, sizeof(position));

   keyFramesRot.reserve(pBone->nNumRotationKeys);
   for (int i = 0; i < pBone->nNumRotationKeys; ++i)
   {
      sMs3dRotationKeyframe rk;
      rk.time = pBone->pRotationKeys[i].fTime;
      memcpy(rk.rotation, pBone->pRotationKeys[i].Rotation, sizeof(rk.rotation));
      keyFramesRot.push_back(rk);
   }

   keyFramesTrans.reserve(pBone->nNumPositionKeys);
   for (int i = 0; i < pBone->nNumPositionKeys; ++i)
   {
      sMs3dPositionKeyframe pk;
      pk.time = pBone->pRotationKeys[i].fTime;
      memcpy(pk.position, pBone->pPositionKeys[i].Position, sizeof(pk.position));
      keyFramesTrans.push_back(pk);
   }
}

///////////////////////////////////////

cMs3dJoint::cMs3dJoint(const cMs3dJoint & other)
 : m_flags(other.m_flags)
{
   memcpy(m_name, other.m_name, sizeof(m_name));
   memcpy(m_parentName, other.m_parentName, sizeof(m_parentName));
   memcpy(rotation, other.rotation, sizeof(rotation));
   memcpy(position, other.position, sizeof(position));
   keyFramesRot.insert(keyFramesRot.begin(), other.keyFramesRot.begin(), other.keyFramesRot.end());
   keyFramesTrans.insert(keyFramesTrans.begin(), other.keyFramesTrans.begin(), other.keyFramesTrans.end());
}

///////////////////////////////////////

cMs3dJoint::~cMs3dJoint()
{
}

///////////////////////////////////////

const cMs3dJoint & cMs3dJoint::operator =(const cMs3dJoint & other)
{
   m_flags = other.m_flags;
   memcpy(m_name, other.m_name, sizeof(m_name));
   memcpy(m_parentName, other.m_parentName, sizeof(m_parentName));
   memcpy(rotation, other.rotation, sizeof(rotation));
   memcpy(position, other.position, sizeof(position));
   keyFramesRot.insert(keyFramesRot.begin(), other.keyFramesRot.begin(), other.keyFramesRot.end());
   keyFramesTrans.insert(keyFramesTrans.begin(), other.keyFramesTrans.begin(), other.keyFramesTrans.end());
   return *this;
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

      if (pReader->Read(&pJoint->m_flags, sizeof(pJoint->m_flags)) != S_OK
         || pReader->Read(pJoint->m_name, sizeof(pJoint->m_name)) != S_OK
         || pReader->Read(pJoint->m_parentName, sizeof(pJoint->m_parentName)) != S_OK
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

      if (pReader->Read(&pJoint->keyFramesRot[0], nKeyFramesRot * sizeof(sMs3dRotationKeyframe)) != S_OK
         || pReader->Read(&pJoint->keyFramesTrans[0], nKeyFramesTrans * sizeof(sMs3dPositionKeyframe)) != S_OK)
      {
         break;
      }

      result = S_OK;
   }
   while (0);

   return result;
}


///////////////////////////////////////////////////////////////////////////////
