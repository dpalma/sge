///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCENENODE_H
#define INCLUDED_SCENENODE_H

#include "vec3.h"
#include "quat.h"
#include "matrix4.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif

class cSceneNode;
class cSceneNodeVisitor;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneNode
//

enum eSceneNodeCapabilities
{
   kSNC_Pickable = 1 << 0,
};

enum eSceneNodeState
{
   kSNS_Selected = 1 << 0,
};

class cSceneNode
{
   cSceneNode(const cSceneNode &);
   const cSceneNode & operator =(const cSceneNode &);

public:
   cSceneNode();
   virtual ~cSceneNode();

   const tVec3 & GetLocalTranslation() const;
   void SetLocalTranslation(const tVec3 & translation);

   const tQuat & GetLocalRotation() const;
   void SetLocalRotation(const tQuat & rotation);

   const tMatrix4 & GetLocalTransform() const;

   const tVec3 & GetWorldTranslation() const;
   const tQuat & GetWorldRotation() const;
   const tMatrix4 & GetWorldTransform() const;

   bool IsPickable() const;
   void SetPickable(bool pickable);

   bool IsSelected() const;
   void SetSelected(bool selected);

   bool AddChild(cSceneNode * pNode);
   bool RemoveChild(cSceneNode * pNode);

   void Traverse(cSceneNodeVisitor * pVisitor);

   virtual void Render();

   virtual float GetBoundingSphereRadius() const { return 0; }

   // @TODO: should this be pushed further down the class hierarchy?
   virtual void Hit() {}
   virtual void ClearHitState() {}

protected:
   const cSceneNode * GetParent() const;

   typedef std::list<cSceneNode *> tChildren;
   tChildren m_children;

private:
   cSceneNode * m_pParent;

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

   uint m_caps;
   uint m_state;
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

///////////////////////////////////////

inline bool cSceneNode::IsPickable() const
{
   return (m_caps & kSNC_Pickable);
}

///////////////////////////////////////

inline void cSceneNode::SetPickable(bool pickable)
{
   if (pickable)
      m_caps |= kSNC_Pickable;
   else
      m_caps &= ~kSNC_Pickable;
}

///////////////////////////////////////

inline bool cSceneNode::IsSelected() const
{
   return (m_state & kSNS_Selected);
}

///////////////////////////////////////

inline void cSceneNode::SetSelected(bool selected)
{
   uint newState = m_state;
   if (selected)
      newState |= kSNS_Selected;
   else
      newState &= ~kSNS_Selected;
   m_state = newState;
}

///////////////////////////////////////

inline const cSceneNode * cSceneNode::GetParent() const
{
   return m_pParent;
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
