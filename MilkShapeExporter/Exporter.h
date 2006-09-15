/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_EXPORTER_H
#define INCLUDED_EXPORTER_H

#include "combase.h"
#include "filespec.h"
#include "vec3.h"

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
