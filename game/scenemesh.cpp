///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "scenemesh.h"
#include "gcommon.h"
#include "ggl.h"

#include "mesh.h"
#include "skeleton.h"

#include "render.h"

#include "vec4.h"
#include "matrix4.h"
#include "resmgr.h"
#include "globalobj.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneMesh
//

///////////////////////////////////////

cSceneMesh::cSceneMesh()
 : m_percent(0)
{
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

bool cSceneMesh::SetMesh(const char * pszMesh)
{
   SafeRelease(m_pMesh);

   UseGlobal(ResourceManager);

   m_pMesh = MeshLoad(pResourceManager, AccessRenderDevice(), pszMesh);

   if (m_pMesh != NULL)
   {
      PostRead();

      tVec3 maxs, mins;
      m_pMesh->GetAABB(&maxs, &mins);

      tVec3 diff = maxs - mins;

      m_centroid = mins + (diff * 0.5f);

      m_bounds.SetRadius(0.5f * Max(diff.x, Max(diff.y, diff.z)));

      return true;
   }
   
   return false;
}

///////////////////////////////////////

#define GetOuter(Class, Member) ((Class *)((byte *)this - (byte *)&((Class *)NULL)->Member))

void cSceneMesh::cSimClient::OnFrame(double elapsedTime)
{
   cSceneMesh * pSceneMesh = GetOuter(cSceneMesh, m_simClient);
   pSceneMesh->m_percent += elapsedTime;
   if (pSceneMesh->m_percent > 1.0f)
      pSceneMesh->m_percent = 0;
   pSceneMesh->SetFrame(pSceneMesh->m_percent);
}

///////////////////////////////////////

tResult cSceneMesh::PostRead()
{
   cAutoIPtr<ISkeleton> pSkeleton;
   if (m_pMesh->GetSkeleton(&pSkeleton) == S_OK)
   {
      m_boneMatrices.resize(pSkeleton->GetBoneCount());

      tMatrices inverses(pSkeleton->GetBoneCount());

      for (int i = 0; i < inverses.size(); i++)
      {
         MatrixInvert(pSkeleton->GetBoneWorldTransform(i), &inverses[i]);
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

            if (pVB->GetVertexDeclaration(&pVertexDecl) == S_OK
               && pVertexDecl->GetElements(elements, &nElements) == S_OK)
            {
               uint vertexSize = GetVertexSize(elements, nElements);

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
                        tVec4 nprime = inverses[index].Transform(tVec4(pNormal[0],pNormal[1],pNormal[2],1));
                        memcpy(pNormal, nprime.v, 3 * sizeof(float));

                        tVec4 vprime = inverses[index].Transform(tVec4(pPosition[0],pPosition[1],pPosition[2],1));
                        memcpy(pPosition, vprime.v, 3 * sizeof(float));
                     }
                  }

                  m_pMesh->UnlockVertexBuffer();
               }
            }
         }
      }

      SetFrame(0);
   }

   return S_OK;
}

///////////////////////////////////////

void cSceneMesh::SetFrame(float percent)
{
   cAutoIPtr<ISkeleton> pSkeleton;
   if (m_pMesh->GetSkeleton(&pSkeleton) == S_OK)
   {
      pSkeleton->GetBoneMatrices(percent, &m_boneMatrices);
   }
}

///////////////////////////////////////

IMesh * cSceneMesh::AccessMesh()
{
   return m_pMesh;
}

///////////////////////////////////////

void cSceneMesh::Render()
{
   if (m_pMesh != NULL)
   {
      glPushMatrix();
      glMultMatrixf(GetTransform().m);

      AccessRenderDevice()->SetBlendMatrices(m_boneMatrices.size(), &m_boneMatrices[0]);
      m_pMesh->Render(AccessRenderDevice());

      tChildren::iterator iter;
      for (iter = m_children.begin(); iter != m_children.end(); iter++)
      {
         (*iter)->Render();
      }
      
      glPopMatrix();
   }
}

///////////////////////////////////////

const cBoundingVolume * cSceneMesh::GetBoundingVolume() const
{
   m_bounds.SetCenter(GetTranslation() + m_centroid);
   return &m_bounds;
}

///////////////////////////////////////////////////////////////////////////////
