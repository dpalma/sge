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

   bool Create(const sBoneInfo * pBones, uint nBones);

   int GetBoneCount() const;

   const char * GetBoneName(int index) const;
   const tMatrix4 & GetBoneWorldTransform(int index) const;

   virtual tResult SetAnimation(IKeyFrameAnimation * pAnimation);
   virtual tResult GetAnimation(IKeyFrameAnimation * * ppAnimation);

   void GetBoneMatrices(float percent, tMatrices * pBoneMatrices) const;

private:
   const cBone & GetBone(int index) const;

   void SetBoneParentChildRelationships();

   typedef std::vector<cBone> tBones;
   tBones m_bones;

   cAutoIPtr<IKeyFrameAnimation> m_pAnimation;
};

///////////////////////////////////////

cSkeleton::cSkeleton()
{
}

///////////////////////////////////////

cSkeleton::~cSkeleton()
{
   m_bones.clear();
}

///////////////////////////////////////

bool cSkeleton::Create(const sBoneInfo * pBones, uint nBones)
{
   Assert(m_bones.empty());

   if ((pBones != NULL) && (nBones > 0))
   {
      m_bones.resize(nBones);
      for (uint i = 0; i < nBones; i++)
      {
         m_bones[i] = pBones[i];
         m_bones[i].SetIndex(i);
      }

      SetBoneParentChildRelationships();

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

tResult cSkeleton::SetAnimation(IKeyFrameAnimation * pAnimation)
{
   SafeRelease(m_pAnimation);
   m_pAnimation = pAnimation;
   if (pAnimation != NULL)
   {
      pAnimation->AddRef();
   }
   return S_OK;
}

///////////////////////////////////////

tResult cSkeleton::GetAnimation(IKeyFrameAnimation * * ppAnimation)
{
   if (ppAnimation != NULL)
   {
      if (!m_pAnimation)
      {
         return S_FALSE;
      }
      else
      {
         *ppAnimation = m_pAnimation;
         (*ppAnimation)->AddRef();
         return S_OK;
      }
   }
   return E_FAIL;
}

///////////////////////////////////////

void cSkeleton::GetBoneMatrices(float percent, tMatrices * pBoneMatrices) const
{
   Assert(percent >= 0 && percent <= 1);
   Assert(pBoneMatrices != NULL);
   Assert(pBoneMatrices->size() == GetBoneCount());

   if (!m_pAnimation)
   {
      return;
   }

   for (int i = 0; i < GetBoneCount(); i++)
   {
      cAutoIPtr<IKeyFrameInterpolator> pInterp;
      if (m_pAnimation->GetInterpolator(i, &pInterp) == S_OK)
      {
         sKeyFrame frame;
         if (pInterp->Interpolate(percent * pInterp->GetPeriod(), &frame) == S_OK)
         {
            tMatrix4 mt, mr;

            frame.rotation.ToMatrix(&mr);
            MatrixTranslate(frame.translation.x, frame.translation.y, frame.translation.z, &mt);

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
}

///////////////////////////////////////

void cSkeleton::SetBoneParentChildRelationships()
{
   tBones::iterator iter;
   for (iter = m_bones.begin(); iter != m_bones.end(); iter++)
   {
      cBone & bone = *iter;
      if (bone.GetParentIndex() > -1)
      {
         m_bones[bone.GetParentIndex()].AddChild(&bone);
      }
   }
}


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ISkeleton
//

///////////////////////////////////////

tResult SkeletonCreate(const sBoneInfo * pBones, uint nBones, 
   ISkeleton * * ppSkeleton)
{
   if (ppSkeleton != NULL && pBones != NULL && nBones > 0)
   {
      cSkeleton * pSkeleton = new cSkeleton;
      if (!pSkeleton->Create(pBones, nBones))
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
