///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "ms3dmesh.h"

#include "skeleton.h"
#include "animation.h"

#include "renderapi.h"
#include "material.h"

#include "techmath.h"
#include "vec4.h"

#include <GL/gl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////

// REFERENCES
// http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=31
// http://rsn.gamedev.net/tutorials/ms3danim.asp

// used as the 4th dimension when a 3D vector is passed to a 4D function
static const tVec4::value_type k4thDimension = 1;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dMesh
//

cMs3dMesh::cMs3dMesh()
 : m_pInnerMesh(MeshCreate()),
   m_bPrepared(false)
{
}

cMs3dMesh::~cMs3dMesh()
{
}

void cMs3dMesh::GetAABB(tVec3 * pMaxs, tVec3 * pMins) const
{
   if (m_pInnerMesh)
      m_pInnerMesh->GetAABB(pMaxs, pMins);
}

void cMs3dMesh::Render(IRenderDevice * pRenderDevice) const
{
   if (!m_bPrepared)
   {
      const_cast<cMs3dMesh *>(this)->Prepare();
      m_bPrepared = true;
   }

   if (m_pInnerMesh)
   {
      pRenderDevice->SetBlendMatrices(m_boneMatrices.size(), &m_boneMatrices[0]);
      m_pInnerMesh->Render(pRenderDevice);
   }
}

uint cMs3dMesh::GetVertexCount() const
{
   if (m_pInnerMesh)
      return m_pInnerMesh->GetVertexCount();
   else
      return 0;
}

tResult cMs3dMesh::GetVertexBuffer(IVertexBuffer * * ppVertexBuffer)
{
   return m_pInnerMesh->GetVertexBuffer(ppVertexBuffer);
}

tResult cMs3dMesh::LockVertexBuffer(uint lock, void * * ppData)
{
   return m_pInnerMesh->LockVertexBuffer(lock, ppData);
}

tResult cMs3dMesh::UnlockVertexBuffer()
{
   return m_pInnerMesh->UnlockVertexBuffer();
}

tResult cMs3dMesh::AddMaterial(IMaterial * pMaterial)
{
   return m_pInnerMesh->AddMaterial(pMaterial);
}

tResult cMs3dMesh::FindMaterial(const char * pszName, IMaterial * * ppMaterial) const
{
   return m_pInnerMesh->FindMaterial(pszName, ppMaterial);
}

uint cMs3dMesh::GetMaterialCount() const
{
   if (m_pInnerMesh)
      return m_pInnerMesh->GetMaterialCount();
   else
      return 0;
}

tResult cMs3dMesh::GetMaterial(uint index, IMaterial * * ppMaterial) const
{
   return m_pInnerMesh->GetMaterial(index, ppMaterial);
}

tResult cMs3dMesh::AddSubMesh(ISubMesh * pSubMesh)
{
   return m_pInnerMesh->AddSubMesh(pSubMesh);
}

uint cMs3dMesh::GetSubMeshCount() const
{
   return m_pInnerMesh->GetSubMeshCount();
}

tResult cMs3dMesh::GetSubMesh(uint index, ISubMesh * * ppSubMesh) const
{
   return m_pInnerMesh->GetSubMesh(index, ppSubMesh);
}

tResult cMs3dMesh::AttachSkeleton(ISkeleton * pSkeleton)
{
   return m_pInnerMesh->AttachSkeleton(pSkeleton);
}

tResult cMs3dMesh::GetSkeleton(ISkeleton * * ppSkeleton)
{
   return m_pInnerMesh->GetSkeleton(ppSkeleton);
}

tResult cMs3dMesh::Load(const char * pszMesh, IRenderDevice * pRenderDevice, IResourceManager * pResourceManager)
{
   SafeRelease(m_pInnerMesh);
   m_pInnerMesh = MeshLoad(pResourceManager, pRenderDevice, pszMesh);
   if (!m_pInnerMesh)
   {
      return E_FAIL;
   }
   m_bPrepared = false;
   return S_OK;
}

void cMs3dMesh::Prepare()
{
   cAutoIPtr<ISkeleton> pSkeleton;
   if (GetSkeleton(&pSkeleton) == S_OK)
   {
      m_boneMatrices.resize(pSkeleton->GetBoneCount());

      tMatrices inverses(pSkeleton->GetBoneCount());

      for (int i = 0; i < inverses.size(); i++)
      {
         MatrixInvert(pSkeleton->GetBoneWorldTransform(i).m, inverses[i].m);
      }

      if (m_pInnerMesh)
      {
         cAutoIPtr<IVertexBuffer> pVB;
         cAutoIPtr<IVertexDeclaration> pVertexDecl;

         // TODO: Handle sub-meshes too (not all meshes have a single shared vertex buffer)

         if (m_pInnerMesh->GetVertexBuffer(&pVB) == S_OK)
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
               if (m_pInnerMesh->LockVertexBuffer(kBL_Default, (void**)&pVertexData) == S_OK)
               {
                  for (uint i = 0; i < m_pInnerMesh->GetVertexCount(); i++)
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
                        tVec4 nprime;
                        inverses[index].Transform(tVec4(pNormal[0],pNormal[1],pNormal[2],1), &nprime);
                        memcpy(pNormal, nprime.v, 3 * sizeof(float));

                        tVec4 vprime;
                        inverses[index].Transform(tVec4(pPosition[0],pPosition[1],pPosition[2],1), &vprime);
                        memcpy(pPosition, vprime.v, 3 * sizeof(float));
                     }
                  }

                  m_pInnerMesh->UnlockVertexBuffer();
               }
            }
         }
      }

      SetFrame(0);
   }
}

void cMs3dMesh::SetFrame(float percent)
{
   cAutoIPtr<ISkeleton> pSkeleton;
   if (GetSkeleton(&pSkeleton) == S_OK)
   {
      cAutoIPtr<IKeyFrameAnimation> pAnimation;
      if (pSkeleton->GetAnimation(&pAnimation) == S_OK)
      {
         pSkeleton->GetBoneMatrices(pAnimation->GetPeriod() * percent, &m_boneMatrices);
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
