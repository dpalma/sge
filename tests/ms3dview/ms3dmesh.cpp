///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "ms3dmesh.h"

#include "FileSpec.h"
#include "ReadWriteAPI.h"
#include "TechMath.h"
#include "animation.h"
#include "vec4.h"

#include "render.h"
#include "material.h"
#include "image.h"
#include "color.h"

#include <cfloat>
#include <string>
#include <map>

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

extern PFNGLVERTEXWEIGHTFEXTPROC glVertexWeightfEXT;
extern PFNGLVERTEXWEIGHTFVEXTPROC glVertexWeightfvEXT;
extern PFNGLVERTEXWEIGHTPOINTEREXTPROC glVertexWeightPointerEXT;

///////////////////////////////////////////////////////////////////////////////

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

// used as the 4th dimension when a 3D vector is passed to a 4D function
static const tVec4::value_type k4thDimension = 1;

static const char g_MS3D[] = "MS3D000000";

///////////////////////////////////////////////////////////////////////////////

void cgErrorCallback()
{
   CGerror lastError = cgGetError();

   if (lastError)
   {
      DebugPrintf(NULL, 0, cgGetErrorString(lastError));

      const char * pszListing = cgGetLastListing(g_cgContext);
      if (pszListing != NULL)
      {
         DebugPrintf(NULL, 0, "   %s\n", pszListing);
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
//
// CLASS: cMs3dGroup
//

///////////////////////////////////////

cMs3dGroup::cMs3dGroup()
 : materialIndex(-1)
{
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cReadWriteOps<cMs3dGroup>
//

template <>
class cReadWriteOps<cMs3dGroup>
{
public:
   static tResult Read(IReader * pReader, cMs3dGroup * pGroup);
};

///////////////////////////////////////

tResult cReadWriteOps<cMs3dGroup>::Read(IReader * pReader, cMs3dGroup * pGroup)
{
   Assert(pReader != NULL);
   Assert(pGroup != NULL);

   tResult result = E_FAIL;

   do
   {
      byte flags; // SELECTED | HIDDEN
      if (pReader->Read(&flags, sizeof(flags)) != S_OK)
         break;

      if (pReader->Read(pGroup->name, sizeof(pGroup->name)) != S_OK)
         break;

      uint16 nTriangles;
      if (pReader->Read(&nTriangles, sizeof(nTriangles)) != S_OK)
         break;

      pGroup->triangleIndices.resize(nTriangles);

      if (pReader->Read(&pGroup->triangleIndices[0], pGroup->triangleIndices.size() * sizeof(uint16)) != S_OK)
         break;

      if (pReader->Read(&pGroup->materialIndex, sizeof(pGroup->materialIndex)) != S_OK)
         break;

      result = S_OK;
   }
   while (0);

   return result;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dJoint
//

///////////////////////////////////////

cMs3dJoint::cMs3dJoint()
 : m_iParentJoint(-1)
{
   name[0] = 0;
   parentName[0] = 0;

   m_local.Identity();
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cReadWriteOps<cMs3dJoint>
//

template <>
class cReadWriteOps<cMs3dJoint>
{
public:
   static tResult Read(IReader * pReader, cMs3dJoint * pJoint);
};

///////////////////////////////////////

tResult cReadWriteOps<cMs3dJoint>::Read(IReader * pReader, cMs3dJoint * pJoint)
{
   Assert(pReader != NULL);
   Assert(pJoint != NULL);

   tResult result = E_FAIL;

   do
   {
      byte flags; // SELECTED | DIRTY
      if (pReader->Read(&flags, sizeof(flags)) != S_OK)
         break;

      if (pReader->Read(pJoint->name, sizeof(pJoint->name)) != S_OK)
         break;

      if (pReader->Read(pJoint->parentName, sizeof(pJoint->parentName)) != S_OK)
         break;

      float rotation[3], position[3];
      if (pReader->Read(rotation, sizeof(rotation)) != S_OK ||
         pReader->Read(position, sizeof(position)) != S_OK)
         break;

      tMatrix4 mt, mr;
      MatrixTranslate(position[0], position[1], position[2], &mt);
      MatrixFromAngles(tVec3(rotation), &mr);
      pJoint->m_local = mt * mr;

      uint16 nKeyFramesRot, nKeyFramesTrans;

      if (pReader->Read(&nKeyFramesRot, sizeof(nKeyFramesRot)) != S_OK)
         break;
      if (pReader->Read(&nKeyFramesTrans, sizeof(nKeyFramesTrans)) != S_OK)
         break;

      if (nKeyFramesRot != nKeyFramesTrans)
      {
         DebugMsg2("Have %d rotation and %d translation key frames\n", nKeyFramesRot, nKeyFramesTrans);
         break;
      }

      pJoint->m_rotationKeys.resize(nKeyFramesRot);
      if (pReader->Read(&pJoint->m_rotationKeys[0], pJoint->m_rotationKeys.size() * sizeof(ms3d_keyframe_rot_t)) != S_OK)
         break;

      pJoint->m_translationKeys.resize(nKeyFramesTrans);
      if (pReader->Read(&pJoint->m_translationKeys[0], pJoint->m_translationKeys.size() * sizeof(ms3d_keyframe_pos_t)) != S_OK)
         break;

      sKeyFrameVec3 * pTranslationFrames = (sKeyFrameVec3 *)alloca(nKeyFramesTrans * sizeof(sKeyFrameVec3));
      for (unsigned i = 0; i < nKeyFramesTrans; i++)
      {
         pTranslationFrames[i].time = pJoint->m_translationKeys[i].time;
         pTranslationFrames[i].value = tVec3(pJoint->m_translationKeys[i].position);
      }

      sKeyFrameQuat * pRotationFrames = (sKeyFrameQuat *)alloca(nKeyFramesRot * sizeof(sKeyFrameQuat));
      for (i = 0; i < nKeyFramesRot; i++)
      {
         pRotationFrames[i].time = pJoint->m_rotationKeys[i].time;
         pRotationFrames[i].value = QuatFromEulerAngles(tVec3(pJoint->m_rotationKeys[i].rotation));
      }

      if (KeyFrameInterpolatorCreate(
         pJoint->name,
         NULL, 0,
         pRotationFrames, nKeyFramesRot,
         pTranslationFrames, nKeyFramesTrans,
         &pJoint->m_pInterpolator) != S_OK)
      {
         break;
      }

      result = S_OK;
   }
   while (0);

   return result;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dSkeleton
//

cMs3dSkeleton::cMs3dSkeleton()
{
}

cMs3dSkeleton::~cMs3dSkeleton()
{
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dMesh
//

cMs3dMesh::cMs3dMesh()
 : m_pfnRender(RenderSoftware),
   m_bCalculatedAABB(false),
   m_program(NULL),
   m_modelViewProjParam(NULL)
{
}

cMs3dMesh::~cMs3dMesh()
{
   Reset();
}

void cMs3dMesh::GetAABB(tVec3 * pMaxs, tVec3 * pMins) const
{
   if (!m_bCalculatedAABB)
   {
      m_maxs = tVec3(FLT_MIN, FLT_MIN, FLT_MIN);
      m_mins = tVec3(FLT_MAX, FLT_MAX, FLT_MAX);

      tVertices::const_iterator iter;
      for (iter = m_vertices.begin(); iter != m_vertices.end(); iter++)
      {
         if (m_maxs.x < iter->vertex[0])
            m_maxs.x = iter->vertex[0];
         if (m_mins.x > iter->vertex[0])
            m_mins.x = iter->vertex[0];

         if (m_maxs.y < iter->vertex[1])
            m_maxs.y = iter->vertex[1];
         if (m_mins.y > iter->vertex[1])
            m_mins.y = iter->vertex[1];

         if (m_maxs.z < iter->vertex[2])
            m_maxs.z = iter->vertex[2];
         if (m_mins.z > iter->vertex[2])
            m_mins.z = iter->vertex[2];
      }

      m_bCalculatedAABB = true;
   }

   Assert(pMaxs && pMins);
   *pMaxs = m_maxs;
   *pMins = m_mins;
}

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

static char * GetFileContents(const cFileSpec & file)
{
   cAutoIPtr<IReader> pReader = FileCreateReader(file);
   if (pReader)
   {
      pReader->Seek(0, kSO_End);
      long size = pReader->Tell();
      pReader->Seek(0, kSO_Set);

      char * pszContents = new char[size + 1];

      if (pReader->Read(pszContents, size) == S_OK)
      {
         pszContents[size] = 0;
         return pszContents;
      }

      delete [] pszContents;
   }

   return NULL;
}

tResult cMs3dMesh::Read(IReader * pReader, IRenderDevice * pRenderDevice, IResourceManager * pResourceManager)
{
   Assert(m_vertices.empty());
   Assert(m_triangles.empty());
   Assert(m_groups.empty());
   Assert(m_materials.empty());
   Assert(m_joints.empty());

   ms3d_header_t header;
   if (pReader->Read(&header, sizeof(header)) != S_OK ||
      memcmp(g_MS3D, header.id, _countof(header.id)) != 0)
      return E_FAIL;

   uint16 nVertices;
   if (pReader->Read(&nVertices, sizeof(nVertices)) != S_OK
      || nVertices == 0)
      return E_FAIL;

   m_vertices.resize(nVertices);
   if (pReader->Read(&m_vertices[0], m_vertices.size() * sizeof(ms3d_vertex_t)) != S_OK)
      return E_FAIL;

   uint16 nTriangles;
   if (pReader->Read(&nTriangles, sizeof(nTriangles)) != S_OK
      || nTriangles == 0)
      return E_FAIL;

   m_triangles.resize(nTriangles);
   if (pReader->Read(&m_triangles[0], m_triangles.size() * sizeof(ms3d_triangle_t)) != S_OK)
      return E_FAIL;

   uint16 nGroups;
   if (pReader->Read(&nGroups, sizeof(nGroups)) != S_OK)
      return E_FAIL;

   uint i;

   m_groups.resize(nGroups);
   for (i = 0; i < nGroups; i++)
   {
      if (pReader->Read(&m_groups[i]) != S_OK)
         return E_FAIL;
   }

   uint16 nMaterials;
   if (pReader->Read(&nMaterials, sizeof(nMaterials)) != S_OK)
      return E_FAIL;

   for (i = 0; i < nMaterials; i++)
   {
      ms3d_material_t material;
      if (pReader->Read(&material, sizeof(material)) != S_OK)
         return E_FAIL;

      cAutoIPtr<ITexture> pTexture;

      if (material.texture[0] != 0)
      {
         cImage * pTextureImage = ImageLoad(pResourceManager, material.texture);
         if (pTextureImage == NULL)
         {
            DebugMsg1("Could not load texture image %s\n", material.texture);
            return E_FAIL;
         }

         if (pRenderDevice->CreateTexture(pTextureImage, &pTexture) != S_OK)
         {
            DebugMsg1("Could not create device texture for %s\n", material.texture);
            delete pTextureImage;
            return E_FAIL;
         }

         delete pTextureImage;
      }

      IMaterial * pMaterial = MaterialCreate();

      if (pMaterial != NULL)
      {
         pMaterial->SetName(material.name);
         pMaterial->SetAmbient(cColor(material.ambient));
         pMaterial->SetDiffuse(cColor(material.diffuse));
         pMaterial->SetSpecular(cColor(material.specular));
         pMaterial->SetEmissive(cColor(material.emissive));
         pMaterial->SetShininess(material.shininess);
         pMaterial->SetTexture(0, pTexture);

         m_materials.push_back(pMaterial);
      }
   }

   Assert(m_materials.size() == nMaterials);

   float animationFPS;
   float currentTime;
   int nTotalFrames;
   uint16 nJoints;
   if (pReader->Read(&animationFPS, sizeof(animationFPS)) != S_OK
      || pReader->Read(&currentTime, sizeof(currentTime)) != S_OK
      || pReader->Read(&nTotalFrames, sizeof(nTotalFrames)) != S_OK
      || pReader->Read(&nJoints, sizeof(nJoints)) != S_OK)
      return E_FAIL;

   m_joints.resize(nJoints);
   for (i = 0; i < nJoints; i++)
   {
      if (pReader->Read(&m_joints[i]) != S_OK)
         break;
   }

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

            m_pfnRender = RenderVertexProgram;
         }
      }

   }
   else
   {
      SetupJoints();
   }

   return S_OK;
}

void cMs3dMesh::Reset()
{
   tMaterials::iterator iter;
   for (iter = m_materials.begin(); iter != m_materials.end(); iter++)
   {
      (*iter)->Release();
   }

   m_vertices.clear();
   m_triangles.clear();
   m_groups.clear();
   m_materials.clear();
   m_joints.clear();

   m_bCalculatedAABB = false;

   if (m_program != NULL)
   {
      cgDestroyProgram(m_program);
      m_program = NULL;
   }

   ReleaseCgContext();
}

void cMs3dMesh::SetupJoints()
{
   if (m_joints.empty())
   {
      return;
   }

   typedef std::map<std::string, int> tJointNames;

   tJointNames jointNames;

   int index;
   std::vector<cMs3dJoint>::iterator iter;

   for (iter = m_joints.begin(), index = 0; iter != m_joints.end(); iter++, index++)
   {
      const char * pszJointName = iter->GetName();
      if (pszJointName && *pszJointName)
      {
         jointNames.insert(std::make_pair(pszJointName, index));
      }
   }

   std::vector<tMatrix4> absolutes(m_joints.size());
   std::vector<tMatrix4> inverses(m_joints.size());

   for (iter = m_joints.begin(), index = 0; iter != m_joints.end(); iter++, index++)
   {
      int iParent = -1;

      const char * pszParentJointName = iter->GetParentName();
      if (pszParentJointName && *pszParentJointName)
      {
         tJointNames::iterator pjni = jointNames.find(pszParentJointName);
         if (pjni != jointNames.end())
         {
            iParent = pjni->second;
            Assert(iParent >= 0 && iParent < m_joints.size());
         }
      }

      iter->SetParentJointIndex(iParent);

      tMatrix4 absolute;

      if (iParent == -1)
      {
         absolute = iter->GetLocalMatrix();
      }
      else
      {
         absolute = absolutes[iParent] * iter->GetLocalMatrix();
      }

      absolutes[index] = absolute;
      MatrixInvert(absolute, &inverses[index]);

      iter->SetFinalMatrix(absolute);
   }

   // transform all vertices by the inverse of the affecting bone's absolute matrix
   tVertices::iterator vertIter;
   for (vertIter = m_vertices.begin(); vertIter != m_vertices.end(); vertIter++)
   {
      if (vertIter->boneId != -1)
      {
         const tMatrix4 & m = inverses[vertIter->boneId];
         tVec4 v2, v(vertIter->vertex[0], vertIter->vertex[1], vertIter->vertex[2], k4thDimension);
         v2 = m.Transform(v);
         vertIter->vertex[0] = v2.x;
         vertIter->vertex[1] = v2.y;
         vertIter->vertex[2] = v2.z;
      }
   }

   // transform the vertex normals as well
   tTriangles::iterator triIter;
   for (triIter = m_triangles.begin(); triIter != m_triangles.end(); triIter++)
   {
      for (int i = 0; i < 3; i++)
      {
         ms3d_vertex_t & v = m_vertices[triIter->vertexIndices[i]];
         if (v.boneId != -1)
         {
            const tMatrix4 & m = inverses[v.boneId];
            tVec4 n(
               triIter->vertexNormals[i][0],
               triIter->vertexNormals[i][1],
               triIter->vertexNormals[i][2],
               k4thDimension);
            tVec4 nprime;
            nprime = m.Transform(n);
            triIter->vertexNormals[i][0] = nprime.x;
            triIter->vertexNormals[i][1] = nprime.y;
            triIter->vertexNormals[i][2] = nprime.z;
         }
      }
   }
}

void cMs3dMesh::SetFrame(float percent)
{
   Assert(percent >= 0 && percent <= 1);

   std::vector<cMs3dJoint>::iterator iter;
   for (iter = m_joints.begin(); iter != m_joints.end(); iter++)
   {
      tQuat rotation;
      tVec3 translation;

      if (iter->AccessInterpolator()->Interpolate(
         percent * iter->AccessInterpolator()->GetPeriod(),
         NULL, &rotation, &translation) == S_OK)
      {
         tMatrix4 mt, mr, mf, temp;
         MatrixTranslate(translation.x, translation.y, translation.z, &mt);
         rotation.ToMatrix(&mr);
         temp = mt * mr;
         mf = iter->GetLocalMatrix() * temp;

         if (iter->GetParentJointIndex() == -1)
         {
            iter->SetFinalMatrix(mf);
         }
         else
         {
            temp = m_joints[iter->GetParentJointIndex()].GetFinalMatrix() * mf;
            iter->SetFinalMatrix(temp);
         }
      }
   }
}

void cMs3dMesh::RenderSoftware() const
{
   tGroups::const_iterator iter;
   for (iter = m_groups.begin(); iter != m_groups.end(); iter++)
   {
      glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);

      if (iter->GetMaterialIndex() > -1)
      {
         IMaterial * pMaterial = const_cast<IMaterial *>(m_materials[iter->GetMaterialIndex()]);
         GlMaterial(pMaterial);
      }

      glBegin(GL_TRIANGLES);

      const std::vector<uint16> & tris = iter->GetTriangleIndices();
      std::vector<uint16>::const_iterator iterTris;
      for (iterTris = tris.begin(); iterTris != tris.end(); iterTris++)
      {
         const ms3d_triangle_t & tri = m_triangles[*iterTris];

         for (int k = 0; k < 3; k++)
         {
            glTexCoord2f(tri.s[k], 1.0f - tri.t[k]);
            const ms3d_vertex_t & vk = m_vertices[tri.vertexIndices[k]];
            if (vk.boneId == -1)
            {
               glNormal3fv(tri.vertexNormals[k]);
               glVertex3fv(vk.vertex);
            }
            else
            {
               const tMatrix4 & m = m_joints[vk.boneId].GetFinalMatrix();

               tVec4 nprime, n(tri.vertexNormals[k][0], tri.vertexNormals[k][1], tri.vertexNormals[k][2], k4thDimension);
               nprime = m.Transform(n);
               glNormal3fv(nprime.v);

               tVec4 vprime, v(vk.vertex[0], vk.vertex[1], vk.vertex[2], k4thDimension);
               vprime = m.Transform(v);
               glVertex3fv(vprime.v);
            }
         }
      }

      glEnd();

      glPopAttrib();
   }
}

void cMs3dMesh::RenderVertexProgram() const
{
   cgGLBindProgram(m_program);

   cgGLEnableProfile(g_cgProfile);

   cgGLSetStateMatrixParameter(m_modelViewProjParam, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);

   tGroups::const_iterator iter;
   for (iter = m_groups.begin(); iter != m_groups.end(); iter++)
   {
      glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);

      if (iter->GetMaterialIndex() > -1)
      {
         IMaterial * pMaterial = const_cast<IMaterial *>(m_materials[iter->GetMaterialIndex()]);
         GlMaterial(pMaterial);
      }

      glBegin(GL_TRIANGLES);

      const std::vector<uint16> & tris = iter->GetTriangleIndices();
      std::vector<uint16>::const_iterator iterTris;
      for (iterTris = tris.begin(); iterTris != tris.end(); iterTris++)
      {
         const ms3d_triangle_t & tri = m_triangles[*iterTris];

         for (int k = 0; k < 3; k++)
         {
            const ms3d_vertex_t & vk = m_vertices[tri.vertexIndices[k]];
            if (vk.boneId != -1)
            {
               //const tMatrix4 & m = m_joints[vk.boneId].GetFinalMatrix();

               glTexCoord2f(tri.s[k], 1.0f - tri.t[k]);
               glNormal3fv(tri.vertexNormals[k]);
               glVertex3fv(vk.vertex);
            }
         }
      }

      glEnd();

      glPopAttrib();
   }

   cgGLDisableProfile(g_cgProfile);
}

///////////////////////////////////////////////////////////////////////////////
