///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "scenenode.h"

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
}

///////////////////////////////////////

void cSceneNode::Traverse(cSceneNodeVisitor * pVisitor)
{
   Assert(pVisitor != NULL);
   pVisitor->VisitSceneNode(this);
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
   // TODO
   //      m_pParent->RemoveChild(this);
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
