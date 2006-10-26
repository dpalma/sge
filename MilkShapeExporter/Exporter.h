/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_EXPORTER_H
#define INCLUDED_EXPORTER_H

#include "IntermediateJoint.h"

#include "engine/modeltypes.h"

#include "tech/combase.h"
#include "tech/filespec.h"
#include "tech/quat.h"
#include "tech/vec3.h"

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

F_DECLARE_INTERFACE(IWriter);

enum eModelAnimationType;

typedef struct msModel msModel;
typedef struct msMesh msMesh;

typedef std::vector<sModelKeyFrame> tModelKeyFrameVector;


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cExportAnimation
//

class cExportAnimation
{
public:
   cExportAnimation(eModelAnimationType type,
      std::vector<tModelKeyFrameVector>::const_iterator firstKFV,
      std::vector<tModelKeyFrameVector>::const_iterator lastKFV);

   eModelAnimationType GetAnimationType() const { return m_type; }

//private:
   eModelAnimationType m_type;
   std::vector<tModelKeyFrameVector> m_keyFrameVectors;
};

template <>
class cReadWriteOps<cExportAnimation>
{
public:
   static tResult Write(IWriter * pWriter, const cExportAnimation & exportAnim);
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cExporter
//

class cExporter
{
public:
   cExporter(msModel * pModel, bool bAutoDeleteModel = true);
   ~cExporter();

   void PreProcess();

   std::vector<sModelVertex>::const_iterator BeginVertices() const { return m_vertices.begin(); }
   std::vector<sModelVertex>::const_iterator EndVertices() const { return m_vertices.end(); }

   std::vector<uint16>::const_iterator BeginIndices() const { return m_indices.begin(); }
   std::vector<uint16>::const_iterator EndIndices() const { return m_indices.end(); }

   std::vector<sModelMesh>::const_iterator BeginModelMeshes() const { return m_modelMeshes.begin(); }
   std::vector<sModelMesh>::const_iterator EndModelMeshes() const { return m_modelMeshes.end(); }

   std::vector<sModelMaterial>::const_iterator BeginMaterials() const { return m_materials.begin(); }
   std::vector<sModelMaterial>::const_iterator EndMaterials() const { return m_materials.end(); }

   std::vector<sModelJoint>::const_iterator BeginModelJoints() const { return m_modelJoints.begin(); }
   std::vector<sModelJoint>::const_iterator EndModelJoints() const { return m_modelJoints.end(); }

   std::vector<cExportAnimation>::const_iterator BeginAnimSeqs() const { return m_animSeqs.begin(); }
   std::vector<cExportAnimation>::const_iterator EndAnimSeqs() const { return m_animSeqs.end(); }

   tResult ExportMesh(const tChar * pszFileName);
   tResult ExportMesh(IWriter * pWriter);

private:
   static void CollectMeshes(msModel * pModel, std::vector<sModelVertex> * pVertices,
         std::vector<uint16> * pIndices, std::vector<sModelMesh> * pModelMeshes);
   static void CollectMeshVertices(msMesh * pMesh, std::vector<sModelVertex> * pVertices);
   static void CollectMeshNormals(msMesh * pMesh, std::vector<tVec3> * pNormals);
   static void CollectModelMaterials(msModel * pModel, std::vector<sModelMaterial> * pMaterials);
   static void CollectJoints(msModel * pModel, std::vector<cIntermediateJoint> * pTempJoints, std::vector<sModelJoint> * pModelJoints);

private:
   msModel * m_pModel;
   bool m_bAutoDeleteModel;

   std::vector<sModelVertex> m_vertices;
   std::vector<uint16> m_indices;
   std::vector<sModelMesh> m_modelMeshes;
   std::vector<sModelMaterial> m_materials;
   std::vector<cIntermediateJoint> m_tempJoints;
   std::vector<sModelJoint> m_modelJoints;
   std::vector<cExportAnimation> m_animSeqs;
};

/////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_EXPORTER_H
