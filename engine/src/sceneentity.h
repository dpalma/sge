///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCENENODE_H
#define INCLUDED_SCENENODE_H

#include "sceneapi.h"

#include "vec3.h"
#include "quat.h"
#include "matrix4.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneNode
//

class cSceneNode : public cComObject<IMPLEMENTS(ISceneEntity)>
{
   cSceneNode(const cSceneNode &);
   const cSceneNode & operator =(const cSceneNode &);

public:
   cSceneNode();
   virtual ~cSceneNode();

   virtual ISceneEntity * AccessParent();
   virtual tResult SetParent(ISceneEntity * pEntity);
   virtual tResult IsChild(ISceneEntity * pEntity) const;
   virtual tResult AddChild(ISceneEntity * pEntity);
   virtual tResult RemoveChild(ISceneEntity * pEntity);

   virtual const tVec3 & GetLocalTranslation() const;
   virtual void SetLocalTranslation(const tVec3 & translation);
   virtual const tQuat & GetLocalRotation() const;
   virtual void SetLocalRotation(const tQuat & rotation);
   virtual const tMatrix4 & GetLocalTransform() const;

   virtual const tVec3 & GetWorldTranslation() const;
   virtual const tQuat & GetWorldRotation() const;
   virtual const tMatrix4 & GetWorldTransform() const;

   virtual void Render();
   virtual float GetBoundingRadius() const;

private:
   ISceneEntity * m_pParent;
   tSceneEntityList m_children;

   tVec3 m_localTranslation;
   tQuat m_localRotation;

   mutable bool m_bHaveLocalTransform;
   mutable tMatrix4 m_localTransform;

   mutable tVec3 m_worldTranslation;
   mutable bool m_bHaveWorldTranslation;

   mutable tQuat m_worldRotation;
   mutable bool m_bHaveWorldRotation;

   mutable bool m_bHaveWorldTransform;
   mutable tMatrix4 m_worldTransform;
};

///////////////////////////////////////

inline const tVec3 & cSceneNode::GetLocalTranslation() const
{
   return m_localTranslation;
}

///////////////////////////////////////

inline void cSceneNode::SetLocalTranslation(const tVec3 & translation)
{
   m_localTranslation = translation;
   m_bHaveLocalTransform = false;
   m_bHaveWorldTranslation = false;
   m_bHaveWorldTransform = false;
}

///////////////////////////////////////

inline const tQuat & cSceneNode::GetLocalRotation() const
{
   return m_localRotation;
}

///////////////////////////////////////

inline void cSceneNode::SetLocalRotation(const tQuat & rotation)
{
   m_localRotation = rotation;
   m_bHaveLocalTransform = false;
   m_bHaveWorldRotation = false;
   m_bHaveWorldTransform = false;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCENENODE_H
