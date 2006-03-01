///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MODEL_H
#define INCLUDED_MODEL_H

#include "enginedll.h"
#include "comtools.h"
#include "modelapi.h"
#include "renderapi.h"

#include "techstring.h"

#ifdef _MSC_VER
#pragma once
#endif

#if !(_MSC_VER > 1300)
#pragma warning(push)
#pragma warning(disable:4251)
#endif


F_DECLARE_INTERFACE(IReader);

enum ePrimitiveType;

class cModel;

///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1300
template class ENGINE_API std::allocator<uint16>;
template class ENGINE_API std::vector<uint16>;
#endif


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


#if _MSC_VER > 1300
template class ENGINE_API std::allocator<sModelVertex>;
template class ENGINE_API std::vector<sModelVertex>;
template class ENGINE_API std::allocator<sBlendedVertex>;
template class ENGINE_API std::vector<sBlendedVertex>;
#endif

typedef std::vector<sModelVertex> tModelVertices;
typedef std::vector<sBlendedVertex> tBlendedVertices;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelMaterial
//

class ENGINE_API cModelMaterial
{
public:
   cModelMaterial();
   cModelMaterial(const cModelMaterial & other);

   // Common case: diffuse and texture only
   cModelMaterial(const float diffuse[4], const tChar * pszTexture);

   // All color components
   cModelMaterial(const float diffuse[4], const float ambient[4],
      const float specular[4], const float emissive[4], float shininess,
      const tChar * pszTexture);

   ~cModelMaterial();

   const cModelMaterial & operator =(const cModelMaterial & other);

   // Apply diffuse color (for glEnable(GL_COLOR_MATERIAL)) and bind the texture
   void GlDiffuseAndTexture() const;

private:
   float m_diffuse[4], m_ambient[4], m_specular[4], m_emissive[4], m_shininess;
   cStr m_texture;
};


#if _MSC_VER > 1300
template class ENGINE_API std::allocator<cModelMaterial>;
template class ENGINE_API std::vector<cModelMaterial>;
#endif

typedef std::vector<cModelMaterial> tModelMaterials;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelMesh
//

class ENGINE_API cModelMesh
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


#if _MSC_VER > 1300
template class ENGINE_API std::allocator<cModelMesh>;
template class ENGINE_API std::vector<cModelMesh>;
#endif

typedef std::vector<cModelMesh> tModelMeshes;


///////////////////////////////////////////////////////////////////////////////

ENGINE_API bool GlValidateIndices(const uint16 * pIndices, uint nIndices, uint nVertices);


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelJoint
//

class ENGINE_API cModelJoint
{
public:
   cModelJoint();
   cModelJoint(const cModelJoint & other);

   cModelJoint(int parentIndex, const tMatrix4 & localTransform);

   ~cModelJoint();

   const cModelJoint & operator =(const cModelJoint & other);

   int GetParentIndex() const;
   const tMatrix4 & GetLocalTransform() const;

private:
   int m_parentIndex;
   tMatrix4 m_localTransform;
};

inline int cModelJoint::GetParentIndex() const
{
   return m_parentIndex;
}

inline const tMatrix4 & cModelJoint::GetLocalTransform() const
{
   return m_localTransform;
}


#if _MSC_VER > 1300
template class ENGINE_API std::allocator<cModelJoint>;
template class ENGINE_API std::vector<cModelJoint>;
#endif

typedef std::vector<cModelJoint> tModelJoints;



///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelSkeleton
//

class ENGINE_API cModelSkeleton
{
   friend class cModel;

public:
   cModelSkeleton();
   cModelSkeleton(const cModelSkeleton & other);

   cModelSkeleton(const tModelJoints & joints);

   ~cModelSkeleton();

   const cModelSkeleton & operator =(const cModelSkeleton & other);

   bool IsAnimated() const { return !m_joints.empty(); }

   void InterpolateMatrices(IModelAnimation * pAnim, double time, tMatrices * pMatrices) const;

   void TempAddAnimation(IModelAnimation * pAnim);
   IModelAnimation * TempAccessAnimation() { return m_pAnim; }

private:
   void CalculateInverses(tMatrices * pInverses) const;

   tModelJoints m_joints;

   cAutoIPtr<IModelAnimation> m_pAnim;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModel
//

#define kRT_Model _T("Model") // resource type

class ENGINE_API cModel
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
          const cModelSkeleton & skeleton);

public:
   virtual ~cModel();

   static tResult Create(const tModelVertices & verts,
                         const tModelMaterials & materials,
                         const tModelMeshes & meshes,
                         cModel * * ppModel);

   static tResult Create(const tModelVertices & verts,
                         const tModelMaterials & materials,
                         const tModelMeshes & meshes,
                         const cModelSkeleton & skeleton,
                         cModel * * ppModel);

   void ApplyJointMatrices(const tMatrices & matrices, tBlendedVertices * pVertices) const;

   const tModelVertices & GetVertices() const;

   const cModelMaterial & GetMaterial(int index);

   tModelMeshes::const_iterator BeginMeshses() const;
   tModelMeshes::const_iterator EndMeshses() const;

   cModelSkeleton * GetSkeleton() { return &m_skeleton; }

   static tResult RegisterResourceFormat();

private:
   // Transform every vertex by the inverse of its affecting
   // joint's absolute transform
   void PreApplyJoints();

   static void * ModelLoadMs3d(IReader * pReader);

   static void ModelUnload(void * pData);


   std::vector<sModelVertex> m_vertices;
   std::vector<cModelMaterial> m_materials;
   std::vector<cModelMesh> m_meshes;
   cModelSkeleton m_skeleton;
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


#if !(_MSC_VER > 1300)
#pragma warning(pop)
#endif


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MODEL_H
