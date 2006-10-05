/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_EXPORTER_H
#define INCLUDED_EXPORTER_H

#include "IntermediateJoint.h"

#include "modeltypes.h"

#include "combase.h"
#include "filespec.h"
#include "quat.h"
#include "vec3.h"

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

F_DECLARE_INTERFACE(IWriter);

typedef struct msModel msModel;
typedef struct msMesh msMesh;

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cExportMesh
//

class cExportMesh
{
public:
   cExportMesh(
      std::vector<sModelVertex>::const_iterator firstVertex,
      std::vector<sModelVertex>::const_iterator lastVertex,
      int primitive,
      std::vector<uint16>::const_iterator firstIndex,
      std::vector<uint16>::const_iterator lastIndex);

   std::vector<sModelVertex> m_vertices;
   int m_primitive;
   std::vector<uint16> m_indices;
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

   std::vector<cExportMesh>::const_iterator BeginMeshes() const { return m_meshes.begin(); }
   std::vector<cExportMesh>::const_iterator EndMeshes() const { return m_meshes.end(); }

   std::vector<sModelMaterial>::const_iterator BeginMaterials() const { return m_materials.begin(); }
   std::vector<sModelMaterial>::const_iterator EndMaterials() const { return m_materials.end(); }

   std::vector<sModelJoint>::const_iterator BeginModelJoints() const { return m_modelJoints.begin(); }
   std::vector<sModelJoint>::const_iterator EndModelJoints() const { return m_modelJoints.end(); }

   tResult ExportMesh(const tChar * pszFileName);

   tResult ExportMesh(IWriter * pWriter);

   tResult ExportSkeleton(IWriter * pWriter);

   tResult ExportAnimation(IWriter * pWriter);

private:
   void CollectMeshVertices(msMesh * pMesh, std::vector<sModelVertex> * pVertices);
   void CollectMeshNormals(msMesh * pMesh, std::vector<tVec3> * pNormals);
   void CollectModelMaterials(msModel * pModel, std::vector<sModelMaterial> * pMaterials);

private:
   msModel * m_pModel;
   bool m_bAutoDeleteModel;

   std::vector<cExportMesh> m_meshes;
   std::vector<sModelMaterial> m_materials;
   std::vector<cIntermediateJoint> m_tempJoints;
   std::vector<sModelJoint> m_modelJoints;
};

/////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_EXPORTER_H
