///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SKELETON_H
#define INCLUDED_SKELETON_H

#include "comtools.h"
#include "matrix4.h"
#include "str.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IKeyFrameInterpolator);

typedef std::vector<tMatrix4> tMatrices;

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


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSkeleton
//

class cSkeleton
{
   cSkeleton(const cSkeleton &); // private, un-implemented
   const cSkeleton & operator =(const cSkeleton &); // private, un-implemented

public:
   cSkeleton();
   ~cSkeleton();

   bool Create(const cBone * pBones, uint nBones, 
      IKeyFrameInterpolator * * pInterpolators, uint nInterpolators);

   int GetBoneCount() const;
   const cBone & GetBone(int index) const;

   void GetBoneMatrices(float percent, tMatrices * pBoneMatrices) const;

   tResult GetInterpolator(int index, IKeyFrameInterpolator * * ppInterpolator) const;

   void Reset();

private:
   void SetupJoints();

   typedef std::vector<cBone> tBones;
   tBones m_bones;

   typedef std::vector<IKeyFrameInterpolator *> tInterpolators;
   tInterpolators m_interpolators;
};

///////////////////////////////////////

inline int cSkeleton::GetBoneCount() const
{
   return m_bones.size();
}

///////////////////////////////////////

inline const cBone & cSkeleton::GetBone(int index) const
{
   return m_bones[index];
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SKELETON_H
