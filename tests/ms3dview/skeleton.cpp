///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "skeleton.h"

#include "animation.h"

#include <algorithm>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cBone
//

///////////////////////////////////////

cBone::cBone()
 : m_index(-1),
   m_parentIndex(-1),
   m_pParent(NULL),
   m_bHaveWorldTransform(false)
{
}

///////////////////////////////////////

cBone::cBone(const cBone & other)
{
   operator =(other);
}

///////////////////////////////////////

const cBone & cBone::operator =(const cBone & other)
{
   m_name = other.m_name;
   m_index = other.m_index;
   m_parentIndex = other.m_parentIndex;
   m_pParent = other.m_pParent;
   m_children.resize(other.m_children.size());
   std::copy(other.m_children.begin(), other.m_children.end(), m_children.begin());
   m_localTransform = other.m_localTransform;
   m_worldTransform = other.m_worldTransform;
   m_bHaveWorldTransform = other.m_bHaveWorldTransform;
   return *this;
}

///////////////////////////////////////

const tMatrix4 & cBone::GetWorldTransform() const
{
   if (!m_bHaveWorldTransform)
   {
      const cBone * pParent = GetParent();

      if (pParent != NULL)
      {
         m_worldTransform = pParent->GetWorldTransform() * GetLocalTransform();
      }
      else
      {
         m_worldTransform = GetLocalTransform();
      }

      m_bHaveWorldTransform = true;
   }

   return m_worldTransform;
}

///////////////////////////////////////

bool cBone::AddChild(const cBone * pChild)
{
   if (pChild != NULL)
   {
      tChildren::iterator iter;
      for (iter = m_children.begin(); iter != m_children.end(); iter++)
      {
         if ((*iter == pChild) || (strcmp((*iter)->GetName(), pChild->GetName()) == 0))
         {
            return false;
         }
      }

      m_children.push_back(pChild);

      const_cast<cBone *>(pChild)->m_pParent = this;
      const_cast<cBone *>(pChild)->m_parentIndex = GetIndex();
      const_cast<cBone *>(pChild)->m_bHaveWorldTransform = false;

      return true;
   }

   return false;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSkeleton
//

cSkeleton::cSkeleton()
{
}

cSkeleton::~cSkeleton()
{
   Reset();
}

bool cSkeleton::Create(const cBone * pBones, uint nBones, 
                       IKeyFrameInterpolator * * pInterpolators, uint nInterpolators)
{
   if ((pInterpolators != NULL) && (nInterpolators > 0))
   {
      m_interpolators.resize(nInterpolators);
      for (uint i = 0; i < nBones; i++)
      {
         m_interpolators[i] = pInterpolators[i];
         m_interpolators[i]->AddRef();
      }
   }

   if ((pBones != NULL) && (nBones > 0))
   {
      m_bones.resize(nBones);
      for (uint i = 0; i < nBones; i++)
      {
         m_bones[i] = pBones[i];
      }

      SetupJoints();

      return true;
   }

   return false;
}

void cSkeleton::GetBoneMatrices(float percent, tMatrices * pBoneMatrices) const
{
   Assert(percent >= 0 && percent <= 1);
   Assert(pBoneMatrices != NULL);
   Assert(pBoneMatrices->size() == GetBoneCount());

   for (int i = 0; i < GetBoneCount(); i++)
   {
      tQuat rotation;
      tVec3 translation;

      IKeyFrameInterpolator * pInterpolator = m_interpolators[i];

      if (pInterpolator->Interpolate(
         percent * pInterpolator->GetPeriod(),
         NULL, &rotation, &translation) == S_OK)
      {
         tMatrix4 mt;
         MatrixTranslate(translation.x, translation.y, translation.z, &mt);

         tMatrix4 mr;
         rotation.ToMatrix(&mr);

         tMatrix4 temp = mt * mr;

         const cBone & bone = GetBone(i);

         tMatrix4 mf = bone.GetLocalTransform() * temp;

         if (bone.GetParentIndex() < 0)
         {
            temp = mf;
         }
         else
         {
            temp = (*pBoneMatrices)[bone.GetParentIndex()] * mf;
         }

         (*pBoneMatrices)[i] = temp;
      }
   }
}

tResult cSkeleton::GetInterpolator(int index, IKeyFrameInterpolator * * ppInterpolator) const
{
   if (index >= 0 && index < m_interpolators.size() && ppInterpolator != NULL)
   {
      *ppInterpolator = m_interpolators[index];
      (*ppInterpolator)->AddRef();
      return S_OK;
   }
   return E_FAIL;
}

void cSkeleton::Reset()
{
   m_bones.clear();
   std::for_each(m_interpolators.begin(), m_interpolators.end(), CTInterfaceMethodRef(&IUnknown::Release));
   m_interpolators.clear();
}

void cSkeleton::SetupJoints()
{
   tBones::iterator iter;
   for (iter = m_bones.begin(); iter != m_bones.end(); iter++)
   {
      if (iter->GetParentIndex() > -1)
      {
         m_bones[iter->GetParentIndex()].AddChild(iter);
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
