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

F_DECLARE_INTERFACE(ISkeleton);

typedef std::vector<tMatrix4> tMatrices;

class cBone;

////////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ISkeleton
//

interface ISkeleton : IUnknown
{
   virtual int GetBoneCount() const = 0;
   virtual const char * GetBoneName(int index) const = 0;
   virtual const tMatrix4 & GetBoneWorldTransform(int index) const = 0;

   // TODO: Pull out animation-related methods into separate interfaces
   virtual void GetBoneMatrices(float percent, tMatrices * pBoneMatrices) const = 0;
   virtual tResult GetInterpolator(int index, IKeyFrameInterpolator * * ppInterpolator) const = 0;
};

///////////////////////////////////////

tResult SkeletonCreate(const cBone * pBones, uint nBones, 
   IKeyFrameInterpolator * * pInterpolators, uint nInterpolators,
   ISkeleton * * ppSkeleton);


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

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SKELETON_H
