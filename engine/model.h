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

   tResult GetVertices(uint * pnVertices, const sModelVertex * * ppVertices) const;

   tResult GetIndices(uint * pnIndices, const uint16 * * ppIndices) const;

   const sModelMaterial & GetMaterial(int index);

   tResult GetMeshes(uint * pnMeshes, const sModelMesh * * ppMeshes) const;

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

inline const sModelMaterial & cModel::GetMaterial(int index)
{
   return m_materials[index];
}


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MODEL_H
