///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "scenegroup.h"

#include "render.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneGroup
//

///////////////////////////////////////

cSceneGroup::cSceneGroup()
 : cSceneNode()
{
}

///////////////////////////////////////

cSceneGroup::~cSceneGroup()
{
   std::list<cSceneNode *>::iterator iter;
   for (iter = m_children.begin(); iter != m_children.end(); iter++)
   {
      delete *iter;
   }
   m_children.clear();
}

///////////////////////////////////////

void cSceneGroup::AddChild(cSceneNode * pNode)
{
   Assert(pNode != NULL);
   m_children.push_back(pNode);
   pNode->SetParent(this);
}

///////////////////////////////////////

bool cSceneGroup::RemoveChild(cSceneNode * pNode)
{
   Assert(pNode != NULL);
   std::list<cSceneNode *>::iterator iter;
   for (iter = m_children.begin(); iter != m_children.end(); iter++)
   {
      if (*iter == pNode)
      {
         m_children.erase(iter);
         return true;
      }
   }
   return false;
}

///////////////////////////////////////

void cSceneGroup::Traverse(cSceneNodeVisitor * pVisitor)
{
   Assert(pVisitor != NULL);
   std::list<cSceneNode *>::iterator iter;
   for (iter = m_children.begin(); iter != m_children.end(); iter++)
   {
      pVisitor->VisitSceneNode(*iter);

      (*iter)->Traverse(pVisitor);
   }
}

///////////////////////////////////////

void cSceneGroup::Update(float timeDelta)
{
}

///////////////////////////////////////

void cSceneGroup::Render()
{
}

///////////////////////////////////////////////////////////////////////////////
