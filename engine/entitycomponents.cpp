///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entitycomponents.h"

#include "engine/terrainapi.h"

#include "tech/globalobj.h"
#include "tech/multivar.h"
#include "tech/statemachinetem.h"
#include "tech/techhash.h"

#include <tinyxml.h>

#include <algorithm>

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

#define IsFlagSet(f, b) (((f)&(b))==(b))


///////////////////////////////////////////////////////////////////////////////

static const tChar g_entityPositionComponentName[] = _T("position");
static const tChar g_entityRenderComponentName[] = _T("render");
static const tChar g_entitySpawnComponentName[] = _T("spawns");
static const tChar g_entityBrainComponentName[] = _T("brain");
static const tChar g_entityBoxSelectionIndicatorComponentName[] = _T("boxselectionindicator");


///////////////////////////////////////////////////////////////////////////////

tEntityComponentID GenerateEntityComponentID(const tChar * pszComponentName)
{
   Assert(pszComponentName != NULL);
   if (pszComponentName == NULL)
   {
      return 0;
   }
   return reinterpret_cast<tEntityComponentID>(Hash(pszComponentName, _tcslen(pszComponentName) * sizeof(tChar)));
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cOldStyleFactoryWrapper
//

typedef tResult (* tEntityComponentFactoryFn)(const TiXmlElement * pTiXmlElement,
                                              IEntity * pEntity,
                                              IEntityComponent * * ppComponent);

class cOldStyleFactoryWrapper : public cComObject<IMPLEMENTS(IEntityComponentFactory)>
{
public:
   cOldStyleFactoryWrapper(tEntityComponentFactoryFn factoryFn)
    : m_factoryFn(factoryFn)
   {
   }

   virtual tResult CreateComponent(IEntity * pEntity, IEntityComponent * * ppComponent)
   {
      return (*m_factoryFn)(NULL, pEntity, ppComponent);
   }

   virtual tResult CreateComponent(const TiXmlElement * pTiXmlElement, IEntity * pEntity, IEntityComponent * * ppComponent)
   {
      return (*m_factoryFn)(pTiXmlElement, pEntity, ppComponent);
   }

private:
   tEntityComponentFactoryFn m_factoryFn;
};

static tResult RegisterOldStyleComponentFactory(const tChar * pszComponent, tEntityComponentID componentId,
                                                tEntityComponentFactoryFn pfnFactory)
{
   if (pszComponent == NULL || pfnFactory == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IEntityComponentFactory> pFactory(new cOldStyleFactoryWrapper(pfnFactory));
   if (!pFactory)
   {
      return E_FAIL;
   }

   UseGlobal(EntityComponentRegistry);
   return pEntityComponentRegistry->RegisterComponentFactory(pszComponent, componentId, pFactory);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityPositionComponent
//

///////////////////////////////////////

const tEntityComponentID IEntityPositionComponent::CID = GenerateEntityComponentID(g_entityPositionComponentName);

///////////////////////////////////////

cEntityPositionComponent::cEntityPositionComponent()
 : m_position(0,0,0)
 , m_orientation(g_terrainVertical,0) // rotation of 0 about the vertical axis by default
 , m_updateWorldTransform(kUpdateAll)
 , m_worldTransform(tMatrix4::GetIdentity())
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
   m_updateWorldTransform |= kUpdateTranslation;
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

tResult cEntityPositionComponent::SetOrientation(const tQuat & orientation)
{
   m_orientation = orientation;
   m_updateWorldTransform |= kUpdateRotation;
   return S_OK;
}

///////////////////////////////////////

tResult cEntityPositionComponent::GetOrientation(tQuat * pOrientation) const
{
   if (pOrientation == NULL)
   {
      return E_POINTER;
   }
   *pOrientation = m_orientation;
   return S_OK;
}

///////////////////////////////////////

const tMatrix4 & cEntityPositionComponent::GetWorldTransform() const
{
   if ((m_updateWorldTransform & kUpdateRotation) == kUpdateRotation)
   {
      m_updateWorldTransform &= ~kUpdateRotation;

      tMatrix3 orientationMatrix;
      m_orientation.ToMatrix(&orientationMatrix);

      m_worldTransform.m00 = orientationMatrix.m00;
      m_worldTransform.m10 = orientationMatrix.m10;
      m_worldTransform.m20 = orientationMatrix.m20;
      m_worldTransform.m01 = orientationMatrix.m01;
      m_worldTransform.m11 = orientationMatrix.m11;
      m_worldTransform.m21 = orientationMatrix.m21;
      m_worldTransform.m02 = orientationMatrix.m02;
      m_worldTransform.m12 = orientationMatrix.m12;
      m_worldTransform.m22 = orientationMatrix.m22;
   }

   if ((m_updateWorldTransform & kUpdateTranslation) == kUpdateTranslation)
   {
      m_updateWorldTransform &= ~kUpdateTranslation;

      m_worldTransform.m03 = m_position.x;
      m_worldTransform.m13 = m_position.y;
      m_worldTransform.m23 = m_position.z;
   }

   return m_worldTransform;
}

///////////////////////////////////////

tResult EntityPositionComponentFactory(const TiXmlElement * pTiXmlElement,
                                       IEntity * pEntity,
                                       IEntityComponent * * ppComponent)
{
   if (pTiXmlElement == NULL || pEntity == NULL || ppComponent == NULL)
   {
      return E_POINTER;
   }

   if (_stricmp(pTiXmlElement->Value(), g_entityPositionComponentName) != 0)
   {
      return E_INVALIDARG;
   }

   cAutoIPtr<cEntityPositionComponent> pPosition = new cEntityPositionComponent();
   if (!pPosition)
   {
      return E_OUTOFMEMORY;
   }

   if (pEntity->SetComponent(IEntityPositionComponent::CID, static_cast<IEntityComponent*>(pPosition)) != S_OK)
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

const tEntityComponentID IEntityRenderComponent::CID = GenerateEntityComponentID(g_entityRenderComponentName);

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

void cEntityRenderComponent::Render()
{
   m_mainModel.Render();
}

///////////////////////////////////////

tResult cEntityRenderComponent::GetBoundingBox(tAxisAlignedBox * pBBox) const
{
   return m_mainModel.GetBoundingBox(pBBox);
}

///////////////////////////////////////

void cEntityRenderComponent::Render(uint flags)
{
   Render();

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

tResult cEntityRenderComponent::RequestAnimation(eAIAgentAnimation anim)
{
   static const struct
   {
      eAIAgentAnimation agentAnim;
      eModelAnimationType modelAnim;
   }
   animMap[] =
   {
      { kAIAA_Fidget,            kMAT_Idle },
      { kAIAA_Walk,              kMAT_Walk },
      { kAIAA_Run,               kMAT_Run },
      { kAIAA_MeleeAttack,       kMAT_Attack },
      { kAIAA_RangedAttack,      kMAT_Attack },
      { kAIAA_Die,               kMAT_Death },
      { kAIAA_TakeDamage,        kMAT_Damage },
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
                                     IEntity * pEntity,
                                     IEntityComponent * * ppComponent)
{
   if (pTiXmlElement == NULL || pEntity == NULL || ppComponent == NULL)
   {
      return E_POINTER;
   }

   if (_stricmp(pTiXmlElement->Value(), g_entityRenderComponentName) != 0)
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

   if (pEntity->SetComponent(IEntityRenderComponent::CID, static_cast<IEntityComponent*>(pRender)) != S_OK)
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

const tEntityComponentID IEntitySpawnComponent::CID = GenerateEntityComponentID(g_entitySpawnComponentName);

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
                                    IEntity * pEntity,
                                    IEntityComponent * * ppComponent)
{
   if (pTiXmlElement == NULL || pEntity == NULL || ppComponent == NULL)
   {
      return E_POINTER;
   }

   if (_stricmp(pTiXmlElement->Value(), g_entitySpawnComponentName) != 0)
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

   if (pEntity->SetComponent(IEntitySpawnComponent::CID, static_cast<IEntityComponent*>(pSpawnComponent)) != S_OK)
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

const tEntityComponentID IEntityBrainComponent::CID = GenerateEntityComponentID(g_entityBrainComponentName);

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
   if ((result = AIAgentCreate(pEntity->GetId(), NULL, &pAgent)) != S_OK)
   {
      return result;
   }

   cAutoIPtr<IAIAgentBehavior> pBehavior;
   if (AIAgentBehaviorFollowOrdersCreate(&pBehavior) == S_OK)
   {
      pAgent->PushBehavior(pBehavior);
   }

   {
      cAutoIPtr<IEntityPositionComponent> pPosition;
      if (pEntity->GetComponent(IID_IEntityPositionComponent, &pPosition) == S_OK)
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
      if (pEntity->GetComponent(IID_IEntityRenderComponent, &pRender) == S_OK)
      {
         cAutoIPtr<IAIAgentAnimationProvider> pAnimationProvider;
         if (pRender->QueryInterface(IID_IAIAgentAnimationProvider, (void**)&pAnimationProvider) == S_OK)
         {
            pAgent->SetAnimationProvider(pAnimationProvider);
         }
      }
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

tResult EntityBrainComponentFactory(const TiXmlElement * pTiXmlElement,
                                    IEntity * pEntity,
                                    IEntityComponent * * ppComponent)
{
   if (pTiXmlElement == NULL || pEntity == NULL || ppComponent == NULL)
   {
      return E_POINTER;
   }

   if (_stricmp(pTiXmlElement->Value(), g_entityBrainComponentName) != 0)
   {
      return E_INVALIDARG;
   }

   tResult result = E_FAIL;
   cAutoIPtr<IEntityBrainComponent> pBrainComponent;
   if ((result = cEntityBrainComponent::Create(pEntity, &pBrainComponent)) != S_OK)
   {
      return result;
   }

   if (pEntity->SetComponent(IEntityBrainComponent::CID, static_cast<IEntityComponent*>(pBrainComponent)) != S_OK)
   {
      return E_FAIL;
   }

   *ppComponent = CTAddRef(static_cast<IEntityComponent*>(pBrainComponent));
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityBoxSelectionIndicatorComponent
//

///////////////////////////////////////

const tEntityComponentID IEntityBoxSelectionIndicatorComponent::CID =
   GenerateEntityComponentID(g_entityBoxSelectionIndicatorComponentName);

///////////////////////////////////////

cEntityBoxSelectionIndicatorComponent::cEntityBoxSelectionIndicatorComponent(IModel * pBBoxModel)
 : m_bboxModel(pBBoxModel)
{
}

///////////////////////////////////////

cEntityBoxSelectionIndicatorComponent::~cEntityBoxSelectionIndicatorComponent()
{
}

///////////////////////////////////////

void cEntityBoxSelectionIndicatorComponent::Render()
{
   m_bboxModel.Render();
}

///////////////////////////////////////

tResult EntityBoxSelectionIndicatorComponentFactory(const TiXmlElement *,
                                                    IEntity * pEntity,
                                                    IEntityComponent * * ppComponent)
{
   if (pEntity == NULL || ppComponent == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IModel> pBBoxModel;

   cAutoIPtr<IEntityRenderComponent> pRender;
   if (pEntity->GetComponent(IID_IEntityRenderComponent, &pRender) == S_OK)
   {
      tAxisAlignedBox bbox;
      if (pRender->GetBoundingBox(&bbox) == S_OK)
      {
         static const float color[] = { 1, 1, 0, 1 };
         ModelCreateBox(bbox.GetMins(), bbox.GetMaxs(), color, &pBBoxModel);
      }
   }

   if (!pBBoxModel)
   {
      return E_FAIL;
   }

   cAutoIPtr<IEntityComponent> pComponent(static_cast<IEntityComponent*>(
      new cEntityBoxSelectionIndicatorComponent(pBBoxModel)));
   if (!pComponent)
   {
      return E_OUTOFMEMORY;
   }

   if (pEntity->SetComponent(IEntityBoxSelectionIndicatorComponent::CID, pComponent) != S_OK)
   {
      return E_FAIL;
   }

   *ppComponent = CTAddRef(pComponent);
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////

void RegisterBuiltInComponents()
{
   Verify(RegisterOldStyleComponentFactory(g_entityPositionComponentName, IEntityPositionComponent::CID, EntityPositionComponentFactory) == S_OK);
   Verify(RegisterOldStyleComponentFactory(g_entityRenderComponentName, IEntityRenderComponent::CID, EntityRenderComponentFactory) == S_OK);
   Verify(RegisterOldStyleComponentFactory(g_entitySpawnComponentName, IEntitySpawnComponent::CID, EntitySpawnComponentFactory) == S_OK);
   Verify(RegisterOldStyleComponentFactory(g_entityBrainComponentName, IEntityBrainComponent::CID, EntityBrainComponentFactory) == S_OK);
   Verify(RegisterOldStyleComponentFactory(g_entityBoxSelectionIndicatorComponentName, IEntityBoxSelectionIndicatorComponent::CID, EntityBoxSelectionIndicatorComponentFactory) == S_OK);
}


///////////////////////////////////////////////////////////////////////////////
