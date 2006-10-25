///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MODEL_H
#define INCLUDED_MODEL_H

#include "comtools.h"
#include "modelapi.h"
#include "modeltypes.h"
#include "renderapi.h"

#ifdef _MSC_VER
#pragma once
#endif


F_DECLARE_INTERFACE(IReader);

class cModel;


///////////////////////////////////////////////////////////////////////////////


typedef std::vector<sModelVertex> tModelVertices;
typedef std::vector<sBlendedVertex> tBlendedVertices;
typedef std::vector<sModelMaterial> tModelMaterials;


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
          const std::vector<uint16> & indices,
          const std::vector<sModelMesh> & meshes2,
          const tModelMaterials & materials,
          IModelSkeleton * pSkeleton);

   cModel(const tModelVertices & verts,
          const tModelMaterials & materials,
          const tModelMeshes & meshes,
          IModelSkeleton * pSkeleton);

   //friend void * ModelMs3dLoad(IReader * pReader);
   //friend void ModelMs3dUnload(void * pData);

public:
   virtual ~cModel();

   static tResult Create(const tModelVertices & verts,
                         const tModelMaterials & materials,
                         const tModelMeshes & meshes,
                         cModel * * ppModel);

   static tResult Create(const tModelVertices & verts,
                         const std::vector<uint16> & indices,
                         const std::vector<sModelMesh> & meshes2,
                         const tModelMaterials & materials,
                         IModelSkeleton * pSkeleton,
                         cModel * * ppModel);

   static tResult Create(const tModelVertices & verts,
                         const tModelMaterials & materials,
                         const tModelMeshes & meshes,
                         IModelSkeleton * pSkeleton,
                         cModel * * ppModel);

   const tModelVertices & GetVertices() const;

   const uint16 * GetIndices() const;

   const sModelMaterial & GetMaterial(int index);

   std::vector<sModelMesh>::const_iterator BeginMeshes() const;
   std::vector<sModelMesh>::const_iterator EndMeshes() const;

   tModelMeshes::const_iterator BeginMeshses() const;
   tModelMeshes::const_iterator EndMeshses() const;

   tResult GetSkeleton(IModelSkeleton * * ppSkeleton) { return m_pSkeleton.GetPointer(ppSkeleton); }
   IModelSkeleton * AccessSkeleton() { return m_pSkeleton; }

private:
   // Transform every vertex by the inverse of its affecting
   // joint's absolute transform
   void PreApplyJoints();

   std::vector<sModelVertex> m_vertices;
   std::vector<uint16> m_indices;
   std::vector<sModelMesh> m_meshes2;
   std::vector<cModelMesh> m_meshes;
   std::vector<sModelMaterial> m_materials;
   cAutoIPtr<IModelSkeleton> m_pSkeleton;
};

inline const tModelVertices & cModel::GetVertices() const
{
   return m_vertices;
}

inline const uint16 * cModel::GetIndices() const
{
   return m_indices.empty() ? NULL : &m_indices[0];
}

inline const sModelMaterial & cModel::GetMaterial(int index)
{
   return m_materials[index];
}

inline std::vector<sModelMesh>::const_iterator cModel::BeginMeshes() const
{
   return m_meshes2.begin();
}

inline std::vector<sModelMesh>::const_iterator cModel::EndMeshes() const
{
   return m_meshes2.end();
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
