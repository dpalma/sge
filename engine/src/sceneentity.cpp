///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "scenegraph.h"

#include "render.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneNode
//

///////////////////////////////////////

cSceneNode::cSceneNode()
 : m_pParent(NULL),
   m_translation(0,0,0),
   m_rotation(0,0,0,1),
   m_bUpdateWorldTransform(false)
{
}

///////////////////////////////////////

cSceneNode::~cSceneNode()
{
   std::list<cSceneNode *>::iterator iter;
   for (iter = m_children.begin(); iter != m_children.end(); iter++)
   {
      delete *iter;
   }
   m_children.clear();
}

///////////////////////////////////////

void cSceneNode::AddChild(cSceneNode * pNode)
{
   Assert(pNode != NULL);
   m_children.push_back(pNode);
   pNode->SetParent(this);
}

///////////////////////////////////////

bool cSceneNode::RemoveChild(cSceneNode * pNode)
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

HANDLE cSceneNode::IterChildrenBegin()
{
   if (!m_children.empty())
   {
      std::list<cSceneNode *>::iterator * pIter = new std::list<cSceneNode *>::iterator(m_children.begin());
      return (HANDLE)pIter;
   }
   return NULL;
}

///////////////////////////////////////

bool cSceneNode::IterNextChild(HANDLE hIter, cSceneNode * * ppNode)
{
   Assert(hIter != NULL);
   std::list<cSceneNode *>::iterator * pIter = (std::list<cSceneNode *>::iterator *)hIter;
   if (*pIter != m_children.end())
   {
      *ppNode = *(*pIter);
      (*pIter)++;
      return true;
   }
   return false;
}

///////////////////////////////////////

void cSceneNode::IterChildrenEnd(HANDLE hIter)
{
   std::list<cSceneNode *>::iterator * pIter = (std::list<cSceneNode *>::iterator *)hIter;
   delete pIter;
}

///////////////////////////////////////

void cSceneNode::Traverse(cSceneNodeVisitor * pVisitor)
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

void cSceneNode::CalcWorldRT(tQuat * pR, tVec3 * pT) const
{
   tVec3 t = GetTranslation();
   tQuat r = GetRotation();
   const cSceneNode * pParent = GetParent();
   while (pParent != NULL)
   {
      r = r * pParent->GetRotation();
      t += pParent->GetTranslation();
      pParent = pParent->GetParent();
   }
   if (pR != NULL)
      *pR = r;
   if (pT != NULL)
      *pT = t;
}

///////////////////////////////////////

const sMatrix4 & cSceneNode::GetWorldTransform() const
{
   if (m_bUpdateWorldTransform)
   {
      tVec3 worldT;
      tQuat worldR;
      CalcWorldRT(&worldR, &worldT);

      sMatrix4 mt, mr;
      MatrixTranslate(worldT.x, worldT.y, worldT.z, &mt);
      worldR.ToMatrix(&mr);

      m_world = mt * mr;

      m_bUpdateWorldTransform = false;
   }
   return m_world;
}

///////////////////////////////////////

void cSceneNode::Update(float timeDelta)
{
}

///////////////////////////////////////

void cSceneNode::Render()
{
}

///////////////////////////////////////

void cSceneNode::SetParent(cSceneNode * pParent)
{
   if (pParent != m_pParent)
   {
      if (m_pParent != NULL)
      {
         m_pParent->RemoveChild(this);
      }
      m_pParent = pParent;
   }
   m_bUpdateWorldTransform = true;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneNodeVisitor
//

cSceneNodeVisitor::~cSceneNodeVisitor()
{
}


///////////////////////////////////////////////////////////////////////////////
