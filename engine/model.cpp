///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "model.h"

#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

#include <iterator>

#include "tech/dbgalloc.h" // must be last header

using namespace boost;
using namespace std;


////////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(Model);

#define LocalMsg(msg)            DebugMsgEx2(Model,msg)
#define LocalMsg1(msg,a)         DebugMsgEx3(Model,msg,(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx4(Model,msg,(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx5(Model,msg,(a),(b),(c))


////////////////////////////////////////////////////////////////////////////////

tResult AnimTypeFromString(const tChar * pszAnimTypeStr, eModelAnimationType * pAnimType)
{
   if (pszAnimTypeStr == NULL || pAnimType == NULL)
   {
      return E_POINTER;
   }

   static const struct
   {
      eModelAnimationType type;
      const tChar * pszType;
   }
   animTypes[] =
   {
      { kMAT_Walk, _T("walk") },
      { kMAT_Run, _T("run") },
      { kMAT_Death, _T("death") },
      { kMAT_Attack, _T("attack") },
      { kMAT_Damage, _T("damage") },
      { kMAT_Idle, _T("idle") },
   };

   cStr animTypeStr(pszAnimTypeStr);

   for (int j = 0; j < _countof(animTypes); j++)
   {
      if (animTypeStr.compare(animTypes[j].pszType) == 0)
      {
         *pAnimType = animTypes[j].type;
         return S_OK;
      }
   }

   return S_FALSE;
}

void ParseAnimDescs(const tChar * pszAnimString, vector<sModelAnimationDesc> * pAnimationDescs)
{
   cStr animString(pszAnimString);
   typedef tokenizer<char_separator<tChar> > tokenizer;
   const char_separator<tChar> newlineSep(_T("\n"));
   tokenizer animTokens(animString, newlineSep);
   tokenizer::iterator iter = animTokens.begin(), end = animTokens.end();
   for (; iter != end; ++iter)
   {
      cStr animString(*iter);
      trim(animString);

      vector<cStr> fields;

      const char_separator<tChar> commaSep(_T(","));
      tokenizer fieldTokens(animString, commaSep);
      tokenizer::iterator iter2 = fieldTokens.begin(), end2 = fieldTokens.end();
      for (; iter2 != end2; ++iter2)
      {
         fields.push_back(*iter2);
      }

      if (fields.size() == 3)
      {
         sModelAnimationDesc animDesc;
         if (AnimTypeFromString(fields[2].c_str(), &animDesc.type) == S_OK)
         {
            try
            {
               animDesc.start = lexical_cast<uint>(fields[0]);
               animDesc.end = lexical_cast<uint>(fields[1]);
               animDesc.fps = 0;
               if (animDesc.start > 0 || animDesc.end > 0)
               {
                  pAnimationDescs->push_back(animDesc);
               }
            }
            catch (const bad_lexical_cast &)
            {
            }
         }
      }
   }
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModel
//

///////////////////////////////////////

cModel::cModel()
{
}

///////////////////////////////////////

cModel::cModel(tModelVertices::const_iterator firstVert, tModelVertices::const_iterator lastVert,
               vector<uint16>::const_iterator firstIndex, vector<uint16>::const_iterator lastIndex,
               vector<sModelMesh>::const_iterator firstMesh, vector<sModelMesh>::const_iterator lastMesh,
               tModelMaterials::const_iterator firstMaterial, tModelMaterials::const_iterator lastMaterial,
               IModelSkeleton * pSkeleton)
 : m_vertices(firstVert, lastVert)
 , m_indices(firstIndex, lastIndex)
 , m_meshes(firstMesh, lastMesh)
 , m_materials(firstMaterial, lastMaterial)
 , m_pSkeleton(CTAddRef(pSkeleton))
{
}

///////////////////////////////////////

cModel::~cModel()
{
}

///////////////////////////////////////

tResult cModel::Create(const tModelVertices & verts,
                       const vector<uint16> & indices,
                       const vector<sModelMesh> & meshes,
                       const tModelMaterials & materials,
                       IModelSkeleton * pSkeleton,
                       IModel * * ppModel)
{
   if (ppModel == NULL)
   {
      return E_POINTER;
   }

   cModel * pModel = new cModel(verts.begin(), verts.end(),
                                indices.begin(), indices.end(),
                                meshes.begin(), meshes.end(),
                                materials.begin(), materials.end(),
                                pSkeleton);
   if (pModel == NULL)
   {
      return E_OUTOFMEMORY;
   }

   pModel->PreApplyJoints();

   *ppModel = static_cast<IModel*>(pModel);
   return S_OK;
}

///////////////////////////////////////

tResult cModel::Create(tModelVertices::const_iterator firstVert, tModelVertices::const_iterator lastVert,
                       vector<uint16>::const_iterator firstIndex, vector<uint16>::const_iterator lastIndex,
                       vector<sModelMesh>::const_iterator firstMesh, vector<sModelMesh>::const_iterator lastMesh,
                       tModelMaterials::const_iterator firstMaterial, tModelMaterials::const_iterator lastMaterial,
                       IModelSkeleton * pSkeleton, IModel * * ppModel)
{
   if (ppModel == NULL)
   {
      return E_POINTER;
   }

   cModel * pModel = new cModel(firstVert, lastVert, firstIndex, lastIndex,
      firstMesh, lastMesh, firstMaterial, lastMaterial, pSkeleton);
   if (pModel == NULL)
   {
      return E_OUTOFMEMORY;
   }

   pModel->PreApplyJoints();

   *ppModel = static_cast<IModel*>(pModel);
   return S_OK;
}

///////////////////////////////////////

tResult cModel::GetVertices(uint * pnVertices, const sModelVertex * * ppVertices) const
{
   if (pnVertices != NULL)
   {
      *pnVertices = m_vertices.size();
   }
   if (ppVertices == NULL)
   {
      return E_POINTER;
   }
   if (!m_vertices.empty())
   {
      *ppVertices = &m_vertices[0];
      return S_OK;
   }
   else
   {
      *ppVertices = NULL;
      return S_FALSE;
   }
}

///////////////////////////////////////

tResult cModel::GetIndices(uint * pnIndices, const uint16 * * ppIndices) const
{
   if (pnIndices != NULL)
   {
      *pnIndices = m_indices.size();
   }
   if (ppIndices == NULL)
   {
      return E_POINTER;
   }
   if (!m_indices.empty())
   {
      *ppIndices = &m_indices[0];
      return S_OK;
   }
   else
   {
      *ppIndices = NULL;
      return S_FALSE;
   }
}

///////////////////////////////////////

tResult cModel::GetMaterialCount(uint * pnMaterials) const
{
   if (pnMaterials == NULL)
   {
      return E_POINTER;
   }
   *pnMaterials = m_materials.size();
   return S_OK;
}

///////////////////////////////////////

tResult cModel::GetMaterial(uint index, sModelMaterial * pModelMaterial) const
{
   if (index >= m_materials.size())
   {
      return E_INVALIDARG;
   }
   if (pModelMaterial == NULL)
   {
      return E_POINTER;
   }
   *pModelMaterial = m_materials[index];
   return S_OK;
}

///////////////////////////////////////

const sModelMaterial * cModel::AccessMaterial(uint index) const
{
   if (index < m_materials.size())
   {
      return &m_materials[index];
   }
   return NULL;
}

///////////////////////////////////////

tResult cModel::GetMeshes(uint * pnMeshes, const sModelMesh * * ppMeshes) const
{
   if (pnMeshes != NULL)
   {
      *pnMeshes = m_meshes.size();
   }
   if (ppMeshes == NULL)
   {
      return E_POINTER;
   }
   if (!m_meshes.empty())
   {
      *ppMeshes = &m_meshes[0];
      return S_OK;
   }
   else
   {
      *ppMeshes = NULL;
      return S_FALSE;
   }
}

///////////////////////////////////////

tResult cModel::GetSkeleton(IModelSkeleton * * ppSkeleton)
{
   return m_pSkeleton.GetPointer(ppSkeleton);
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

   vector<tMatrix34> bindMatrices(nJoints);
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

tResult ModelCreateBox(const tVec3 & mins, const tVec3 & maxs, const float color[4], IModel * * ppModel)
{
   sModelVertex verts[] =
   {
      { 0, 0, tVec3(), tVec3(mins.x, mins.y, mins.z), -1 },
      { 0, 0, tVec3(), tVec3(maxs.x, mins.y, mins.z), -1 },
      { 0, 0, tVec3(), tVec3(maxs.x, mins.y, maxs.z), -1 },
      { 0, 0, tVec3(), tVec3(mins.x, mins.y, maxs.z), -1 },
      { 0, 0, tVec3(), tVec3(mins.x, maxs.y, mins.z), -1 },
      { 0, 0, tVec3(), tVec3(maxs.x, maxs.y, mins.z), -1 },
      { 0, 0, tVec3(), tVec3(maxs.x, maxs.y, maxs.z), -1 },
      { 0, 0, tVec3(), tVec3(mins.x, maxs.y, maxs.z), -1 },
   };

   const uint16 indices[] =
   {
      0, 1,
      1, 2,
      2, 3,
      3, 0,

      4, 5,
      5, 6,
      6, 7,
      7, 4,

      0, 4,
      1, 5,
      2, 6,
      3, 7,
   };

   sModelMesh mesh = {0};
   mesh.primitive = kPT_Lines;
   mesh.materialIndex = 0;
   mesh.nIndices = _countof(indices);
   mesh.indexStart = 0;

   sModelMaterial material = {0};
   memcpy(material.diffuse, color, sizeof(material.diffuse));

   cAutoIPtr<IModel> pModel(new cModel(&verts[0], &verts[_countof(verts)],
      &indices[0], &indices[_countof(indices)], &mesh, (&mesh) + 1,
      &material, (&material) + 1, NULL));

   if (!pModel)
   {
      return E_OUTOFMEMORY;
   }

   return pModel.GetPointer(ppModel);
}

///////////////////////////////////////////////////////////////////////////////
