///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCENENODE_H
#define INCLUDED_SCENENODE_H

#include "vec3.h"
#include "quat.h"
#include "matrix4.h"

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
   virtual ~cSceneNode() = 0;

   bool IsPickable() const;
   void SetPickable(bool pickable);

   bool IsSelected() const;
   void SetSelected(bool selected);

   virtual void Traverse(cSceneNodeVisitor * pVisitor);

   virtual void Render() = 0;

   virtual const cBoundingVolume * GetBoundingVolume() const { return NULL; }
   virtual float GetBoundingSphereRadius() const { return 0; }

   // @TODO: should this be pushed further down the class hierarchy?
   virtual void Hit() {}
   virtual void ClearHitState() {}

private:
   uint m_caps;
   uint m_state;
};

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
