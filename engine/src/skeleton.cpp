///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "skeleton.h"

#include "animation.h"
#include "str.h"

#include <algorithm>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cBone
//

class cBone
{
   friend class cSkeleton;

public:
   cBone();
   cBone(const cBone & other);
   const cBone & operator =(const cBone & other);
   const cBone & operator =(const sBoneInfo & boneInfo);

   const char * GetName() const;
   void SetName(const char * pszName);

   // The index of this bone in its containing skeleton
   int GetIndex() const;
   void SetIndex(int index);

   int GetParentIndex() const;
   void SetParentIndex(int parentIndex);

   const tMatrix4 & GetLocalTransform() const;
   void SetLocalTransform(const tMatrix4 & matrix);

   const tMatrix4 & GetWorldTransform() const;

private:
   const cBone * GetParent() const;

   bool AddChild(const cBone * pChild);

   cStr m_name;
   int m_index;
   int m_parentIndex;
   const cBone * m_pParent;
   typedef std::vector<const cBone *> tChildren;
   tChildren m_children;
   tMatrix4 m_localTransform;
   mutable tMatrix4 m_worldTransform;
   mutable bool m_bHaveWorldTransform;
};

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

const cBone & cBone::operator =(const sBoneInfo & boneInfo)
{
   m_name = boneInfo.name;
   m_index = -1;
   m_parentIndex = boneInfo.parentIndex;
   m_pParent = NULL;
   m_children.clear();
   m_localTransform = boneInfo.localTransform;
   m_bHaveWorldTransform = false;
   return *this;
}

///////////////////////////////////////

inline const char * cBone::GetName() const
{
   return m_name;
}

///////////////////////////////////////

inline void cBone::SetName(const char * pszName)
{
   m_name = (pszName != NULL) ? pszName : "";
}

///////////////////////////////////////

inline int cBone::GetIndex() const
{
   return m_index;
}

///////////////////////////////////////

inline void cBone::SetIndex(int index)
{
   m_index = index;
}

///////////////////////////////////////

inline int cBone::GetParentIndex() const
{
   return m_parentIndex;
}

///////////////////////////////////////

inline void cBone::SetParentIndex(int parentIndex)
{
   m_parentIndex = parentIndex;
}

///////////////////////////////////////

inline const tMatrix4 & cBone::GetLocalTransform() const
{
   return m_localTransform;
}

///////////////////////////////////////

inline void cBone::SetLocalTransform(const tMatrix4 & matrix)
{
   m_localTransform = matrix;
   m_bHaveWorldTransform = false;
}

///////////////////////////////////////

inline const cBone * cBone::GetParent() const
{
   return m_pParent;
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

class cSkeleton : public cComObject<IMPLEMENTS(ISkeleton)>
{
   cSkeleton(const cSkeleton &); // private, un-implemented
   const cSkeleton & operator =(const cSkeleton &); // private, un-implemented

public:
   cSkeleton();
   ~cSkeleton();

   bool Create(const sBoneInfo * pBones, uint nBones, 
      IKeyFrameInterpolator * * pInterpolators, uint nInterpolators);

   int GetBoneCount() const;

   const char * GetBoneName(int index) const;
   const tMatrix4 & GetBoneWorldTransform(int index) const;

   void GetBoneMatrices(float percent, tMatrices * pBoneMatrices) const;

   tResult GetInterpolator(int index, IKeyFrameInterpolator * * ppInterpolator) const;

private:
   const cBone & GetBone(int index) const;

   void SetupJoints();

   typedef std::vector<cBone> tBones;
   tBones m_bones;

   typedef std::vector<IKeyFrameInterpolator *> tInterpolators;
   tInterpolators m_interpolators;
};

///////////////////////////////////////

cSkeleton::cSkeleton()
{
}

///////////////////////////////////////

cSkeleton::~cSkeleton()
{
   m_bones.clear();
   std::for_each(m_interpolators.begin(), m_interpolators.end(), CTInterfaceMethodRef(&IUnknown::Release));
   m_interpolators.clear();
}

///////////////////////////////////////

bool cSkeleton::Create(const sBoneInfo * pBones, uint nBones, 
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
         m_bones[i].SetIndex(i);
      }

      SetupJoints();

      return true;
   }

   return false;
}

///////////////////////////////////////

inline int cSkeleton::GetBoneCount() const
{
   return m_bones.size();
}

///////////////////////////////////////

inline const char * cSkeleton::GetBoneName(int index) const
{
	return GetBone(index).GetName();
}

///////////////////////////////////////

inline const tMatrix4 & cSkeleton::GetBoneWorldTransform(int index) const
{
	return GetBone(index).GetWorldTransform();
}

///////////////////////////////////////

inline const cBone & cSkeleton::GetBone(int index) const
{
   return m_bones[index];
}

///////////////////////////////////////

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

///////////////////////////////////////

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

///////////////////////////////////////

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
//
// INTERFACE: ISkeleton
//

///////////////////////////////////////

tResult SkeletonCreate(const sBoneInfo * pBones, uint nBones, 
   IKeyFrameInterpolator * * pInterpolators, uint nInterpolators,
   ISkeleton * * ppSkeleton)
{
   if (ppSkeleton != NULL && pBones != NULL && nBones > 0)
   {
      cSkeleton * pSkeleton = new cSkeleton;
      if (!pSkeleton->Create(pBones, nBones, pInterpolators, nInterpolators))
      {
         delete pSkeleton;
         pSkeleton = NULL;
      }
      *ppSkeleton = static_cast<ISkeleton *>(pSkeleton);
      return (*ppSkeleton) != NULL ? S_OK : E_FAIL;
   }
   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////
