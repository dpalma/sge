///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "minigl.h"

#include "ms3dread.h"
#include "ms3d.h"
#include "mesh.h"
#include "material.h"
#include "readwriteapi.h"
#include "str.h"
#include "vec3.h"
#include "image.h"
#include "matrix4.h"
#include "color.h"
#include "render.h"
#include "resmgr.h"
#include "globalobj.h"

#include <cfloat>
#include <vector>
#include <algorithm>

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
   virtual tResult AddMaterial(IMaterial * pMaterial);
   virtual tResult FindMaterial(const char * pszName, IMaterial * * ppMaterial);

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

static void ForEachRelease(IUnknown * p)
{
   p->Release();
}

cMs3dMesh::~cMs3dMesh()
{
   std::for_each(m_materials.begin(), m_materials.end(), ForEachRelease);
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

      for (int i = 0; i < iter->GetNumTriangles(); i++)
      {
         const ms3d_triangle_t & tri = m_triangles[iter->GetTriangle(i)];

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

tResult cMs3dMesh::AddMaterial(IMaterial * pMaterial)
{
   if (pMaterial == NULL || FindMaterial(pMaterial->GetName(), NULL) == S_OK)
      return E_FAIL;
   m_materials.push_back(pMaterial);
   pMaterial->AddRef();
   return S_OK;
}

tResult cMs3dMesh::FindMaterial(const char * pszName, IMaterial * * ppMaterial)
{
   if (pszName == NULL || pszName[0] == 0)
      return E_FAIL;
   tMaterials::iterator iter;
   for (iter = m_materials.begin(); iter != m_materials.end(); iter++)
   {
      if (strcmp(pszName, (*iter)->GetName()) == 0)
      {
         if (ppMaterial != NULL)
         {
            *ppMaterial = *iter;
            (*ppMaterial)->AddRef();
         }
         return S_OK;
      }
   }
   return S_FALSE;
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

      cAutoIPtr<IMaterial> pMaterial = MaterialCreate();
      if (pMaterial != NULL)
      {
         pMaterial->SetName(material.name);
         pMaterial->SetAmbient(cColor(material.ambient));
         pMaterial->SetDiffuse(cColor(material.diffuse));
         pMaterial->SetSpecular(cColor(material.specular));
         pMaterial->SetEmissive(cColor(material.emissive));
         pMaterial->SetShininess(material.shininess);
         pMaterial->SetTexture(0, pTexture);
         AddMaterial(pMaterial);
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
