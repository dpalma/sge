///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "ms3dmesh.h"

#include "skeleton.h"
#include "animation.h"

#include "render.h"
#include "material.h"

#include "techmath.h"
#include "vec4.h"

#include <GL/gl.h>
#include "GL/glext.h"

#include <Cg/cg.h>
#include <Cg/cgGL.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// REFERENCES
// http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=31
// http://rsn.gamedev.net/tutorials/ms3danim.asp

///////////////////////////////////////////////////////////////////////////////

// used as the 4th dimension when a 3D vector is passed to a 4D function
static const tVec4::value_type k4thDimension = 1;

CGcontext g_cgContext = NULL;
ulong g_nCgContextRefs = 0;

CGprofile g_cgProfile = CG_PROFILE_UNKNOWN;

static CGcontext GetCgContext()
{
   if (g_cgContext == NULL)
   {
      g_cgContext = cgCreateContext();
   }
   ++g_nCgContextRefs;
   return g_cgContext;
}

static void ReleaseCgContext()
{
   if (--g_nCgContextRefs == 0)
   {
      cgDestroyContext(g_cgContext);
      g_cgContext = NULL;
   }
}

///////////////////////////////////////////////////////////////////////////////

void cgErrorCallback()
{
   CGerror lastError = cgGetError();

   if (lastError)
   {
      DebugMsg(cgGetErrorString(lastError));

      const char * pszListing = cgGetLastListing(g_cgContext);
      if (pszListing != NULL)
      {
         DebugMsg1("   %s\n", pszListing);
      }
   }
}

///////////////////////////////////////////////////////////////////////////////

static void MatrixFromAngles(tVec3 angles, tMatrix4 * pMatrix)
{
   tMatrix4 rotX, rotY, rotZ, temp1, temp2;
   MatrixRotateX(Rad2Deg(angles.x), &rotX);
   MatrixRotateY(Rad2Deg(angles.y), &rotY);
   MatrixRotateZ(Rad2Deg(angles.z), &rotZ);
   temp1 = rotZ;
   temp2 = temp1 * rotY;
   *pMatrix = temp2 * rotX;
}

///////////////////////////////////////////////////////////////////////////////

static char * GetResource(const char * pResId, const char * pResType)
{
   HRSRC hR = FindResource(AfxGetInstanceHandle(), pResId, pResType);
   if (hR)
   {
      uint resSize = SizeofResource(AfxGetInstanceHandle(), hR);
      HGLOBAL hG = LoadResource(AfxGetInstanceHandle(), hR);
      if (hG)
      {
         void * pResData = LockResource(hG);
         if (pResData)
         {
            char * pszContents = new char[resSize + 1];
            strcpy(pszContents, (const char *)pResData);
            return pszContents;
         }
      }
   }
   return NULL;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dMesh
//

cMs3dMesh::cMs3dMesh()
 : m_pInnerMesh(MeshCreate()),
   m_program(NULL),
   m_modelViewProjParam(NULL)
{
}

cMs3dMesh::~cMs3dMesh()
{
   if (m_program != NULL)
   {
      cgDestroyProgram(m_program);
      m_program = NULL;
   }

   ReleaseCgContext();
}

void cMs3dMesh::GetAABB(tVec3 * pMaxs, tVec3 * pMins) const
{
   if (m_pInnerMesh)
      m_pInnerMesh->GetAABB(pMaxs, pMins);
}

void cMs3dMesh::Render(IRenderDevice * pRenderDevice) const
{
   pRenderDevice->SetBlendMatrices(m_boneMatrices.size(), &m_boneMatrices[0]);
   if (m_pInnerMesh)
      m_pInnerMesh->Render(pRenderDevice);
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
   if (m_pInnerMesh)
   {
      return PostRead();
   }

   return E_FAIL;
}

tResult cMs3dMesh::PostRead()
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
                        tVec4 nprime = inverses[index].Transform(tVec4(pNormal[0],pNormal[1],pNormal[2],1));
                        memcpy(pNormal, nprime.v, 3 * sizeof(float));

                        tVec4 vprime = inverses[index].Transform(tVec4(pPosition[0],pPosition[1],pPosition[2],1));
                        memcpy(pPosition, vprime.v, 3 * sizeof(float));
                     }
                  }

                  m_pInnerMesh->UnlockVertexBuffer();
               }
            }
         }
      }

      SetFrame(0);

      cgSetErrorCallback(cgErrorCallback);

      g_cgProfile = cgGLGetLatestProfile(CG_GL_VERTEX);

      if (g_cgProfile != CG_PROFILE_UNKNOWN)
      {
         char * pszProgram = GetResource("ms3dmeshanim.cg", "CG");

         if (pszProgram != NULL)
         {
            m_program = cgCreateProgram(GetCgContext(), CG_SOURCE, pszProgram,
               g_cgProfile, NULL, NULL);

            delete [] pszProgram;

            if (m_program != NULL)
            {
               cgGLLoadProgram(m_program);

               m_modelViewProjParam = cgGetNamedParameter(m_program, "modelViewProj");
            }
         }
      }
   }

   return S_OK;
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

void cMs3dMesh::RenderVertexProgram() const
{
   cgGLBindProgram(m_program);

   cgGLEnableProfile(g_cgProfile);

   cgGLSetStateMatrixParameter(m_modelViewProjParam, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);

//   tGroups::const_iterator iter;
//   for (iter = m_groups.begin(); iter != m_groups.end(); iter++)
//   {
//      glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
//
//      cAutoIPtr<IMaterial> pMaterial;
//      if ((iter->GetMaterialIndex() > -1) 
//         && (GetMaterial(iter->GetMaterialIndex(), &pMaterial) == S_OK))
//      {
//         GlMaterial(pMaterial);
//      }
//
//      glBegin(GL_TRIANGLES);
//
//      const std::vector<uint16> & tris = iter->GetTriangleIndices();
//      std::vector<uint16>::const_iterator iterTris;
//      for (iterTris = tris.begin(); iterTris != tris.end(); iterTris++)
//      {
//         const ms3d_triangle_t & tri = m_triangles[*iterTris];
//
//         for (int k = 0; k < 3; k++)
//         {
//            const ms3d_vertex_t & vk = m_vertices[tri.vertexIndices[k]];
//            if (vk.boneId != -1)
//            {
//               //const tMatrix4 & m = m_joints[vk.boneId].GetFinalMatrix();
//
//               glTexCoord2f(tri.s[k], 1.0f - tri.t[k]);
//               glNormal3fv(tri.vertexNormals[k]);
//               glVertex3fv(vk.vertex);
//            }
//         }
//      }
//
//      glEnd();
//
//      glPopAttrib();
//   }

   cgGLDisableProfile(g_cgProfile);
}

///////////////////////////////////////////////////////////////////////////////
