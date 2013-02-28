///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MS3DMODEL_H
#define INCLUDED_MS3DMODEL_H

#include "ms3dmodeldll.h"

#include "ms3dgroup.h"
#include "ms3djoint.h"
#include "ms3dmaterial.h"
#include "ms3dtriangle.h"
#include "ms3dvertex.h"

#include "tech/comtools.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

struct sModelJoint;
struct sModelKeyFrame;
struct sModelMaterial;
struct sModelMesh;
struct sModelVertex;

F_DECLARE_INTERFACE(IModel);
F_DECLARE_INTERFACE(IReader);

///////////////////////////////////////////////////////////////////////////////

struct sMs3dComment
{
   int index;
   std::string comment;
};

///////////////////////////////////////////////////////////////////////////////

#define MS3DMODEL_EXPORT_VECTOR(type) \
   template class MS3DMODEL_API std::allocator<type>; \
   template class MS3DMODEL_API std::vector<type>

MS3DMODEL_EXPORT_VECTOR(cMs3dGroup);
MS3DMODEL_EXPORT_VECTOR(cMs3dJoint);
MS3DMODEL_EXPORT_VECTOR(cMs3dMaterial);
MS3DMODEL_EXPORT_VECTOR(cMs3dTriangle);
MS3DMODEL_EXPORT_VECTOR(cMs3dVertex);
MS3DMODEL_EXPORT_VECTOR(sMs3dComment);

///////////////////////////////////////////////////////////////////////////////

MS3DMODEL_API void CompileMeshes(const std::vector<cMs3dVertex> & ms3dVerts,
                                 const std::vector<cMs3dTriangle> & ms3dTris,
                                 const std::vector<cMs3dGroup> & ms3dGroups,
                                 std::vector<sModelVertex> * pModelVertices,
                                 std::vector<sModelMesh> * pModelMeshes,
                                 std::vector<uint16> * pModelIndices);

MS3DMODEL_API void CompileMaterials(const std::vector<cMs3dMaterial> & ms3dMaterials,
                                    std::vector<sModelMaterial> * pModelMaterials);

MS3DMODEL_API void CompileJointsAndKeyFrames(float animationFPS, const std::vector<cMs3dJoint> & ms3dJoints,
                                             std::vector<sModelJoint> * pModelJoints,
                                             std::vector< std::vector<sModelKeyFrame> > * pModelKeyFrames);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dModel
//

class MS3DMODEL_API cMs3dModel
{
   cMs3dModel(const cMs3dModel &);
   const cMs3dModel & operator =(const cMs3dModel &);

public:
   cMs3dModel();
   ~cMs3dModel();

   tResult Read(IReader * pReader);

   IModel * CreateModel();

   static void * Load(IReader * pReader);
   static void Unload(void * pData);

   inline float GetAnimationFPS() const { return m_animationFPS; }
   inline int GetAnimationTotalFrameCount() const { return m_nTotalFrames; }
   inline const std::vector<cMs3dVertex> & GetVertices() const { return m_ms3dVerts; }
   inline const std::vector<cMs3dTriangle> & GetTriangles() const { return m_ms3dTris; }
   inline const std::vector<cMs3dGroup> & GetGroups() const { return m_ms3dGroups; }
   inline const std::vector<cMs3dMaterial> & GetMaterials() const { return m_ms3dMaterials; }
   inline const std::vector<cMs3dJoint> & GetJoints() const { return m_ms3dJoints; }
   inline const std::string & GetModelComment() const { return m_modelComment; }

private:
   std::vector<cMs3dVertex> m_ms3dVerts;
   std::vector<cMs3dTriangle> m_ms3dTris;
   std::vector<cMs3dGroup> m_ms3dGroups;
   std::vector<cMs3dMaterial> m_ms3dMaterials;
   float m_animationFPS;
   float m_currentTime;
   int m_nTotalFrames;
   std::vector<cMs3dJoint> m_ms3dJoints;
   std::vector<sMs3dComment> m_groupComments;
   std::vector<sMs3dComment> m_materialComments;
   std::vector<sMs3dComment> m_jointComments;
   std::string m_modelComment;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MS3DMODEL_H
