///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "scenemesh.h"

#include "meshapi.h"
#include "skeleton.h"
#include "animation.h"

#include "renderapi.h"

#include "vec4.h"
#include "matrix4.h"
#include "resourceapi.h"
#include "globalobj.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneMesh
//

ISceneEntity * SceneEntityCreate(IMesh * pMesh)
{
   return static_cast<ISceneEntity *>(new cSceneMesh(pMesh));
}

///////////////////////////////////////

cSceneMesh::cSceneMesh(IMesh * pMesh)
 : m_simClient(this),
   m_animationTime(0),
   m_boundingSphereRadius(0),
   m_pSceneEntity(SceneEntityCreate()),
   m_pMesh(pMesh)
{
   Assert(pMesh != NULL);
   if (pMesh != NULL)
   {
      pMesh->AddRef();
   }

   PostRead();

   tVec3 maxs, mins;
   m_pMesh->GetAABB(&maxs, &mins);

   tVec3 diff = maxs - mins;

   m_boundingSphereRadius = 0.5f * Max(diff.x, Max(diff.y, diff.z));

   UseGlobal(Sim);
   pSim->Connect(&m_simClient);
}

///////////////////////////////////////

cSceneMesh::~cSceneMesh()
{
   UseGlobal(Sim);
   pSim->Disconnect(&m_simClient);
}

///////////////////////////////////////

void cSceneMesh::Animate(double elapsedTime)
{
   cAutoIPtr<ISkeleton> pSkeleton;
   cAutoIPtr<IKeyFrameAnimation> pAnimation;
   if (m_pMesh->GetSkeleton(&pSkeleton) == S_OK
      && pSkeleton->GetAnimation(&pAnimation) == S_OK)
   {
      tTime period = pAnimation->GetPeriod();
      m_animationTime += elapsedTime;
      while (m_animationTime > period)
      {
         m_animationTime -= period;
      }
      pSkeleton->GetBoneMatrices(m_animationTime, &m_boneMatrices);
   }
}

///////////////////////////////////////

cSceneMesh::cSimClient::cSimClient(cSceneMesh * pOuter)
 : m_pOuter(pOuter)
{
}

///////////////////////////////////////

void cSceneMesh::cSimClient::OnFrame(double elapsedTime)
{
   Assert(m_pOuter != NULL);
   m_pOuter->Animate(elapsedTime);
}

///////////////////////////////////////

tResult cSceneMesh::PostRead()
{
   cAutoIPtr<ISkeleton> pSkeleton;
   if (m_pMesh->GetSkeleton(&pSkeleton) == S_OK)
   {
      m_boneMatrices.resize(pSkeleton->GetBoneCount());

      tMatrices inverses(pSkeleton->GetBoneCount());

      for (uint i = 0; i < inverses.size(); i++)
      {
         MatrixInvert(pSkeleton->GetBoneWorldTransform(i).m, inverses[i].m);
      }

      if (m_pMesh)
      {
         cAutoIPtr<IVertexBuffer> pVB;
         cAutoIPtr<IVertexDeclaration> pVertexDecl;

         // TODO: Handle sub-meshes too (not all meshes have a single shared vertex buffer)

         if (m_pMesh->GetVertexBuffer(&pVB) == S_OK)
         {
            sVertexElement elements[256];
            int nElements = _countof(elements);
            uint vertexSize;

            if (pVB->GetVertexDeclaration(&pVertexDecl) == S_OK
               && pVertexDecl->GetElements(elements, &nElements) == S_OK
               && pVertexDecl->GetVertexSize(&vertexSize) == S_OK)
            {
               uint positionOffset, normalOffset, indexOffset;

               for (int i = 0; i < nElements; i++)
               {
                  switch (elements[i].usage)
                  {
                     case kVDU_Position:
                     {
                        positionOffset = elements[i].offset;
                        break;
                     }

                     case kVDU_Normal:
                     {
                        normalOffset = elements[i].offset;
                        break;
                     }

                     case kVDU_Index:
                     {
                        indexOffset = elements[i].offset;
                        break;
                     }
                  }
               }

               // transform all vertices by the inverse of the affecting bone's absolute matrix
               byte * pVertexData;
               if (m_pMesh->LockVertexBuffer(kBL_Default, (void**)&pVertexData) == S_OK)
               {
                  for (uint i = 0; i < m_pMesh->GetVertexCount(); i++)
                  {
                     byte * pVertexBase = pVertexData + (i * vertexSize);

                     float * pPosition = reinterpret_cast<float *>(pVertexBase + positionOffset);
                     float * pNormal = reinterpret_cast<float *>(pVertexBase + normalOffset);
                     const float * pIndex = reinterpret_cast<const float *>(pVertexBase + indexOffset);

                     int index = (int)*pIndex;

                     // TODO: No size-checking is done for position and normal members
                     // (i.e., float1, float2, float3, etc.)

                     if (index >= 0)
                     {
                        tVec4 normal(pNormal[0],pNormal[1],pNormal[2],1);
                        tVec4 position(pPosition[0],pPosition[1],pPosition[2],1);

                        tVec4 nprime;
                        inverses[index].Transform(normal, &nprime);
                        memcpy(pNormal, nprime.v, 3 * sizeof(float));

                        tVec4 vprime;
                        inverses[index].Transform(position, &vprime);
                        memcpy(pPosition, vprime.v, 3 * sizeof(float));
                     }
                  }

                  m_pMesh->UnlockVertexBuffer();
               }
            }
         }
      }

      Animate(0);
   }

   return S_OK;
}

///////////////////////////////////////

IMesh * cSceneMesh::AccessMesh()
{
   return m_pMesh;
}

///////////////////////////////////////

void cSceneMesh::Render(IRenderDevice * pRenderDevice)
{
   if (m_pMesh != NULL)
   {
      if (!m_boneMatrices.empty())
      {
         pRenderDevice->SetBlendMatrices(m_boneMatrices.size(), &m_boneMatrices[0]);
      }
      m_pMesh->Render(pRenderDevice);
   }
}

///////////////////////////////////////

float cSceneMesh::GetBoundingRadius() const
{
   return m_boundingSphereRadius;
}

///////////////////////////////////////////////////////////////////////////////
