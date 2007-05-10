////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYCOMPONENTS_H
#define INCLUDED_ENTITYCOMPONENTS_H

#include "modelutils.h"

#include "engine/entityapi.h"

#include "ai/aiagentapi.h"

#include "engine/modelapi.h"
#include "engine/modeltypes.h"

#include "tech/axisalignedbox.h"
#include "tech/statemachine.h"

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityPositionComponent
//

class cEntityPositionComponent : public cComObject2<IMPLEMENTS(IEntityPositionComponent),
                                                    IMPLEMENTS(IAIAgentLocationProvider)>
{
public:
   cEntityPositionComponent();
   ~cEntityPositionComponent();

   virtual tResult SetPosition(const tVec3 & position);
   virtual tResult GetPosition(tVec3 * pPosition) const;

   virtual tResult SetOrientation(const tQuat & orientation);
   virtual tResult GetOrientation(tQuat * pOrientation) const;

   virtual const tMatrix4 & GetWorldTransform() const;

private:
   tVec3 m_position;
   tQuat m_orientation;

   enum eUpdateWorldTransformFlags
   {
      kUpdateNone = 0,
      kUpdateRotation = 1,
      kUpdateTranslation = 2,
      kUpdateAll = 0xFF,
   };
   mutable uint m_updateWorldTransform;
   mutable tMatrix4 m_worldTransform;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityRenderComponent
//

class cEntityRenderComponent : public cComObject4<IMPLEMENTS(IEntityRenderComponent),
                                                  IMPLEMENTS(IRenderable),
                                                  IMPLEMENTS(IUpdatable),
                                                  IMPLEMENTS(IAIAgentAnimationProvider)>
{
public:
   cEntityRenderComponent(const tChar * pszModel);
   ~cEntityRenderComponent();

   virtual void Render();

   virtual tResult GetBoundingBox(tAxisAlignedBox * pBBox) const;

   virtual void Render(uint flags);

   virtual void Update(double elapsedTime);

   virtual tResult RequestAnimation(eAIAgentAnimation anim);

private:
   cAnimatedModelRenderer m_mainModel;
   cBasicModelRenderer m_bboxModel;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntitySpawnComponent
//

class cEntitySpawnComponent : public cComObject<IMPLEMENTS(IEntitySpawnComponent)>
{
public:
   cEntitySpawnComponent(uint maxQueueSize);
   ~cEntitySpawnComponent();

   virtual uint GetMaxQueueSize() const;

   virtual tResult SetRallyPoint(const tVec3 & rallyPoint);
   virtual tResult GetRallyPoint(tVec3 * pRallyPoint) const;

private:
   uint m_maxQueueSize;
   tVec3 m_rallyPoint;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityBrainComponent
//

class cEntityBrainComponent : public cComObject<IMPLEMENTS(IEntityBrainComponent)>
{
   cEntityBrainComponent(IAIAgent * pAgent);

public:
   ~cEntityBrainComponent();

   static tResult Create(IEntity * pEntity, IEntityBrainComponent * * ppBrainComponent);

private:
   cAutoIPtr<IAIAgent> m_pAgent;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYCOMPONENTS_H
