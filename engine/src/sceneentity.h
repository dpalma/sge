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
class cBoundingVolume;

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

   const tVec3 & GetTranslation() const;
   void SetTranslation(const tVec3 & translation);

   const tQuat & GetRotation() const;
   void SetRotation(const tQuat & rotation);

   const sMatrix4 & GetTransform() const;

   bool IsPickable() const;
   void SetPickable(bool pickable);

   bool IsSelected() const;
   void SetSelected(bool selected);

   bool AddChild(cSceneNode * pNode);
   bool RemoveChild(cSceneNode * pNode);

   virtual void Traverse(cSceneNodeVisitor * pVisitor);

   virtual void Render();

   virtual const cBoundingVolume * GetBoundingVolume() const { return NULL; }
   virtual float GetBoundingSphereRadius() const { return 0; }

   // @TODO: should this be pushed further down the class hierarchy?
   virtual void Hit() {}
   virtual void ClearHitState() {}

protected:
   typedef std::list<cSceneNode *> tChildren;
   tChildren m_children;

private:
   tVec3 m_translation;
   tQuat m_rotation;

   mutable bool m_bUpdateLocalTransform;
   mutable sMatrix4 m_localTransform;

   uint m_caps;
   uint m_state;
};

///////////////////////////////////////

inline const tVec3 & cSceneNode::GetTranslation() const
{
   return m_translation;
}

///////////////////////////////////////

inline void cSceneNode::SetTranslation(const tVec3 & translation)
{
   m_translation = translation;
   m_bUpdateLocalTransform = true;
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
   m_bUpdateLocalTransform = true;
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
