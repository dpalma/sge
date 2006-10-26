///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "model.h"

#include "render/renderapi.h"

#include "tech/resourceapi.h"
#include "tech/readwriteapi.h"
#include "tech/globalobj.h"
#include "tech/filespec.h"
#include "tech/techmath.h"

#include <algorithm>
#include <cfloat>

#include "tech/dbgalloc.h" // must be last header


////////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(Model);

#define LocalMsg(msg)            DebugMsgEx2(Model,msg)
#define LocalMsg1(msg,a)         DebugMsgEx3(Model,msg,(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx4(Model,msg,(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx5(Model,msg,(a),(b),(c))


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModel
//

///////////////////////////////////////

cModel::cModel()
{
}

///////////////////////////////////////

cModel::cModel(const tModelVertices & verts,
               const std::vector<uint16> & indices,
               const std::vector<sModelMesh> & meshes,
               const tModelMaterials & materials,
               IModelSkeleton * pSkeleton)
 : m_vertices(verts)
 , m_indices(indices)
 , m_meshes(meshes)
 , m_materials(materials)
 , m_pSkeleton(CTAddRef(pSkeleton))
{
}

///////////////////////////////////////

cModel::~cModel()
{
}

///////////////////////////////////////

tResult cModel::Create(const tModelVertices & verts,
                       const std::vector<uint16> & indices,
                       const std::vector<sModelMesh> & meshes,
                       const tModelMaterials & materials,
                       IModelSkeleton * pSkeleton,
                       cModel * * ppModel)
{
   if (ppModel == NULL)
   {
      return E_POINTER;
   }

   cModel * pModel = new cModel(verts, indices, meshes, materials, pSkeleton);
   if (pModel == NULL)
   {
      return E_OUTOFMEMORY;
   }

   pModel->PreApplyJoints();

   *ppModel = pModel;
   return S_OK;
}

///////////////////////////////////////
// TODO: How does this work for more than one joint per vertex with blend weights?
// (Answer: I don't think you can pre-apply like this.)

void cModel::PreApplyJoints()
{
   if (!m_pSkeleton)
   {
      return;
   }

   size_t nJoints = 0;
   if (m_pSkeleton->GetJointCount(&nJoints) != S_OK || nJoints == 0)
   {
      return;
   }

   std::vector<tMatrix34> bindMatrices(nJoints);
   m_pSkeleton->GetBindMatrices(bindMatrices.size(), &bindMatrices[0]);

   for (tModelVertices::iterator iter = m_vertices.begin(); iter != m_vertices.end(); iter++)
   {
      int index = FloatToInt(iter->bone);
      if (index < 0)
      {
         continue;
      }

      tVec3 transformedNormal;
      bindMatrices[index].Transform(iter->normal, &transformedNormal);
      iter->normal = transformedNormal;

      tVec3 transformedPosition;
      bindMatrices[index].Transform(iter->pos, &transformedPosition);
      iter->pos = transformedPosition;
   }
}

///////////////////////////////////////////////////////////////////////////////
