///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCENENODE_H
#define INCLUDED_SCENENODE_H

#include "vec3.h"
#include "quat.h"
#include "matrix4.h"
#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IRenderDevice);

class cSceneNode;
class cSceneNodeVisitor;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneNode
//

class cSceneNode
{
   cSceneNode(const cSceneNode &);
   const cSceneNode & operator =(const cSceneNode &);

public:
   cSceneNode();
   virtual ~cSceneNode() = 0;

   const cSceneNode * GetParent() const;
   void SetParent(cSceneNode * pParent);

   virtual void Traverse(cSceneNodeVisitor * pVisitor);

   const tVec3 & GetTranslation() const;
   void SetTranslation(const tVec3 & translation);

   const tQuat & GetRotation() const;
   void SetRotation(const tQuat & rotation);

   void CalcWorldRT(tQuat * pR, tVec3 * pT) const;

   const sMatrix4 & GetWorldTransform() const;

   virtual void Update(float timeDelta);

   virtual void Render();

   // @HACK: need a better, more flexible bounding volume solution
   virtual float GetBoundingSphereRadius() const { return 0; }

   // @TODO: should this be pushed further down the class hierarchy?
   virtual void Hit() {}
   virtual void ClearHitState() {}

private:
   cSceneNode * m_pParent;

   tVec3 m_translation;
   tQuat m_rotation;

   mutable bool m_bUpdateWorldTransform;
   mutable sMatrix4 m_world; // world transformation matrix
};

///////////////////////////////////////

inline const cSceneNode * cSceneNode::GetParent() const
{
   return m_pParent;
}

///////////////////////////////////////

inline const tVec3 & cSceneNode::GetTranslation() const
{
   return m_translation;
}

///////////////////////////////////////

inline void cSceneNode::SetTranslation(const tVec3 & translation)
{
   m_translation = translation;
   m_bUpdateWorldTransform = true;
}

///////////////////////////////////////

inline const tQuat & cSceneNode::GetRotation() const
{
   return m_rotation;
}

///////////////////////////////////////

inline void cSceneNode::SetRotation(const tQuat & rotation)
{
   m_rotation = rotation;
   m_bUpdateWorldTransform = true;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneNodeVisitor
//

class cSceneNodeVisitor
{
public:
   virtual ~cSceneNodeVisitor() = 0;

   virtual void VisitSceneNode(cSceneNode * pNode) = 0;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCENENODE_H
