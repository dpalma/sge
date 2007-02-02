///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entitycomponents.h"

#include "tech/globalobj.h"
#include "tech/multivar.h"
#include "tech/statemachinetem.h"

#include <tinyxml.h>

#include <algorithm>

#include "tech/dbgalloc.h" // must be last header

#pragma warning(disable:4355) // 'this' : used in base member initializer list

///////////////////////////////////////////////////////////////////////////////

#define IsFlagSet(f, b) (((f)&(b))==(b))


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityPositionComponent
//

///////////////////////////////////////

cEntityPositionComponent::cEntityPositionComponent()
 : m_position(0,0,0)
 , m_bUpdateWorldTransform(true)
{
}

///////////////////////////////////////

cEntityPositionComponent::~cEntityPositionComponent()
{
}

///////////////////////////////////////

tResult cEntityPositionComponent::SetPosition(const tVec3 & position)
{
   m_position = position;
   m_bUpdateWorldTransform = true;
   return S_OK;
}

///////////////////////////////////////

tResult cEntityPositionComponent::GetPosition(tVec3 * pPosition) const
{
   if (pPosition == NULL)
   {
      return E_POINTER;
   }
   *pPosition = m_position;
   return S_OK;
}

///////////////////////////////////////

const tMatrix4 & cEntityPositionComponent::GetWorldTransform() const
{
   if (m_bUpdateWorldTransform)
   {
      m_bUpdateWorldTransform = false;
      MatrixTranslate(m_position.x, m_position.y, m_position.z, &m_worldTransform);
   }
   return m_worldTransform;
}

///////////////////////////////////////

tResult EntityPositionComponentFactory(const TiXmlElement * pTiXmlElement,
                                       IEntity * pEntity, void * pUser,
                                       IEntityComponent * * ppComponent)
{
   if (pTiXmlElement == NULL || pEntity == NULL || ppComponent == NULL)
   {
      return E_POINTER;
   }

   if (_stricmp(pTiXmlElement->Value(), "position") != 0)
   {
      return E_INVALIDARG;
   }

   cAutoIPtr<cEntityPositionComponent> pPosition = new cEntityPositionComponent();
   if (!pPosition)
   {
      return E_OUTOFMEMORY;
   }

   if (pEntity->SetComponent(kECT_Position, static_cast<IEntityComponent*>(pPosition)) != S_OK)
   {
      return E_FAIL;
   }

   *ppComponent = CTAddRef(static_cast<IEntityComponent*>(pPosition));
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityRenderComponent
//

///////////////////////////////////////

cEntityRenderComponent::cEntityRenderComponent(const tChar * pszModel)
 : m_mainModel(pszModel)
 , m_bboxModel(NULL)
{
}

///////////////////////////////////////

cEntityRenderComponent::~cEntityRenderComponent()
{
}

///////////////////////////////////////

tResult cEntityRenderComponent::GetBoundingBox(tAxisAlignedBox * pBBox) const
{
   return m_mainModel.GetBoundingBox(pBBox);
}

///////////////////////////////////////

void cEntityRenderComponent::Render(uint flags)
{
   m_mainModel.Render();

   if (IsFlagSet(flags, kERF_Selected))
   {
      m_bboxModel.Render();
   }
}

///////////////////////////////////////

void cEntityRenderComponent::Update(double elapsedTime)
{
   m_mainModel.Update(elapsedTime);

   tAxisAlignedBox bbox;
   if (m_mainModel.GetBoundingBox(&bbox) == S_OK && m_bboxModel.AccessModel() == NULL)
   {
      static const float color[] = { 1, 1, 0, 1 };
      cAutoIPtr<IModel> pBBoxModel;
      if (ModelCreateBox(bbox.GetMins(), bbox.GetMaxs(), color, &pBBoxModel) == S_OK)
      {
         m_bboxModel.SetModel(pBBoxModel);
      }
   }
}

///////////////////////////////////////

tResult cEntityRenderComponent::SetAnimation(eAIAgentAnimation anim)
{
   static const struct
   {
      eAIAgentAnimation agentAnim;
      eModelAnimationType modelAnim;
   }
   animMap[] =
   {
      { kAIAgentAnimIdle, kMAT_Idle },
      { kAIAgentAnimWalk, kMAT_Walk },
      { kAIAgentAnimRun, kMAT_Run },
      { kAIAgentAnimMeleeAttack, kMAT_Attack },
      { kAIAgentAnimRangedAttack, kMAT_Attack },
      { kAIAgentAnimDie, kMAT_Death },
      { kAIAgentAnimTakeDamage, kMAT_Damage },
   };
   for (int i = 0; i < _countof(animMap); ++i)
   {
      if (anim == animMap[i].agentAnim)
      {
         return m_mainModel.SetAnimation(animMap[i].modelAnim);
      }
   }
   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////

tResult EntityCreateRenderComponent(const tChar * pszModel, IEntityRenderComponent * * ppRenderComponent)
{
   if (pszModel == NULL || ppRenderComponent == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IEntityRenderComponent> pRenderComponent(
      static_cast<IEntityRenderComponent*>(new cEntityRenderComponent(pszModel)));
   if (!pRenderComponent)
   {
      return E_OUTOFMEMORY;
   }

   return pRenderComponent.GetPointer(ppRenderComponent);
}

///////////////////////////////////////////////////////////////////////////////

tResult EntityRenderComponentFactory(const TiXmlElement * pTiXmlElement,
                                     IEntity * pEntity, void * pUser,
                                     IEntityComponent * * ppComponent)
{
   if (pTiXmlElement == NULL || pEntity == NULL || ppComponent == NULL)
   {
      return E_POINTER;
   }

   if (_stricmp(pTiXmlElement->Value(), "render") != 0)
   {
      return E_INVALIDARG;
   }

   if (pTiXmlElement->Attribute("model") == NULL)
   {
      return E_FAIL;
   }

#ifdef _UNICODE
   cMultiVar temp(pTiXmlElement->Attribute("model"));
   const wchar_t * pszModel = temp.ToWideString();
#else
   const char * pszModel = pTiXmlElement->Attribute("model");
#endif

   cAutoIPtr<cEntityRenderComponent> pRender = new cEntityRenderComponent(pszModel);
   if (!pRender)
   {
      return E_OUTOFMEMORY;
   }

   if (pEntity->SetComponent(kECT_Render, static_cast<IEntityComponent*>(pRender)) != S_OK)
   {
      return E_FAIL;
   }

   *ppComponent = CTAddRef(static_cast<IEntityComponent*>(pRender));
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntitySpawnComponent
//

///////////////////////////////////////

cEntitySpawnComponent::cEntitySpawnComponent(uint maxQueueSize)
 : m_maxQueueSize(maxQueueSize)
 , m_rallyPoint(0,0,0)
{
}

///////////////////////////////////////

cEntitySpawnComponent::~cEntitySpawnComponent()
{
}

///////////////////////////////////////

uint cEntitySpawnComponent::GetMaxQueueSize() const
{
   return m_maxQueueSize;
}

///////////////////////////////////////

tResult cEntitySpawnComponent::SetRallyPoint(const tVec3 & rallyPoint)
{
   m_rallyPoint = rallyPoint;
   return S_OK;
}

///////////////////////////////////////

tResult cEntitySpawnComponent::GetRallyPoint(tVec3 * pRallyPoint) const
{
   if (pRallyPoint == NULL)
   {
      return E_POINTER;
   }
   *pRallyPoint = m_rallyPoint;
   return S_OK;
}

///////////////////////////////////////

tResult EntitySpawnComponentFactory(const TiXmlElement * pTiXmlElement,
                                    IEntity * pEntity, void * pUser,
                                    IEntityComponent * * ppComponent)
{
   if (pTiXmlElement == NULL || pEntity == NULL || ppComponent == NULL)
   {
      return E_POINTER;
   }

   if (_stricmp(pTiXmlElement->Value(), "spawns") != 0)
   {
      return E_INVALIDARG;
   }

   int maxQueueSize = 0;
   if (pTiXmlElement->QueryIntAttribute("queuesize", &maxQueueSize) != TIXML_SUCCESS)
   {
      WarnMsg("No queue size specified for spawn component\n");
   }

   cAutoIPtr<cEntitySpawnComponent> pSpawnComponent = new cEntitySpawnComponent(static_cast<uint>(maxQueueSize));
   if (!pSpawnComponent)
   {
      return E_OUTOFMEMORY;
   }

   if (pEntity->SetComponent(kECT_Spawn, static_cast<IEntityComponent*>(pSpawnComponent)) != S_OK)
   {
      return E_FAIL;
   }

   *ppComponent = CTAddRef(static_cast<IEntityComponent*>(pSpawnComponent));
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityBrainComponent
//

///////////////////////////////////////

cEntityBrainComponent::cEntityBrainComponent(IAIAgent * pAgent)
 : m_pAgent(CTAddRef(pAgent))
{
}

///////////////////////////////////////

cEntityBrainComponent::~cEntityBrainComponent()
{
}

///////////////////////////////////////

tResult cEntityBrainComponent::Create(IEntity * pEntity, IEntityBrainComponent * * ppBrainComponent)
{
   if (pEntity == NULL || ppBrainComponent == NULL)
   {
      return E_POINTER;
   }

   tResult result = E_FAIL;

   cAutoIPtr<IAIAgent> pAgent;
   if ((result = AIAgentCreate(pEntity->GetId(), &pAgent)) != S_OK)
   {
      return result;
   }

   {
      cAutoIPtr<IEntityPositionComponent> pPosition;
      if (pEntity->GetComponent(kECT_Position, IID_IEntityPositionComponent, &pPosition) == S_OK)
      {
         cAutoIPtr<IAIAgentLocationProvider> pLocationProvider;
         if (pPosition->QueryInterface(IID_IAIAgentLocationProvider, (void**)&pLocationProvider) == S_OK)
         {
            pAgent->SetLocationProvider(pLocationProvider);
         }
      }
   }

   {
      cAutoIPtr<IEntityRenderComponent> pRender;
      if (pEntity->GetComponent(kECT_Render, IID_IEntityRenderComponent, &pRender) == S_OK)
      {
         cAutoIPtr<IAIAgentAnimationProvider> pAnimationProvider;
         if (pRender->QueryInterface(IID_IAIAgentAnimationProvider, (void**)&pAnimationProvider) == S_OK)
         {
            pAgent->SetAnimationProvider(pAnimationProvider);
         }
      }
   }

   cAutoIPtr<IAIBehavior> pDefaultBehavior;
   if ((result = AIBehaviorStandCreate(&pDefaultBehavior)) != S_OK)
   {
      return result;
   }

   if ((result = pAgent->SetDefaultBehavior(pDefaultBehavior)) != S_OK)
   {
      return result;
   }

   cAutoIPtr<cEntityBrainComponent> pBrainComponent = new cEntityBrainComponent(pAgent);
   if (!pBrainComponent)
   {
      return E_OUTOFMEMORY;
   }

   *ppBrainComponent = static_cast<IEntityBrainComponent *>(CTAddRef(pBrainComponent));
   return S_OK;
}

///////////////////////////////////////

tResult cEntityBrainComponent::MoveTo(const tVec3 & point)
{
   Assert(!!m_pAgent);

   cAutoIPtr<IAIBehavior> pMoveToBehavior;
   if (AIBehaviorMoveToCreate(point, &pMoveToBehavior) == S_OK)
   {
      m_pAgent->PushBehavior(pMoveToBehavior);
      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cEntityBrainComponent::Stop()
{
   Assert(!!m_pAgent);
   int sanityCheck = 1000;
   while ((m_pAgent->PopBehavior() == S_OK) && (--sanityCheck > 0))
   {
      // do nothing
   }
   return S_OK;
}

///////////////////////////////////////

void cEntityBrainComponent::Update(double elapsedTime)
{
   Assert(!!m_pAgent);
   cAutoIPtr<IAIBehavior> pBehavior;
   if (m_pAgent->GetActiveBehavior(&pBehavior) == S_OK)
   {
      if (pBehavior->Update(m_pAgent, elapsedTime) == S_AI_BEHAVIOR_DONE)
      {
         m_pAgent->PopBehavior();
      }
   }
}

///////////////////////////////////////

tResult EntityBrainComponentFactory(const TiXmlElement * pTiXmlElement,
                                    IEntity * pEntity, void * pUser,
                                    IEntityComponent * * ppComponent)
{
   if (pTiXmlElement == NULL || pEntity == NULL || ppComponent == NULL)
   {
      return E_POINTER;
   }

   if (_stricmp(pTiXmlElement->Value(), "brain") != 0)
   {
      return E_INVALIDARG;
   }

   tResult result = E_FAIL;
   cAutoIPtr<IEntityBrainComponent> pBrainComponent;
   if ((result = cEntityBrainComponent::Create(pEntity, &pBrainComponent)) != S_OK)
   {
      return result;
   }

   if (pEntity->SetComponent(kECT_Brain, static_cast<IEntityComponent*>(pBrainComponent)) != S_OK)
   {
      return E_FAIL;
   }

   *ppComponent = CTAddRef(static_cast<IEntityComponent*>(pBrainComponent));
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////

void RegisterBuiltInComponents()
{
   UseGlobal(EntityManager);
   Verify(pEntityManager->RegisterComponentFactory(_T("position"), EntityPositionComponentFactory) == S_OK);
   Verify(pEntityManager->RegisterComponentFactory(_T("render"), EntityRenderComponentFactory) == S_OK);
   Verify(pEntityManager->RegisterComponentFactory(_T("spawns"), EntitySpawnComponentFactory) == S_OK);
   Verify(pEntityManager->RegisterComponentFactory(_T("brain"), EntityBrainComponentFactory) == S_OK);
}


///////////////////////////////////////////////////////////////////////////////
