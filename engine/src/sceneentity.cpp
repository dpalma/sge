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
 : m_pParent(NULL),
   m_caps(kDefaultCaps),
   m_state(0),
   m_localTranslation(0,0,0),
   m_localRotation(0,0,0,1),
   m_bHaveLocalTransform(false),
   m_bHaveWorldTranslation(false),
   m_bHaveWorldRotation(false),
   m_bHaveWorldTransform(false)
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

const tMatrix4 & cSceneNode::GetLocalTransform() const
{
   if (!m_bHaveLocalTransform)
   {
      const tVec3 & t = GetLocalTranslation();
      const tQuat & r = GetLocalRotation();

      sMatrix4 mt, mr;
      MatrixTranslate(t.x, t.y, t.z, &mt);
      r.ToMatrix(&mr);

      m_localTransform = mt * mr;

      m_bHaveLocalTransform = true;
   }

   return m_localTransform;
}

///////////////////////////////////////

const tVec3 & cSceneNode::GetWorldTranslation() const
{
   if (!m_bHaveWorldTranslation)
   {
      const cSceneNode * pParent = GetParent();
      if (pParent != NULL)
      {
         m_worldTranslation = GetLocalTranslation() + pParent->GetWorldTranslation();
      }
      else
      {
         m_worldTranslation = GetLocalTranslation();
      }

      m_bHaveWorldTranslation = true;
   }

   return m_worldTranslation;
}

///////////////////////////////////////

const tQuat & cSceneNode::GetWorldRotation() const
{
   if (!m_bHaveWorldRotation)
   {
      const cSceneNode * pParent = GetParent();
      if (pParent != NULL)
      {
         m_worldRotation = GetLocalRotation() * pParent->GetWorldRotation();
      }
      else
      {
         m_worldRotation = GetLocalRotation();
      }

      m_bHaveWorldRotation = true;
   }

   return m_worldRotation;
}

///////////////////////////////////////

const tMatrix4 & cSceneNode::GetWorldTransform() const
{
   if (!m_bHaveWorldTransform)
   {
      const tVec3 & t = GetWorldTranslation();
      const tQuat & r = GetWorldRotation();

      sMatrix4 mt, mr;
      MatrixTranslate(t.x, t.y, t.z, &mt);
      r.ToMatrix(&mr);

      m_worldTransform = mt * mr;

      m_bHaveWorldTransform = true;
   }

   return m_worldTransform;
}

///////////////////////////////////////

bool cSceneNode::AddChild(cSceneNode * pNode)
{
   if (pNode != NULL)
   {
      m_children.push_back(pNode);
      pNode->m_pParent = this;
      pNode->m_bHaveWorldTranslation = false;
      pNode->m_bHaveWorldRotation = false;
      pNode->m_bHaveWorldTransform = false;
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
         Assert(pNode->m_pParent == this);
         pNode->m_pParent = NULL;
         pNode->m_bHaveWorldTranslation = false;
         pNode->m_bHaveWorldRotation = false;
         pNode->m_bHaveWorldTransform = false;
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
   glMultMatrixf(GetLocalTransform().m);

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
