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

class cModel : public cComObject<IMPLEMENTS(IModel)>
{
   cModel(const cModel &);
   const cModel & operator =(const cModel &);

   cModel();

   template <typename VertIter, typename IndexIter, typename MeshIter, typename MaterialIter>
   cModel(VertIter firstVert, VertIter lastVert,
      IndexIter firstIndex, IndexIter lastIndex,
      MeshIter firstMesh, MeshIter lastMesh,
      MaterialIter firstMaterial, MaterialIter lastMaterial,
      IModelSkeleton * pSkeleton)
    : m_vertices(firstVert, lastVert)
    , m_indices(firstIndex, lastIndex)
    , m_meshes(firstMesh, lastMesh)
    , m_materials(firstMaterial, lastMaterial)
    , m_pSkeleton(CTAddRef(pSkeleton))
   {
   }

   cModel(tModelVertices::const_iterator firstVert, tModelVertices::const_iterator lastVert,
      std::vector<uint16>::const_iterator firstIndex, std::vector<uint16>::const_iterator lastIndex,
      std::vector<sModelMesh>::const_iterator firstMesh, std::vector<sModelMesh>::const_iterator lastMesh,
      tModelMaterials::const_iterator firstMaterial, tModelMaterials::const_iterator lastMaterial,
      IModelSkeleton * pSkeleton);

   friend tResult ModelCreateBox(const tVec3 & mins, const tVec3 & maxs, const float color[4], IModel * * ppModel);

public:
   virtual ~cModel();

   static tResult Create(const sModelVertex * pVerts, size_t nVerts,
                         const uint16 * pIndices, size_t nIndices,
                         const sModelMesh * pMeshes, size_t nMeshes,
                         const sModelMaterial * pMaterials, size_t nMaterials,
                         IModelSkeleton * pSkeleton,
                         IModel * * ppModel);

   static tResult Create(tModelVertices::const_iterator firstVert, tModelVertices::const_iterator lastVert,
      std::vector<uint16>::const_iterator firstIndex, std::vector<uint16>::const_iterator lastIndex,
      std::vector<sModelMesh>::const_iterator firstMesh, std::vector<sModelMesh>::const_iterator lastMesh,
      tModelMaterials::const_iterator firstMaterial, tModelMaterials::const_iterator lastMaterial,
      IModelSkeleton * pSkeleton, IModel * * ppModel);

   tResult GetVertices(uint * pnVertices, const sModelVertex * * ppVertices) const;

   tResult GetIndices(uint * pnIndices, const uint16 * * ppIndices) const;

   tResult GetMaterialCount(uint * pnMaterials) const;
   tResult GetMaterial(uint index, sModelMaterial * pModelMaterial) const;
   const sModelMaterial * AccessMaterial(uint index) const;

   tResult GetMeshes(uint * pnMeshes, const sModelMesh * * ppMeshes) const;

   tResult GetSkeleton(IModelSkeleton * * ppSkeleton);

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


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MODEL_H
