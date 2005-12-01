///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MODEL_H
#define INCLUDED_MODEL_H

#include "enginedll.h"
#include "comtools.h"
#include "modelapi.h"
#include "renderapi.h"

#include "quat.h"
#include "vec3.h"
#include "matrix4.h"
#include "techstring.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

#if _MSC_VER <= 1300
#pragma warning(push)
#pragma warning(disable:4251)
#endif


F_DECLARE_INTERFACE(IReader);

enum ePrimitiveType;

///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1300
template class ENGINE_API cMatrix4<float>;
template class ENGINE_API std::allocator< cMatrix4<float> >;
template class ENGINE_API std::vector< cMatrix4<float> >;
template class ENGINE_API std::allocator<uint16>;
template class ENGINE_API std::vector<uint16>;
#endif

typedef std::vector< cMatrix4<float> > tMatrices;


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
// STRUCT: sModelKeyFrame
//

struct sModelKeyFrame
{
   double time;
   tVec3 translation;
   tQuat rotation;
};

#if _MSC_VER > 1300
template class ENGINE_API std::allocator<sModelKeyFrame>;
template class ENGINE_API std::vector<sModelKeyFrame>;
#endif

typedef std::vector<sModelKeyFrame> tModelKeyFrames;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelJoint
//

class ENGINE_API cModelJoint
{
public:
   cModelJoint();
   cModelJoint(const cModelJoint & other);

   cModelJoint(int parentIndex, const tMatrix4 & localTransform, const tModelKeyFrames & keyFrames);

   ~cModelJoint();

   const cModelJoint & operator =(const cModelJoint & other);

   int GetParentIndex() const;
   const tMatrix4 & GetLocalTransform() const;

   uint GetKeyFrameCount() const;
   tResult GetKeyFrame(uint index, sModelKeyFrame * pFrame) const;

   tResult Interpolate(double time, tVec3 * pTrans, tQuat * pRot) const;

private:
   int m_parentIndex;
   tMatrix4 m_localTransform;

   std::vector<sModelKeyFrame> m_keyFrames;
};

inline int cModelJoint::GetParentIndex() const
{
   return m_parentIndex;
}

inline const tMatrix4 & cModelJoint::GetLocalTransform() const
{
   return m_localTransform;
}

inline uint cModelJoint::GetKeyFrameCount() const
{
   return m_keyFrames.size();
}


#if _MSC_VER > 1300
template class ENGINE_API std::allocator<cModelJoint>;
template class ENGINE_API std::vector<cModelJoint>;
#endif

typedef std::vector<cModelJoint> tModelJoints;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelAnimation
//

class ENGINE_API cModelAnimation
{
public:
   cModelAnimation();
   cModelAnimation(const cModelAnimation & other);

   cModelAnimation(const tModelKeyFrames & keyFrames);

   ~cModelAnimation();

   const cModelAnimation & operator =(const cModelAnimation & other);

   size_t GetKeyFrameCount() const;
   tResult GetKeyFrame(uint index, sModelKeyFrame * pFrame) const;

   tResult Interpolate(double time, tVec3 * pTrans, tQuat * pRot) const;

private:
   std::vector<sModelKeyFrame> m_keyFrames;
};

inline size_t cModelAnimation::GetKeyFrameCount() const
{
   return m_keyFrames.size();
}


#if _MSC_VER > 1300
template class ENGINE_API std::allocator<cModelAnimation>;
template class ENGINE_API std::vector<cModelAnimation>;
#endif

typedef std::vector<cModelAnimation> tModelAnimations;



///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelSkeleton
//

class ENGINE_API cModelSkeleton
{
public:
   cModelSkeleton();
   cModelSkeleton(const cModelSkeleton & other);

   cModelSkeleton(const tModelJoints & joints);

   ~cModelSkeleton();

   const cModelSkeleton & operator =(const cModelSkeleton & other);

   bool IsAnimated() const { return !m_joints.empty(); }

   void PreApplyInverses(tModelVertices::iterator first,
                         tModelVertices::iterator last,
                         tModelVertices::iterator dest) const;

   double GetAnimationLength() const;

   void InterpolateMatrices(double time, tMatrices * pMatrices) const;

private:
   void CalculateInverses();

   tModelJoints m_joints;
   tMatrices m_inverses;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModel
//

#define kRT_Model "Model" // resource type

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

   bool IsAnimated() const;
   double GetTotalAnimationLength() const;

   void InterpolateJointMatrices(double time, tMatrices * pMatrices) const;
   void ApplyJointMatrices(const tMatrices & matrices, tBlendedVertices * pVertices) const;

   const tModelVertices & GetVertices() const;

   const cModelMaterial & GetMaterial(int index);

   tModelMeshes::const_iterator BeginMeshses() const;
   tModelMeshes::const_iterator EndMeshses() const;

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

inline bool cModel::IsAnimated() const
{
   return m_skeleton.IsAnimated();
}

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


#if _MSC_VER <= 1300
#pragma warning(pop)
#endif


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MODEL_H
