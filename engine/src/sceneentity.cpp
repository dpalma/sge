///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "scenenode.h"
#include "ggl.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneNode
//

static const int kDefaultCaps = kSNC_Pickable;

///////////////////////////////////////

cSceneNode::cSceneNode()
 : m_caps(kDefaultCaps),
   m_state(0),
   m_translation(0,0,0),
   m_rotation(0,0,0,1),
   m_bUpdateLocalTransform(false)
{
   m_localTransform.Identity();
}

///////////////////////////////////////

cSceneNode::~cSceneNode()
{
   tChildren::iterator iter;
   for (iter = m_children.begin(); iter != m_children.end(); iter++)
   {
      delete *iter;
   }
   m_children.clear();
}

///////////////////////////////////////

const sMatrix4 & cSceneNode::GetTransform() const
{
   if (m_bUpdateLocalTransform)
   {
      const tVec3 & t = GetTranslation();
      const tQuat & r = GetRotation();

      sMatrix4 mt, mr;
      MatrixTranslate(t.x, t.y, t.z, &mt);
      r.ToMatrix(&mr);

      m_localTransform = mt * mr;
      m_bUpdateLocalTransform = false;
   }

   return m_localTransform;
}

///////////////////////////////////////

bool cSceneNode::AddChild(cSceneNode * pNode)
{
   if (pNode != NULL)
   {
      m_children.push_back(pNode);
      return true;
   }
   return false;
}

///////////////////////////////////////

bool cSceneNode::RemoveChild(cSceneNode * pNode)
{
   Assert(pNode != NULL);
   tChildren::iterator iter;
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

void cSceneNode::Traverse(cSceneNodeVisitor * pVisitor)
{
   Assert(pVisitor != NULL);
   tChildren::iterator iter;
   for (iter = m_children.begin(); iter != m_children.end(); iter++)
   {
      pVisitor->VisitSceneNode(*iter);

      (*iter)->Traverse(pVisitor);
   }
}

///////////////////////////////////////

void cSceneNode::Render()
{
   glPushMatrix();
   glMultMatrixf(GetTransform().m);

   tChildren::iterator iter;
   for (iter = m_children.begin(); iter != m_children.end(); iter++)
   {
      (*iter)->Render();
   }

   glPopMatrix();
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneNodeVisitor
//

cSceneNodeVisitor::~cSceneNodeVisitor()
{
}


///////////////////////////////////////////////////////////////////////////////
