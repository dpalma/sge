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

private:
   std::list<cSceneNode *> m_children;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCENEGROUP_H
