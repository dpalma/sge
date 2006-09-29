/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_EXPORTER_H
#define INCLUDED_EXPORTER_H

#include "IntermediateJoint.h"

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

struct sExportHeader
{
   union
   {
      byte bytes[sizeof(uint32)];
      uint32 value;
   } id;
   uint32 version;
};

struct sExportVertex
{
   tVec3::value_type u, v;
   tVec3 normal;
   tVec3 pos;
   float bone; // TODO: call these bones or joints?
};

struct sExportMaterial
{
   float diffuse[4], ambient[4], specular[4], emissive[4], shininess;
   char szTexture[cFileSpec::kMaxPath];
};

struct sExportJoint
{
   int parentIndex;
   tVec3 localTranslation;
   tQuat localRotation;
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

   tResult ExportMesh(const tChar * pszFileName);

   tResult ExportMesh(IWriter * pWriter);

   tResult ExportSkeleton(IWriter * pWriter, std::vector<cIntermediateJoint> * pJoints);

   tResult ExportAnimation(IWriter * pWriter, const std::vector<cIntermediateJoint> & joints);

private:
   void CollectMeshVertices(msMesh * pMesh, std::vector<sExportVertex> * pVertices);
   void CollectMeshNormals(msMesh * pMesh, std::vector<tVec3> * pNormals);
   void CollectModelMaterials(msModel * pModel, std::vector<sExportMaterial> * pMaterials);

private:
   msModel * m_pModel;
   bool m_bAutoDeleteModel;
};

/////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_EXPORTER_H
