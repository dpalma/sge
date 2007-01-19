///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entitycomponents.h"

#include "render/renderapi.h"

#include "tech/color.h"
#include "tech/globalobj.h"
#include "tech/matrix34.h"
#include "tech/multivar.h"
#include "tech/resourceapi.h"
#include "tech/statemachinetem.h"

#include <tinyxml.h>

#include <algorithm>
#include <cfloat>

#include "tech/dbgalloc.h" // must be last header

#pragma warning(disable:4355) // 'this' : used in base member initializer list

///////////////////////////////////////////////////////////////////////////////

#define IsFlagSet(f, b) (((f)&(b))==(b))

extern tResult ModelEntityCreate(tEntityId id, const tChar * pszModel, const tVec3 & position, IEntity * * ppEntity);

extern "C" DECLSPEC_DLLIMPORT short STDCALL GetAsyncKeyState(int);


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

const sVertexElement g_modelVert[] =
{
   { kVEU_TexCoord,  kVET_Float2,   0, 0 },
   { kVEU_Normal,    kVET_Float3,   0, 2 * sizeof(float) },
   { kVEU_Position,  kVET_Float3,   0, 5 * sizeof(float) },
   { kVEU_Index,     kVET_Float1,   0, 8 * sizeof(float) },
};

const sVertexElement g_blendedVert[] =
{
   { kVEU_TexCoord,  kVET_Float2,   0, 0 },
   { kVEU_Normal,    kVET_Float3,   0, 2 * sizeof(float) },
   { kVEU_Position,  kVET_Float3,   0, 5 * sizeof(float) },
};


////////////////////////////////////////////////////////////////////////////////

static void CalculateBBox(uint nVertices, const sModelVertex * pVertices, tAxisAlignedBox * pBBox)
{
   tVec3 mins(FLT_MAX, FLT_MAX, FLT_MAX), maxs(-FLT_MAX, -FLT_MAX, -FLT_MAX);
   const sModelVertex * pVertex = pVertices;
   for (uint i = 0; i < nVertices; ++i, ++pVertex)
   {
      if (pVertex->pos.x < mins.x)
      {
         mins.x = pVertex->pos.x;
      }
      if (pVertex->pos.y < mins.y)
      {
         mins.y = pVertex->pos.y;
      }
      if (pVertex->pos.z < mins.z)
      {
         mins.z = pVertex->pos.z;
      }
      if (pVertex->pos.x > maxs.x)
      {
         maxs.x = pVertex->pos.x;
      }
      if (pVertex->pos.y > maxs.y)
      {
         maxs.y = pVertex->pos.y;
      }
      if (pVertex->pos.z > maxs.z)
      {
         maxs.z = pVertex->pos.z;
      }
   }
   *pBBox = tAxisAlignedBox(mins, maxs);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityModelRenderer
//

///////////////////////////////////////

cEntityModelRenderer::cEntityModelRenderer(const tChar * pszModel)
 : m_model(pszModel ? pszModel : _T(""))
 , m_pModel(NULL)
{
}

///////////////////////////////////////

cEntityModelRenderer::~cEntityModelRenderer()
{
}

///////////////////////////////////////

tResult cEntityModelRenderer::GetBoundingBox(tAxisAlignedBox * pBBox) const
{
   if (pBBox == NULL)
   {
      return E_POINTER;
   }
   *pBBox = m_bbox;
   return S_OK;
}

///////////////////////////////////////

static void ApplyJointMatrices(uint nVertices, const sModelVertex * pVertices,
                               const std::vector<tMatrix34> & matrices,
                               tBlendedVertices * pBlendedVertices)
{
   pBlendedVertices->resize(nVertices);

   const sModelVertex * pV = pVertices;
   for (uint i = 0; i < nVertices; ++i, ++pV)
   {
      sBlendedVertex & v = (*pBlendedVertices)[i];
      v.u = pV->u;
      v.v = pV->v;
      // TODO: call them bones or joints???
      int iJoint = FloatToInt(pV->bone);
      if (iJoint < 0)
      {
         v.normal = pV->normal;
         v.pos = pV->pos;
      }
      else
      {
         const tMatrix34 & m = matrices[iJoint];
         m.Transform(pV->normal, &v.normal);
         m.Transform(pV->pos, &v.pos);
      }
   }
}

void cEntityModelRenderer::Update(double elapsedTime)
{
   UseGlobal(ResourceManager);
   IModel * pModel = NULL;
   if (pResourceManager->Load(m_model.c_str(), kRT_Model, NULL, (void**)&pModel) != S_OK)
   {
      m_pModel = static_cast<IModel*>(NULL);
      return;
   }

   AssertMsg(pModel != NULL, _T("Should have returned by now if ResourceManager->Load failed\n"));

   if (pModel != m_pModel)
   {
      m_pModel = CTAddRef(pModel);

      cAutoIPtr<IModelSkeleton> pSkeleton;
      if (m_pModel->GetSkeleton(&pSkeleton) == S_OK)
      {
         size_t nJoints = 0;
         if (pSkeleton->GetJointCount(&nJoints) == S_OK && nJoints > 0)
         {
            m_blendMatrices.resize(nJoints);

            if (ModelAnimationControllerCreate(pSkeleton, &m_pAnimController) == S_OK)
            {
               SetAnimation(kMAT_Idle);
            }
         }
         else
         {
            m_blendMatrices.clear();
         }
      }

      uint nVertices = 0;
      const sModelVertex * pVertices = NULL;
      if (m_pModel->GetVertices(&nVertices, &pVertices) == S_OK)
      {
         CalculateBBox(nVertices, pVertices, &m_bbox);
      }
   }

   if (!!m_pAnimController)
   {
      if (m_pAnimController->Advance(elapsedTime, m_blendMatrices.size(), &m_blendMatrices[0]) == S_OK)
      {
         uint nVertices = 0;
         const sModelVertex * pVertices = NULL;
         if (m_pModel->GetVertices(&nVertices, &pVertices) == S_OK)
         {
            ApplyJointMatrices(nVertices, pVertices, m_blendMatrices, &m_blendedVerts);
         }
      }
   }
}

///////////////////////////////////////

void cEntityModelRenderer::Render()
{
   UseGlobal(Renderer);

   if (!m_blendedVerts.empty())
   {
      pRenderer->SetVertexFormat(g_blendedVert, _countof(g_blendedVert));
      pRenderer->SubmitVertices(&m_blendedVerts[0], m_blendedVerts.size());
   }
   else
   {
      if (!m_pModel)
      {
         // TODO: Maybe use a generated stand-in model to indicate loading failure
         return;
      }
      uint nVertices = 0;
      const sModelVertex * pVertices = NULL;
      if (m_pModel->GetVertices(&nVertices, &pVertices) == S_OK)
      {
         pRenderer->SetVertexFormat(g_modelVert, _countof(g_modelVert));
         pRenderer->SubmitVertices(const_cast<sModelVertex *>(pVertices), nVertices);
      }
   }

   pRenderer->SetIndexFormat(kIF_16Bit);

   const uint16 * pIndices = NULL;
   if (m_pModel->GetIndices(NULL, &pIndices) == S_OK)
   {
      uint nMeshes = 0;
      const sModelMesh * pMeshes = NULL;
      if (m_pModel->GetMeshes(&nMeshes, &pMeshes) == S_OK)
      {
         const sModelMesh * pMesh = pMeshes;
         for (uint i = 0; i < nMeshes; ++i, ++pMesh)
         {
            if (pMesh->materialIndex >= 0)
            {
               const sModelMaterial * pM = m_pModel->AccessMaterial(pMesh->materialIndex);
               if (pM != NULL)
               {
                  pRenderer->SetDiffuseColor(pM->diffuse);
                  pRenderer->SetTexture(0, pM->szTexture);
               }
            }
            pRenderer->Render(static_cast<ePrimitiveType>(pMesh->primitive),
               pIndices + pMesh->indexStart, pMesh->nIndices);
         }
      }
   }
}

///////////////////////////////////////

tResult cEntityModelRenderer::SetAnimation(eModelAnimationType type)
{
   if (!m_pAnimController)
   {
      return E_FAIL;
   }
   cAutoIPtr<IModelSkeleton> pSkel;
   if (m_pModel->GetSkeleton(&pSkel) == S_OK)
   {
      cAutoIPtr<IModelAnimation> pAnim;
      if (pSkel->GetAnimation(type, &pAnim) == S_OK)
      {
         return m_pAnimController->SetAnimation(pAnim);
      }
   }
   return E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityRenderComponent
//

///////////////////////////////////////

cEntityRenderComponent::cEntityRenderComponent(const tChar * pszModel)
 : m_mainModel(pszModel)
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
}

///////////////////////////////////////

tResult cEntityRenderComponent::SetAnimation(eModelAnimationType type)
{
   return m_mainModel.SetAnimation(type);
}

///////////////////////////////////////

void cEntityRenderComponent::Update(double elapsedTime)
{
   m_mainModel.Update(elapsedTime);
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
   if ((result = AIAgentCreate(&pAgent)) != S_OK)
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
