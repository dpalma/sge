///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCENEGROUP_H
#define INCLUDED_SCENEGROUP_H

#include "scenenode.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneGroup
//

class cSceneGroup : public cSceneNode
{
   cSceneGroup(const cSceneGroup &);
   const cSceneGroup & operator =(const cSceneGroup &);

public:
   cSceneGroup();
   virtual ~cSceneGroup();

   void AddChild(cSceneNode * pNode);
   bool RemoveChild(cSceneNode * pNode);

   void Traverse(cSceneNodeVisitor * pVisitor);

   virtual void Update(float timeDelta);

   virtual void Render();

protected:
   typedef std::list<cSceneNode *> tChildren;

   tChildren m_children;
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneTransformGroup
//

class cSceneTransformGroup : public cSceneGroup
{
   cSceneTransformGroup(const cSceneTransformGroup &);
   const cSceneTransformGroup & operator =(const cSceneTransformGroup &);

public:
   cSceneTransformGroup();
   virtual ~cSceneTransformGroup();

   const tVec3 & GetTranslation() const;
   void SetTranslation(const tVec3 & translation);

   const tQuat & GetRotation() const;
   void SetRotation(const tQuat & rotation);

   const sMatrix4 & GetTransform() const;

//   void CalcWorldRT(tQuat * pR, tVec3 * pT) const;
//
//   const sMatrix4 & GetWorldTransform() const;

   virtual void Render();

private:
   tVec3 m_translation;
   tQuat m_rotation;

   mutable bool m_bUpdateLocalTransform;
   mutable sMatrix4 m_localTransform;

//   mutable bool m_bUpdateWorldTransform;
//   mutable sMatrix4 m_worldTransform;
};

///////////////////////////////////////

inline const tVec3 & cSceneTransformGroup::GetTranslation() const
{
   return m_translation;
}

///////////////////////////////////////

inline void cSceneTransformGroup::SetTranslation(const tVec3 & translation)
{
   m_translation = translation;
   m_bUpdateLocalTransform = true;
//   m_bUpdateWorldTransform = true;
}

///////////////////////////////////////

inline const tQuat & cSceneTransformGroup::GetRotation() const
{
   return m_rotation;
}

///////////////////////////////////////

inline void cSceneTransformGroup::SetRotation(const tQuat & rotation)
{
   m_rotation = rotation;
   m_bUpdateLocalTransform = true;
//   m_bUpdateWorldTransform = true;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCENEGROUP_H
