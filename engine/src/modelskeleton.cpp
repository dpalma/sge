///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "modelskeleton.h"

#include "matrix3.h"

#include <algorithm>
#include <cfloat>
#include <stack>

#include "dbgalloc.h" // must be last header

extern cRand g_engineRand;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelSkeleton
//

///////////////////////////////////////

cModelSkeleton::cModelSkeleton()
{
}

///////////////////////////////////////

cModelSkeleton::cModelSkeleton(const cModelSkeleton & other)
{
   operator =(other);
}

///////////////////////////////////////

cModelSkeleton::cModelSkeleton(const tModelJoints & joints)
 : m_joints(joints.size())
{
   if (!joints.empty())
   {
      std::copy(joints.begin(), joints.end(), m_joints.begin());
   }
}

///////////////////////////////////////

cModelSkeleton::~cModelSkeleton()
{
   std::multimap<eModelAnimationType, IModelAnimation *>::iterator iter = m_anims.begin();
   for (; iter != m_anims.end(); iter++)
   {
      iter->second->Release();
   }
}

///////////////////////////////////////

const cModelSkeleton & cModelSkeleton::operator =(const cModelSkeleton & other)
{
   m_joints.resize(other.m_joints.size());
   std::copy(other.m_joints.begin(), other.m_joints.end(), m_joints.begin());
   tAnimMap::const_iterator iter = other.m_anims.begin();
   for (; iter != other.m_anims.end(); iter++)
   {
      m_anims.insert(std::make_pair(iter->first, CTAddRef(iter->second)));
   }
   return *this;
}

///////////////////////////////////////

tResult cModelSkeleton::GetJointCount(size_t * pJointCount) const
{
   if (pJointCount == NULL)
   {
      return E_POINTER;
   }
   *pJointCount = m_joints.size();
   return S_OK;
}

///////////////////////////////////////

tResult cModelSkeleton::GetJoint(size_t iJoint, sModelJoint * pJoint) const
{
   if (iJoint >= m_joints.size())
   {
      return E_INVALIDARG;
   }
   if (pJoint == NULL)
   {
      return E_POINTER;
   }
   *pJoint = m_joints[iJoint];
   return S_OK;
}

///////////////////////////////////////

tResult cModelSkeleton::GetBindMatrices(size_t nMaxMatrices, tMatrix4 * pMatrices) const
{
   if (nMaxMatrices < m_joints.size())
   {
      return E_INVALIDARG;
   }

   if (pMatrices == NULL)
   {
      return E_POINTER;
   }

   uint i;
   int iRootJoint = -1;
   std::multimap<int, int> jointChildMap;
   tModelJoints::const_iterator iter = m_joints.begin();
   for (i = 0; iter != m_joints.end(); iter++, i++)
   {
      int iParent = iter->parentIndex;
      if (iParent >= 0)
      {
         jointChildMap.insert(std::make_pair(iParent, i));
      }
      else
      {
         Assert(iRootJoint == -1);
         iRootJoint = i;
      }
   }

   if (iRootJoint < 0)
   {
      ErrorMsg("Bad set of joints: no root\n");
      return E_FAIL;
   }

   std::vector<tQuat> absoluteRotations(m_joints.size(), tQuat::GetMultIdentity());
   std::vector<tVec3> absoluteTranslations(m_joints.size(), tVec3(0,0,0));

   std::stack<int> s;
   s.push(iRootJoint);
   while (!s.empty())
   {
      int iJoint = s.top();
      s.pop();

      int iParent = m_joints[iJoint].parentIndex;
      if (iParent == -1)
      {
         absoluteRotations[iJoint] = m_joints[iJoint].localRotation;
         absoluteTranslations[iJoint] = m_joints[iJoint].localTranslation;
      }
      else
      {
         absoluteRotations[iJoint] = absoluteRotations[iParent] * m_joints[iJoint].localRotation;
         absoluteTranslations[iJoint] = absoluteTranslations[iParent] + m_joints[iJoint].localTranslation;
      }

      std::multimap<int, int>::iterator iter = jointChildMap.lower_bound(iJoint);
      std::multimap<int, int>::iterator end = jointChildMap.upper_bound(iJoint);
      for (; iter != end; iter++)
      {
         s.push(iter->second);
      }
   }

   for (i = 0; i < m_joints.size(); i++)
   {
      tQuat invRot = absoluteRotations[i].Inverse();
      tVec3 invTrans = -absoluteTranslations[i];
      tMatrix3 mr;
      invRot.ToMatrix(&mr);
      tMatrix4 temp;
      temp.m00 = mr.m00;
      temp.m10 = mr.m10;
      temp.m20 = mr.m20;
      temp.m30 = 0;
      temp.m01 = mr.m01;
      temp.m11 = mr.m11;
      temp.m21 = mr.m21;
      temp.m31 = 0;
      temp.m02 = mr.m02;
      temp.m12 = mr.m12;
      temp.m22 = mr.m22;
      temp.m32 = 0;
      temp.m03 = invTrans.x;
      temp.m13 = invTrans.y;
      temp.m23 = invTrans.z;
      temp.m33 = 1;
      pMatrices[i] = temp;
   }

   return S_OK;
}

///////////////////////////////////////

tResult cModelSkeleton::AddAnimation(eModelAnimationType type,
                                     IModelAnimation * pAnim)
{
   if (pAnim == NULL)
   {
      return E_POINTER;
   }
   m_anims.insert(std::make_pair(type, CTAddRef(pAnim)));
   return S_OK;
}

///////////////////////////////////////

tResult cModelSkeleton::GetAnimation(eModelAnimationType type,
                                     IModelAnimation * * ppAnim) const
{
   if (ppAnim == NULL)
   {
      return E_POINTER;
   }
   
   tAnimMap::const_iterator first = m_anims.lower_bound(type);
   if (first == m_anims.end())
   {
      return S_FALSE;
   }

   tAnimMap::const_iterator last = m_anims.upper_bound(type);

   if (first == last)
   {
      *ppAnim = CTAddRef(first->second);
      return S_OK;
   }
   else
   {
      tAnimMap::difference_type nAnims = 0;
      tAnimMap::const_iterator iter = first;
      for (; iter != last; iter++)
      {
         nAnims++;
      }
      uint i, iAnim = g_engineRand.Next() % nAnims;
      for (i = 0, iter = first; iter != last; i++, iter++)
      {
         if (i == iAnim)
         {
            break;
         }
      }
      *ppAnim = CTAddRef(iter->second);
      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////

tResult ModelSkeletonCreate(const tModelJoints & joints, IModelSkeleton * * ppSkeleton)
{
   if (ppSkeleton == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IModelSkeleton> pSkeleton(static_cast<IModelSkeleton*>(new cModelSkeleton(joints)));
   if (!pSkeleton)
   {
      return E_OUTOFMEMORY;
   }

   return pSkeleton.GetPointer(ppSkeleton);
}

///////////////////////////////////////////////////////////////////////////////
