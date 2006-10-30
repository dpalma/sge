////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYCOMPONENTS_H
#define INCLUDED_ENTITYCOMPONENTS_H

#include "engine/entityapi.h"

#include "engine/modelapi.h"
#include "engine/modeltypes.h"

#include "tech/schedulerapi.h"
#include "tech/axisalignedbox.h"
#include "tech/statemachine.h"

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
// CLASS: cEntityModelRenderer
//

typedef std::vector<sBlendedVertex> tBlendedVertices;

class cEntityModelRenderer
{
   cEntityModelRenderer(const cEntityModelRenderer &);
   const cEntityModelRenderer & operator =(const cEntityModelRenderer &);

public:
   cEntityModelRenderer(const tChar * pszModel);
   ~cEntityModelRenderer();

   tResult GetBoundingBox(tAxisAlignedBox * pBBox) const;
   void Update(double elapsedTime);
   void Render();
   tResult SetAnimation(eModelAnimationType type);

private:
   cStr m_model;
   cAutoIPtr<IModel> m_pModel;
   std::vector<tMatrix34> m_blendMatrices;
   tBlendedVertices m_blendedVerts;
   cAutoIPtr<IModelAnimationController> m_pAnimController;
   tAxisAlignedBox m_bbox;
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

   virtual tResult GetBoundingBox(tAxisAlignedBox * pBBox) const;

   virtual void Update(double elapsedTime);
   virtual void Render(uint flags);
   virtual tResult SetAnimation(eModelAnimationType type);

private:
   cEntityModelRenderer m_mainModel;
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
// CLASS: cEntityBasicBrain
//

class cEntityBasicBrain : public cStateMachine<cEntityBasicBrain, double>
{
public:
   cEntityBasicBrain();
   ~cEntityBasicBrain();

   void MoveTo(const tVec3 & point, IEntityPositionComponent * pPosition, IEntityRenderComponent * pRender);
   void Stop();

private:
   void OnEnterIdle();
   void OnIdle(double elapsed);
   void OnExitIdle();

   void OnEnterMoving();
   void OnMoving(double elapsed);
   void OnExitMoving();

   tState m_idleState;
   tState m_movingState;

   class cTask : public cComObject<IMPLEMENTS(ITask)>
   {
   public:
      cTask(cEntityBasicBrain * pOuter);
      virtual void DeleteThis() {}
      virtual tResult Execute(double time);
   private:
      cEntityBasicBrain * m_pOuter;
      double m_lastTime;
   };

   friend class cTask;
   cTask m_task;

   tVec3 m_moveGoal;
   cAutoIPtr<IEntityPositionComponent> m_pPosition;
   cAutoIPtr<IEntityRenderComponent> m_pRender;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityBrainComponent
//

class cEntityBrainComponent : public cComObject<IMPLEMENTS(IEntityBrainComponent)>
{
public:
   cEntityBrainComponent(IEntity * pEntity);
   ~cEntityBrainComponent();

   virtual tResult MoveTo(const tVec3 & point);
   virtual tResult Stop();

private:
   cEntityBasicBrain m_brain;
   IEntity * m_pEntity; // Don't hold a reference to the entity because it owns this component
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYCOMPONENTS_H
