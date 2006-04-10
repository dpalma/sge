////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYCOMPONENTS_H
#define INCLUDED_ENTITYCOMPONENTS_H

#include "entityapi.h"

#include "model.h"

#include "axisalignedbox.h"

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityPositionComponent
//

class cEntityPositionComponent : public cComObject<IMPLEMENTS(IEntityPositionComponent)>
{
public:
   cEntityPositionComponent();
   ~cEntityPositionComponent();

   virtual tResult SetPosition(const tVec3 & position);
   virtual tResult GetPosition(tVec3 * pPosition) const;
   virtual const tMatrix4 & GetWorldTransform() const;

private:
   tVec3 m_position;

   mutable bool m_bUpdateWorldTransform;
   mutable tMatrix4 m_worldTransform;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityRenderComponent
//

class cEntityRenderComponent : public cComObject<IMPLEMENTS(IEntityRenderComponent)>
{
public:
   cEntityRenderComponent(const tChar * pszModel);
   ~cEntityRenderComponent();

   virtual tResult GetModel(cStr * pModel) const;

   virtual tResult GetBoundingBox(tAxisAlignedBox * pBBox) const;

   virtual void Update(double elapsedTime);
   virtual void Render();

private:
   cStr m_model;
   cModel * m_pModel;
   tBlendedVertices m_blendedVerts;
   cAutoIPtr<IModelAnimationController> m_pAnimController;
   tAxisAlignedBox m_bbox;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntitySpawnComponent
//

class cEntitySpawnComponent : public cComObject<IMPLEMENTS(IEntitySpawnComponent)>
{
public:
   cEntitySpawnComponent();
   ~cEntitySpawnComponent();

   virtual uint GetMaxQueueSize() const;

   virtual tResult SetRallyPoint(const tVec3 & rallyPoint);
   virtual tResult GetRallyPoint(tVec3 * pRallyPoint) const;

   virtual tResult Spawn(const tChar * pszEntity);

private:
   uint m_maxQueueSize;
   tVec3 m_rallyPoint;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYCOMPONENTS_H
