///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "minigl.h"

#include "ms3d.h"
#include "mesh.h"
#include "material.h"
#include "readwriteapi.h"
#include "str.h"
#include "vec3.h"
#include "comtools.h"
#include "image.h"
#include "matrix4.h"
#include "color.h"
#include "render.h"
#include "resmgr.h"
#include "globalobj.h"

#include <cfloat>
#include <vector>

#include "dbgalloc.h" // must be last header

// REFERENCES
// http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=31
// http://rsn.gamedev.net/tutorials/ms3danim.asp

static const char g_MS3D[] = "MS3D000000";

// If this assertion fails, the struct packing was likely wrong
// when ms3d.h was compiled.
AssertOnce(sizeof(ms3d_header_t) == 14);


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dGroup
//

class cMs3dGroup
{
   friend class cReadWriteOps<cMs3dGroup>;

public:
   cMs3dGroup();

   int GetMaterialIndex() const
   {
      return materialIndex;
   }

   const std::vector<uint16> & GetTriangleIndices() const
   {
      return triangleIndices;
   }

private:
   byte flags; // SELECTED | HIDDEN
   char name[32];
   std::vector<uint16> triangleIndices;
   char materialIndex;
};

///////////////////////////////////////

cMs3dGroup::cMs3dGroup()
 : flags(0), materialIndex(-1)
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
      if (pReader->Read(&pGroup->flags, sizeof(pGroup->flags)) != S_OK)
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

class cMs3dJoint
{
   friend class cReadWriteOps<cMs3dJoint>;

public:
   cMs3dJoint();

#ifdef _DEBUG
   void DebugPrint();
#endif

private:
   byte flags; // SELECTED | DIRTY
   char name[32];
   char parentName[32];
   float rotation[3]; // local reference matrix
   float position[3];

   std::vector<ms3d_keyframe_rot_t> keyFramesRot; // local animation matrices
   std::vector<ms3d_keyframe_pos_t> keyFramesTrans; // local animation matrices
};

///////////////////////////////////////

cMs3dJoint::cMs3dJoint()
 : flags(0)
{
   name[0] = 0;
   parentName[0] = 0;
}

///////////////////////////////////////

#ifdef _DEBUG
void cMs3dJoint::DebugPrint()
{
   DebugPrintf(NULL, 0,
      "Joint: %s\n"
      "   parent: %s\n"
      "   rotation: <%.3f, %.3f, %.3f>\n"
      "   position: <%.3f, %.3f, %.3f>\n"
      "   %d rotation keyframes\n"
      "   %d position keyframes\n",
      name, parentName, rotation[0], rotation[1], rotation[2],
      position[0], position[1], position[2],
      keyFramesRot.size(), keyFramesTrans.size());
}
#endif

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
      if (pReader->Read(&pJoint->flags, sizeof(pJoint->flags)) != S_OK)
         break;

      if (pReader->Read(pJoint->name, sizeof(pJoint->name)) != S_OK)
         break;

      if (pReader->Read(pJoint->parentName, sizeof(pJoint->parentName)) != S_OK)
         break;

      if (pReader->Read(pJoint->rotation, sizeof(pJoint->rotation)) != S_OK)
         break;

      if (pReader->Read(pJoint->position, sizeof(pJoint->position)) != S_OK)
         break;

      uint16 nKeyFramesRot, nKeyFramesTrans;

      if (pReader->Read(&nKeyFramesRot, sizeof(nKeyFramesRot)) != S_OK)
         break;
      if (pReader->Read(&nKeyFramesTrans, sizeof(nKeyFramesTrans)) != S_OK)
         break;

      pJoint->keyFramesRot.resize(nKeyFramesRot);
      pJoint->keyFramesTrans.resize(nKeyFramesTrans);

      if (pReader->Read(&pJoint->keyFramesRot[0], pJoint->keyFramesRot.size() * sizeof(ms3d_keyframe_rot_t)) != S_OK)
         break;
      if (pReader->Read(&pJoint->keyFramesTrans[0], pJoint->keyFramesTrans.size() * sizeof(ms3d_keyframe_pos_t)) != S_OK)
         break;

      result = S_OK;
   }
   while (0);

   return result;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dMesh
//

class cMs3dMesh : public cComObject<IMPLEMENTS(IMesh)>
{
   cMs3dMesh(const cMs3dMesh &); // private, un-implemented
   const cMs3dMesh & operator=(const cMs3dMesh &); // private, un-implemented

public:
   ////////////////////////////////////

   cMs3dMesh();
   virtual ~cMs3dMesh();

   virtual void GetAABB(tVec3 * pMaxs, tVec3 * pMins) const;
   virtual void Render(IRenderDevice * pRenderDevice) const;

   tResult Read(IRenderDevice * pRenderDevice, IReader * pReader);

private:
   typedef std::vector<ms3d_vertex_t> tVertices;
   typedef std::vector<ms3d_triangle_t> tTriangles;
   typedef std::vector<cMs3dGroup> tGroups;
   typedef std::vector<IMaterial *> tMaterials;
   typedef std::vector<cMs3dJoint> tJoints;

   tVertices m_vertices;
   tTriangles m_triangles;
   tGroups m_groups;
   tMaterials m_materials;
   tJoints m_joints;

   mutable tVec3 m_maxs, m_mins;
   mutable bool m_bCalculatedAABB;
};

cMs3dMesh::cMs3dMesh()
 : m_bCalculatedAABB(false)
{
}

cMs3dMesh::~cMs3dMesh()
{
   tMaterials::iterator iter;
   for (iter = m_materials.begin(); iter != m_materials.end(); iter++)
   {
      (*iter)->Release();
   }
   m_materials.clear();
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

void cMs3dMesh::Render(IRenderDevice * pRenderDevice) const
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

         glNormal3fv(tri.vertexNormals[0]);
         glTexCoord2f(tri.s[0], 1.0f - tri.t[0]);
         glVertex3fv(m_vertices[tri.vertexIndices[0]].vertex);

         glNormal3fv(tri.vertexNormals[1]);
         glTexCoord2f(tri.s[1], 1.0f - tri.t[1]);
         glVertex3fv(m_vertices[tri.vertexIndices[1]].vertex);

         glNormal3fv(tri.vertexNormals[2]);
         glTexCoord2f(tri.s[2], 1.0f - tri.t[2]);
         glVertex3fv(m_vertices[tri.vertexIndices[2]].vertex);
      }

      glEnd();

      glPopAttrib();
   }
}

tResult cMs3dMesh::Read(IRenderDevice * pRenderDevice, IReader * pReader)
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
         UseGlobal(ResourceManager);

         cImage texture;
         if (ImageLoad(pResourceManager, material.texture, &texture))
         {
            pRenderDevice->CreateTexture(&texture, &pTexture);
         }
      }

      IMaterial * pMaterial = MaterialCreate();

      if (pMaterial != NULL)
      {
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
#ifdef _DEBUG
      m_joints[i].DebugPrint();
#endif
   }

   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////

IMesh * LoadMs3d(IRenderDevice * pRenderDevice, IReader * pReader)
{
   Assert(pReader != NULL);

   cMs3dMesh * pMesh = new cMs3dMesh;

   if (pMesh != NULL)
   {
      if (pMesh->Read(pRenderDevice, pReader) != S_OK)
      {
         delete pMesh;
         return NULL;
      }

      return pMesh;
   }

   return NULL;
}

///////////////////////////////////////////////////////////////////////////////
