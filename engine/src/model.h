///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MODEL_H
#define INCLUDED_MODEL_H

#include "comtools.h"
#include "modelapi.h"
#include "renderapi.h"

#ifdef _MSC_VER
#pragma once
#endif


F_DECLARE_INTERFACE(IReader);

class cModel;


///////////////////////////////////////////////////////////////////////////////

struct sModelVertex
{
   tVec3::value_type u, v;
   tVec3 normal;
   tVec3 pos;
   float bone; // TODO: call these bones or joints?
};

// for software vertex blending: bone indices not needed after blending
struct sBlendedVertex
{
   tVec3::value_type u, v;
   tVec3 normal;
   tVec3 pos;
};


typedef std::vector<sModelVertex> tModelVertices;
typedef std::vector<sBlendedVertex> tBlendedVertices;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelMaterial
//

class cModelMaterial
{
public:
   cModelMaterial();
   cModelMaterial(const cModelMaterial & other);

   // Common case: diffuse and texture only
   cModelMaterial(const float diffuse[4], const char * pszTexture);

   // All color components
   cModelMaterial(const float diffuse[4], const float ambient[4],
      const float specular[4], const float emissive[4], float shininess,
      const char * pszTexture);

   ~cModelMaterial();

   const cModelMaterial & operator =(const cModelMaterial & other);

   // Apply diffuse color (for glEnable(GL_COLOR_MATERIAL)) and bind the texture
   void GlDiffuseAndTexture() const;

private:
   float m_diffuse[4], m_ambient[4], m_specular[4], m_emissive[4], m_shininess;
   std::string m_texture;
};


typedef std::vector<cModelMaterial> tModelMaterials;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelMesh
//

class cModelMesh
{
public:
   cModelMesh();
   cModelMesh(const cModelMesh & other);

   cModelMesh(ePrimitiveType primitive, const std::vector<uint16> & indices, int8 materialIndex);

   ~cModelMesh();

   const cModelMesh & operator =(const cModelMesh & other);

   ePrimitiveType GetPrimitiveType() const;
   const uint16 * GetIndexData() const;
   uint GetIndexCount() const;
   int8 GetMaterialIndex() const;

private:
   ePrimitiveType m_primitive;
   std::vector<uint16> m_indices;
   int8 m_materialIndex;
};

inline ePrimitiveType cModelMesh::GetPrimitiveType() const
{
   return m_primitive;
}

inline const uint16 * cModelMesh::GetIndexData() const
{
   return &m_indices[0];
}

inline uint cModelMesh::GetIndexCount() const
{
   return m_indices.size();
}

inline int8 cModelMesh::GetMaterialIndex() const
{
   return m_materialIndex;
}

typedef std::vector<cModelMesh> tModelMeshes;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModel
//

class cModel
{
   cModel(const cModel &);
   const cModel & operator =(const cModel &);

   cModel();

   cModel(const tModelVertices & verts,
          const tModelMaterials & materials,
          const tModelMeshes & meshes);

   cModel(const tModelVertices & verts,
          const tModelMaterials & materials,
          const tModelMeshes & meshes,
          IModelSkeleton * pSkeleton);

   friend void * ModelMs3dLoad(IReader * pReader);
   friend void ModelMs3dUnload(void * pData);

public:
   virtual ~cModel();

   static tResult Create(const tModelVertices & verts,
                         const tModelMaterials & materials,
                         const tModelMeshes & meshes,
                         cModel * * ppModel);

   static tResult Create(const tModelVertices & verts,
                         const tModelMaterials & materials,
                         const tModelMeshes & meshes,
                         IModelSkeleton * pSkeleton,
                         cModel * * ppModel);

   void ApplyJointMatrices(const tMatrices & matrices, tBlendedVertices * pVertices) const;

   const tModelVertices & GetVertices() const;

   const cModelMaterial & GetMaterial(int index);

   tModelMeshes::const_iterator BeginMeshses() const;
   tModelMeshes::const_iterator EndMeshses() const;

   tResult GetSkeleton(IModelSkeleton * * ppSkeleton) { return m_pSkeleton.GetPointer(ppSkeleton); }
   IModelSkeleton * AccessSkeleton() { return m_pSkeleton; }

private:
   // Transform every vertex by the inverse of its affecting
   // joint's absolute transform
   void PreApplyJoints();


   std::vector<sModelVertex> m_vertices;
   std::vector<cModelMaterial> m_materials;
   std::vector<cModelMesh> m_meshes;
   cAutoIPtr<IModelSkeleton> m_pSkeleton;
};

inline const tModelVertices & cModel::GetVertices() const
{
   return m_vertices;
}

inline const cModelMaterial & cModel::GetMaterial(int index)
{
   return m_materials[index];
}

inline tModelMeshes::const_iterator cModel::BeginMeshses() const
{
   return m_meshes.begin();
}

inline tModelMeshes::const_iterator cModel::EndMeshses() const
{
   return m_meshes.end();
}


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MODEL_H
