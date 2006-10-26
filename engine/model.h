///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MODEL_H
#define INCLUDED_MODEL_H

#include "engine/modelapi.h"
#include "engine/modeltypes.h"
#include "render/renderapi.h"

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////

typedef std::vector<sModelVertex> tModelVertices;
typedef std::vector<sBlendedVertex> tBlendedVertices;
typedef std::vector<sModelMaterial> tModelMaterials;


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
          const std::vector<uint16> & indices,
          const std::vector<sModelMesh> & meshes2,
          const tModelMaterials & materials,
          IModelSkeleton * pSkeleton);

public:
   virtual ~cModel();

   static tResult Create(const tModelVertices & verts,
                         const std::vector<uint16> & indices,
                         const std::vector<sModelMesh> & meshes,
                         const tModelMaterials & materials,
                         IModelSkeleton * pSkeleton,
                         cModel * * ppModel);

   const tModelVertices & GetVertices() const;

   const uint16 * GetIndices() const;

   const sModelMaterial & GetMaterial(int index);

   std::vector<sModelMesh>::const_iterator BeginMeshes() const;
   std::vector<sModelMesh>::const_iterator EndMeshes() const;

   tResult GetSkeleton(IModelSkeleton * * ppSkeleton) { return m_pSkeleton.GetPointer(ppSkeleton); }
   IModelSkeleton * AccessSkeleton() { return m_pSkeleton; }

private:
   // Transform every vertex by the inverse of its affecting
   // joint's absolute transform
   void PreApplyJoints();

   std::vector<sModelVertex> m_vertices;
   std::vector<uint16> m_indices;
   std::vector<sModelMesh> m_meshes;
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
   return m_meshes.begin();
}

inline std::vector<sModelMesh>::const_iterator cModel::EndMeshes() const
{
   return m_meshes.end();
}


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MODEL_H
