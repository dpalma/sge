///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "modelutils.h"

#include "render/renderapi.h"

#include "tech/color.h"
#include "tech/globalobj.h"
#include "tech/matrix34.h"
#include "tech/resourceapi.h"

#include <cfloat>

#include "tech/dbgalloc.h" // must be last header


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
// CLASS: cBasicModelRenderer
//

///////////////////////////////////////

cBasicModelRenderer::cBasicModelRenderer(IModel * pModel)
 : m_pModel(CTAddRef(pModel))
{
}

///////////////////////////////////////

cBasicModelRenderer::~cBasicModelRenderer()
{
}

///////////////////////////////////////

tResult cBasicModelRenderer::GetBoundingBox(tAxisAlignedBox * pBBox) const
{
   if (pBBox == NULL)
   {
      return E_POINTER;
   }
   *pBBox = m_bbox;
   return S_OK;
}

///////////////////////////////////////

void cBasicModelRenderer::Render()
{
   if (!m_pModel)
   {
      // TODO: Maybe use a generated stand-in model to indicate loading failure
      return;
   }

   UseGlobal(Renderer);

   uint nVertices = 0;
   const sModelVertex * pVertices = NULL;
   if (m_pModel->GetVertices(&nVertices, &pVertices) == S_OK)
   {
      pRenderer->SetVertexFormat(g_modelVert, _countof(g_modelVert));
      pRenderer->SubmitVertices(const_cast<sModelVertex *>(pVertices), nVertices);
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


///////////////////////////////////////////////////////////////////////////////

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


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAnimatedModelRenderer
//

///////////////////////////////////////

cAnimatedModelRenderer::cAnimatedModelRenderer(const tChar * pszModel)
 : m_model(pszModel ? pszModel : _T(""))
 , m_pModel(NULL)
{
}

///////////////////////////////////////

cAnimatedModelRenderer::~cAnimatedModelRenderer()
{
}

///////////////////////////////////////

tResult cAnimatedModelRenderer::GetBoundingBox(tAxisAlignedBox * pBBox) const
{
   if (pBBox == NULL)
   {
      return E_POINTER;
   }
   if (!m_pModel)
   {
      return E_FAIL;
   }
   *pBBox = m_bbox;
   return S_OK;
}

///////////////////////////////////////

void cAnimatedModelRenderer::Update(double elapsedTime)
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

void cAnimatedModelRenderer::Render()
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

tResult cAnimatedModelRenderer::SetAnimation(eModelAnimationType type)
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
