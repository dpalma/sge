///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "scenegroup.h"

#include "ggl.h"
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
   tChildren::iterator iter;
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

void cSceneGroup::Traverse(cSceneNodeVisitor * pVisitor)
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

void cSceneGroup::Update(float timeDelta)
{
}

///////////////////////////////////////

void cSceneGroup::Render()
{
   tChildren::iterator iter;
   for (iter = m_children.begin(); iter != m_children.end(); iter++)
   {
      (*iter)->Render();
   }
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneTransformGroup
//

///////////////////////////////////////

cSceneTransformGroup::cSceneTransformGroup()
 : cSceneGroup(),
   m_translation(0,0,0),
   m_rotation(0,0,0,1),
   m_bUpdateLocalTransform(false)
//   m_bUpdateWorldTransform(false)
{
}

///////////////////////////////////////

cSceneTransformGroup::~cSceneTransformGroup()
{
}

///////////////////////////////////////

const sMatrix4 & cSceneTransformGroup::GetTransform() const
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

/////////////////////////////////////////
//
//void cSceneTransformGroup::CalcWorldRT(tQuat * pR, tVec3 * pT) const
//{
//   tVec3 t = GetTranslation();
//   tQuat r = GetRotation();
//   const cSceneNode * pParent = GetParent();
//   while (pParent != NULL)
//   {
//      r = r * pParent->GetRotation();
//      t += pParent->GetTranslation();
//      pParent = pParent->GetParent();
//   }
//   if (pR != NULL)
//      *pR = r;
//   if (pT != NULL)
//      *pT = t;
//}
//
/////////////////////////////////////////
//
//const sMatrix4 & cSceneTransformGroup::GetWorldTransform() const
//{
//   if (m_bUpdateWorldTransform)
//   {
//      tVec3 worldT;
//      tQuat worldR;
//      CalcWorldRT(&worldR, &worldT);
//
//      sMatrix4 mt, mr;
//      MatrixTranslate(worldT.x, worldT.y, worldT.z, &mt);
//      worldR.ToMatrix(&mr);
//
//      m_worldTransform = mt * mr;
//      m_bUpdateWorldTransform = false;
//   }
//
//   return m_worldTransform;
//}

///////////////////////////////////////

void cSceneTransformGroup::Render()
{
   glPushMatrix();
   glMultMatrixf(GetTransform().m);

   cSceneGroup::Render();

   glPopMatrix();
}

///////////////////////////////////////////////////////////////////////////////
