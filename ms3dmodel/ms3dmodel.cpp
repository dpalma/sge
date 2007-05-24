////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ms3dmodel/ms3dmodel.h"

#include "ms3dheader.h"
#include "vertexmapper.h"

#include "ms3dmodel/ms3dmodelapi.h"

#include "engine/modelapi.h"
#include "engine/modeltypes.h"

#include "render/renderapi.h"

#include "tech/vec4.h"
#include "tech/matrix4.h"
#include "tech/resourceapi.h"
#include "tech/readwriteapi.h"
#include "tech/globalobj.h"
#include "tech/filespec.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#define BOOST_MEM_FN_ENABLE_STDCALL
#include <boost/mem_fn.hpp>

#include <algorithm>
#include <map>
#include <vector>

#include "tech/dbgalloc.h" // must be last header

using namespace boost;
using namespace std;

////////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(Ms3dModel);

#define LocalMsg(msg)                  DebugMsgEx(Ms3dModel,msg)
#define LocalMsg1(msg,a)               DebugMsgEx1(Ms3dModel,msg,(a))
#define LocalMsg2(msg,a,b)             DebugMsgEx2(Ms3dModel,msg,(a),(b))
#define LocalMsg3(msg,a,b,c)           DebugMsgEx3(Ms3dModel,msg,(a),(b),(c))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(Ms3dModel,(cond),msg)
#define LocalMsgIf1(cond,msg,a)        DebugMsgIfEx1(Ms3dModel,(cond),msg,(a))
#define LocalMsgIf2(cond,msg,a,b)      DebugMsgIfEx2(Ms3dModel,(cond),msg,(a),(b))
#define LocalMsgIf3(cond,msg,a,b,c)    DebugMsgIfEx3(Ms3dModel,(cond),msg,(a),(b),(c))

////////////////////////////////////////////////////////////////////////////////

static bool ModelVertsEqual(const sModelVertex & vert1, const sModelVertex & vert2)
{
   if ((vert1.u == vert2.u)
      && (vert1.v == vert2.v)
      && (vert1.normal.x == vert2.normal.x)
      && (vert1.normal.y == vert2.normal.y)
      && (vert1.normal.z == vert2.normal.z)
      && (vert1.pos.x == vert2.pos.x)
      && (vert1.pos.y == vert2.pos.y)
      && (vert1.pos.z == vert2.pos.z)
      && (vert1.bone == vert2.bone))
   {
      return true;
   }
   return false;
}

static void MatrixFromAngles(tVec3 angles, tMatrix4 * pMatrix)
{
   tMatrix4 rotX, rotY, rotZ, temp1, temp2;
   MatrixRotateX(Rad2Deg(angles.x), &rotX);
   MatrixRotateY(Rad2Deg(angles.y), &rotY);
   MatrixRotateZ(Rad2Deg(angles.z), &rotZ);
   temp1 = rotZ;
   temp1.Multiply(rotY, &temp2);
   temp2.Multiply(rotX, pMatrix);
}

template <typename T, typename SIZE_T>
static tResult ReadVector(IReader * pReader, std::vector<T> * pVector)
{
   if (pReader == NULL || pVector == NULL)
   {
      return E_POINTER;
   }

   SIZE_T count;
   if (pReader->Read(&count, sizeof(count)) != S_OK)
   {
      return E_FAIL;
   }

   if (count == 0)
   {
      return S_FALSE;
   }

   pVector->resize(count);

   for (SIZE_T i = 0; i < count; ++i)
   {
      if (pReader->Read(&((*pVector)[i])) != S_OK)
      {
         return NULL;
      }
   }

   return S_OK;
}

void ReadComments(IReader * pReader, vector<string> * pComments)
{
   uint nComments = 0;
   if (pReader->Read(&nComments, sizeof(nComments)) != S_OK
      || nComments == 0)
   {
      return;
   }

   pComments->reserve(nComments);

   for (uint i = 0; i < nComments; i++)
   {
      int index, length;
      if (pReader->Read(&index, sizeof(index)) == S_OK
         && pReader->Read(&length, sizeof(length)) == S_OK)
      {
         Assert(index == i);

         char * pszTemp = static_cast<char *>(alloca((length + 1) * sizeof(char)));
         if (pReader->Read(pszTemp, length * sizeof(char)) == S_OK)
         {
            pszTemp[length] = 0;
            pComments->push_back(string(pszTemp));
         }
      }
   }
}

static void CompileMeshes(const vector<cMs3dVertex> & ms3dVerts,
                          const vector<cMs3dTriangle> & ms3dTris,
                          const vector<cMs3dGroup> & ms3dGroups,
                          vector<sModelVertex> * pModelVertices,
                          vector<sModelMesh> * pModelMeshes,
                          vector<uint16> * pModelIndices)
{
   //////////////////////////////
   // Re-map the vertices based on the triangles because Milkshape file
   // triangles contain some vertex info.

   // TODO: clean up this vertex mapping code !!!!!!!

   pModelVertices->resize(ms3dVerts.size());

   cVertexMapper vertexMapper(ms3dVerts);

   typedef multimap<uint, uint> tVertexMap;
   tVertexMap vertexMap;

   vector<cMs3dTriangle>::const_iterator iter = ms3dTris.begin(), end = ms3dTris.end();
   for (; iter != end; ++iter)
   {
      for (int k = 0; k < 3; ++k)
      {
         vertexMapper.MapVertex(
            iter->GetVertexIndex(k), 
            iter->GetVertexNormal(k), 
            iter->GetS(k), 
            iter->GetT(k));

         uint index = iter->GetVertexIndex(k);
         if (vertexMap.find(index) == vertexMap.end())
         {
            (*pModelVertices)[index].u = iter->GetS(k);
            (*pModelVertices)[index].v = 1 - iter->GetT(k);
            (*pModelVertices)[index].normal = iter->GetVertexNormal(k);
            (*pModelVertices)[index].pos = ms3dVerts[index].GetPosition();
            (*pModelVertices)[index].bone = ms3dVerts[index].GetBone();
            vertexMap.insert(make_pair(index,index));
         }
         else
         {
            sModelVertex vert = (*pModelVertices)[index];
            vert.u = iter->GetS(k);
            vert.v = 1 - iter->GetT(k);
            vert.normal = iter->GetVertexNormal(k);
            uint iVertMatch = ~0u;
            tVertexMap::iterator viter = vertexMap.lower_bound(index);
            tVertexMap::iterator vend = vertexMap.upper_bound(index);
            for (; viter != vend; viter++)
            {
               uint index2 = viter->second;
               if (index2 != index)
               {
                  const sModelVertex & vert1 = vert;
                  const sModelVertex & vert2 = (*pModelVertices)[index2];
                  if ((vert1.normal.x == vert2.normal.x)
                     && (vert1.normal.y == vert2.normal.y)
                     && (vert1.normal.z == vert2.normal.z)
                     && (vert1.u == vert2.u)
                     && (vert1.v == vert2.v))
                  {
                     iVertMatch = index2;
                     break;
                  }
               }
            }
            if (iVertMatch == ~0u)
            {
               // Not already mapped, but there may be a suitable vertex
               // elsewhere in the array. Have to look through whole map.
               tVertexMap::iterator viter = vertexMap.begin();
               tVertexMap::iterator vend = vertexMap.end();
               for (; viter != vend; viter++)
               {
                  const sModelVertex & vert1 = vert;
                  const sModelVertex & vert2 = (*pModelVertices)[viter->second];
                  if (ModelVertsEqual(vert1, vert2))
                  {
                     iVertMatch = viter->second;
                     break;
                  }
               }
               if (iVertMatch == ~0u)
               {
                  // Not mapped and no usable vertex already in the array
                  // so create a new one.
                  pModelVertices->push_back(vert);
                  vertexMap.insert(make_pair(index,pModelVertices->size()-1));
               }
            }
         }
      }
   }

//   DebugMsg2("Mapped vertex array has %d members (originally %d)\n", vertices.size(), ms3dVerts.size());

   //////////////////////////////
   // Prepare the groups for the model

   pModelMeshes->resize(ms3dGroups.size());

   for (uint i = 0; i < ms3dGroups.size(); i++)
   {
      const cMs3dGroup & group = ms3dGroups[i];

      vector<uint16> mappedIndices;
      for (uint j = 0; j < group.GetNumTriangles(); j++)
      {
         const cMs3dTriangle & tri = ms3dTris[group.GetTriangle(j)];
         for (int k = 0; k < 3; k++)
         {
            mappedIndices.push_back(vertexMapper.MapVertex(
               tri.GetVertexIndex(k), tri.GetVertexNormal(k), 
               tri.GetS(k), tri.GetT(k)));
         }
      }

      (*pModelMeshes)[i].primitive = kPT_Triangles;
      (*pModelMeshes)[i].materialIndex = group.GetMaterialIndex();
      (*pModelMeshes)[i].indexStart = pModelIndices->size();
      (*pModelMeshes)[i].nIndices = mappedIndices.size();

      pModelIndices->insert(pModelIndices->end(), mappedIndices.begin(), mappedIndices.end());
   }
}

static void CompileMaterials(const vector<cMs3dMaterial> & ms3dMaterials,
                             vector<sModelMaterial> * pModelMaterials)
{
   if (!ms3dMaterials.empty())
   {
      vector<cMs3dMaterial>::const_iterator iter = ms3dMaterials.begin(), end = ms3dMaterials.end();
      for (; iter != end; ++iter)
      {
         sModelMaterial m;
         memcpy(m.diffuse, iter->GetDiffuse(), sizeof(m.diffuse));
         cStr texture;
         iter->GetTexture(&texture);
         _tcscpy(m.szTexture, texture.c_str());
         pModelMaterials->push_back(m);
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dModel
//

////////////////////////////////////////

cMs3dModel::cMs3dModel()
 : m_animationFPS(0)
 , m_currentTime(0)
 , m_nTotalFrames(0)
{
}

////////////////////////////////////////

cMs3dModel::~cMs3dModel()
{
}

////////////////////////////////////////

IModel * cMs3dModel::Read(IReader * pReader)
{
   if (pReader == NULL)
   {
      return NULL;
   }

   LocalMsg("Loading MS3D file...\n");

   //////////////////////////////
   // Read the header

   cMs3dHeader header;
   if (pReader->Read(&header) != S_OK)
   {
      ErrorMsg("Error reading MS3D file header\n");
      return NULL;
   }

   if (!header.EqualTo(cMs3dHeader::gm_ms3dHeader))
   {
      ErrorMsg("Bad MS3D file header\n");
      return NULL;
   }

   //////////////////////////////
   // Read the vertices

   if (ReadVector<cMs3dVertex, uint16>(pReader, &ms3dVerts) != S_OK)
   {
      return NULL;
   }

   LocalMsg1("%d Vertices\n", ms3dVerts.size());

   //////////////////////////////
   // Read the triangles

   if (ReadVector<cMs3dTriangle, uint16>(pReader, &ms3dTris) != S_OK)
   {
      return NULL;
   }

   LocalMsg1("%d Triangles\n", ms3dTris.size());

   //////////////////////////////
   // Read the groups

   if (ReadVector<cMs3dGroup, uint16>(pReader, &ms3dGroups) != S_OK)
   {
      return NULL;
   }

   LocalMsg1("%d Groups\n", ms3dGroups.size());

   //////////////////////////////

   vector<sModelVertex> vertices;
   vector<sModelMesh> meshes2;
   vector<uint16> indices;
   CompileMeshes(ms3dVerts, ms3dTris, ms3dGroups, &vertices, &meshes2, &indices);

   //////////////////////////////
   // Read the materials

   if (FAILED(ReadVector<cMs3dMaterial, uint16>(pReader, &ms3dMaterials)))
   {
      return NULL;
   }

   LocalMsg1("%d Materials\n", ms3dMaterials.size());

   vector<sModelMaterial> materials;
   CompileMaterials(ms3dMaterials, &materials);

   //////////////////////////////
   // Read the animation info

   if (pReader->Read(&m_animationFPS, sizeof(m_animationFPS)) != S_OK
      || pReader->Read(&m_currentTime, sizeof(m_currentTime)) != S_OK
      || pReader->Read(&m_nTotalFrames, sizeof(m_nTotalFrames)) != S_OK)
   {
      return NULL;
   }

   LocalMsg1("%d Animation Frames\n", m_nTotalFrames);
   LocalMsg1("Animation FPS = %f\n", m_animationFPS);

   //////////////////////////////
   // Read the joints

   if (FAILED(ReadVector<cMs3dJoint, uint16>(pReader, &ms3dJoints)))
   {
      return NULL;
   }

   LocalMsg1("%d Joints\n", ms3dJoints.size());

   uint nJoints = ms3dJoints.size();

   vector<sModelJoint> joints(nJoints);
   vector< vector<sModelKeyFrame> > jointKeyFrames(nJoints);

   if (!ms3dJoints.empty())
   {
      map<cStr, int> jointNameMap;
      vector<cMs3dJoint>::iterator iter = ms3dJoints.begin();
      vector<cMs3dJoint>::iterator end = ms3dJoints.end();
      for (uint i = 0; iter != end; iter++, i++)
      {
         jointNameMap.insert(make_pair(iter->GetName(), i));
      }

      iter = ms3dJoints.begin();
      end = ms3dJoints.end();
      for (uint i = 0; iter != end; iter++, i++)
      {
         LocalMsg1("Joint %d\n", i);

         int parentIndex = -1;

         if (strlen(iter->GetParentName()) > 0)
         {
            map<cStr, int>::iterator found = jointNameMap.find(iter->GetParentName());
            if (found != jointNameMap.end())
            {
               parentIndex = found->second;
            }
         }

         AssertMsg(iter->GetRotationKeys().size() == iter->GetPositionKeys().size(),
            _T("Should have been rejected by cMs3dJoint reader"));

         vector<sModelKeyFrame> keyFrames(iter->GetRotationKeys().size());

         const vector<sMs3dRotationKeyframe> & keyFramesRot = iter->GetRotationKeys();
         const vector<sMs3dPositionKeyframe> & keyFramesTrans = iter->GetPositionKeys();
         for (uint j = 0; j < keyFrames.size(); j++)
         {
            if (keyFramesRot[j].time != keyFramesTrans[j].time)
            {
               ErrorMsg("Time of rotation key frame not same as translation key frame\n");
               return NULL;
            }

            keyFrames[j].time = keyFramesRot[j].time;
            keyFrames[j].translation = tVec3(keyFramesTrans[j].position);
            keyFrames[j].rotation = QuatFromEulerAngles(tVec3(keyFramesRot[j].rotation));

            int frame = FloatToInt(static_cast<float>(keyFrames[j].time) * m_animationFPS);
            LocalMsg3("Key frame %d at %.3f is #%d\n", j, keyFrames[j].time, frame);
         }

         jointKeyFrames[i].resize(keyFrames.size());
         copy(keyFrames.begin(), keyFrames.end(), jointKeyFrames[i].begin());

         joints[i].localTranslation = tVec3(iter->GetPosition());
         joints[i].localRotation = QuatFromEulerAngles(tVec3(iter->GetRotation()));
         joints[i].parentIndex = parentIndex;
      }
   }

   {
      int iRootJoint = -1;
      vector<sModelJoint>::const_iterator iter = joints.begin();
      for (int i = 0; iter != joints.end(); iter++, i++)
      {
         if (iter->parentIndex < 0)
         {
            if (iRootJoint >= 0)
            {
               ErrorMsg("More than one root joint");
               return NULL;
            }
            iRootJoint = i;
         }
      }
   }

   cAutoIPtr<IModelSkeleton> pSkeleton;
   if (!joints.empty())
   {
      if (ModelSkeletonCreate(&joints[0], joints.size(), &pSkeleton) != S_OK)
      {
         ErrorMsg("Failed to create skeleton for model\n");
         return NULL;
      }
   }

   //////////////////////////////
   // Read the comments, if present (MilkShape versions 1.7+)

   int subVersion = 0;
   if (pReader->Read(&subVersion, sizeof(subVersion)) == S_OK
      && subVersion == 1)
   {
      ReadComments(pReader, &m_groupComments);
      ReadComments(pReader, &m_materialComments);
      ReadComments(pReader, &m_jointComments);

      {
         int hasModelComment = 0;
         if (pReader->Read(&hasModelComment, sizeof(hasModelComment)) == S_OK
            && hasModelComment == 1)
         {
            int length;
            if (pReader->Read(&length, sizeof(length)) == S_OK)
            {
               char * pszTemp = static_cast<char *>(alloca((length + 1) * sizeof(char)));
               if (pReader->Read(pszTemp, length * sizeof(char)) == S_OK)
               {
                  pszTemp[length] = 0;
                  m_modelComment.assign(pszTemp);
               }
            }
         }
      }
   }

   vector<sModelAnimationDesc> animDescs;
   if (!m_modelComment.empty())
   {
      ParseAnimDescs(m_modelComment.c_str(), &animDescs);
   }

   if (!animDescs.empty())
   {
      float totalAnimTime = static_cast<float>(m_nTotalFrames) / m_animationFPS;

      LocalMsg1("%d Animation Sequences\n", animDescs.size());

      vector<sModelAnimationDesc>::const_iterator iter, end;
      for (iter = animDescs.begin(), end = animDescs.end(); iter != end; iter++)
      {
         const sModelAnimationDesc & animDesc = *iter;

         static const char * const animTypes[] =
         {
            "Walk",
            "Run",
            "Death",
            "Attack",
            "Damage",
            "Idle",
         };

         LocalMsg3("%s: %d, %d\n", animTypes[animDesc.type], animDesc.start, animDesc.end);

         bool bError = false;
         vector<IModelKeyFrameInterpolator*> interpolators;
         for (uint i = 0; i < jointKeyFrames.size(); i++)
         {
            const vector<sModelKeyFrame> & keyFrames = jointKeyFrames[i];

            LocalMsg2("Joint %d KeyFrames (size = %d)\n", i, keyFrames.size());

            int iStart = -1, iEnd = -1;
            vector<sModelKeyFrame>::const_iterator kfIter = keyFrames.begin(), kfEnd = keyFrames.end();
            for (int iKeyFrame = 0; kfIter != kfEnd; ++iKeyFrame, ++kfIter)
            {
               uint frame = FloatToInt(static_cast<float>(kfIter->time) * m_animationFPS);
               if (frame == animDesc.start)
               {
                  iStart = iKeyFrame;
               }
               if (frame >= animDesc.start && LOG_IS_CHANNEL_ENABLED(Ms3dModel))
               {
                  LocalMsg1("Time: %f, ", kfIter->time);
                  LogMsgNoFL3(kDebug, "Translation <%f, %f, %f>, ", kfIter->translation.x, kfIter->translation.y, kfIter->translation.z);
                  LogMsgNoFL4(kDebug, "Rotation (%f, %f, %f, %f)\n", kfIter->rotation.x, kfIter->rotation.y, kfIter->rotation.z, kfIter->rotation.w);
               }
               if (frame >= animDesc.end)
               {
                  iEnd = iKeyFrame;
                  break;
               }
            }

            LocalMsg2("%d KeyFrames for joint %d\n", iEnd - iStart + 1, i);

            cAutoIPtr<IModelKeyFrameInterpolator> pInterp;

            if (iStart == -1 || iEnd == -1
               || ModelKeyFrameInterpolatorCreate(&keyFrames[iStart], iEnd - iStart + 1, &pInterp) != S_OK)
            {
               bError = true;
               break;
            }

            interpolators.push_back(CTAddRef(pInterp));
         }

         if (!bError)
         {
            cAutoIPtr<IModelAnimation> pAnim;
            if (ModelAnimationCreate(&interpolators[0], interpolators.size(), &pAnim) == S_OK)
            {
               pSkeleton->AddAnimation(animDesc.type, pAnim);
            }
         }

         for_each(interpolators.begin(), interpolators.end(), mem_fn(&IModelKeyFrameInterpolator::Release));
      }
   }

   //////////////////////////////
   // Create the model

   IModel * pModel = NULL;
   if (ModelCreate(
      &vertices[0], vertices.size(),
      &indices[0], indices.size(),
      &meshes2[0], meshes2.size(),
      materials.empty() ? NULL : &materials[0], materials.size(),
      pSkeleton, &pModel) == S_OK)
   {
      return pModel;
   }

   return NULL;
}

////////////////////////////////////////

void * cMs3dModel::Load(IReader * pReader)
{
   cMs3dModel ms3dModel;
   return ms3dModel.Read(pReader);
}

////////////////////////////////////////

void cMs3dModel::Unload(void * pData)
{
   IModel * pModel = reinterpret_cast<IModel*>(pData);
   SafeRelease(pModel);
}


////////////////////////////////////////////////////////////////////////////////

tResult Ms3dModelResourceRegister(IResourceManager * pResourceManager)
{
   if (pResourceManager == NULL)
   {
      return E_POINTER;
   }
   return pResourceManager->RegisterFormat(kRT_Model, _T("ms3d"), cMs3dModel::Load, NULL, cMs3dModel::Unload);
}

////////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

static const byte g_boxModel[] =
{
   0x4D, 0x53, 0x33, 0x44, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x04, 0x00, 0x00,
   0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x08, 0xC0, 0x00, 0x00, 0x20, 0x40, 0x00,
   0x00, 0x10, 0x40, 0xFF, 0x05, 0x00, 0x00, 0x00, 0x08, 0xC0, 0x00, 0x00, 0xE0,
   0xBF, 0x00, 0x00, 0x10, 0x40, 0xFF, 0x04, 0x00, 0x00, 0x00, 0x08, 0x40, 0x00,
   0x00, 0x20, 0x40, 0x00, 0x00, 0x10, 0x40, 0xFF, 0x04, 0x00, 0x00, 0x00, 0x08,
   0x40, 0x00, 0x00, 0xE0, 0xBF, 0x00, 0x00, 0x10, 0x40, 0xFF, 0x05, 0x00, 0x00,
   0x00, 0x08, 0x40, 0x00, 0x00, 0x20, 0x40, 0x00, 0x00, 0x00, 0xC0, 0xFF, 0x05,
   0x00, 0x00, 0x00, 0x08, 0x40, 0x00, 0x00, 0xE0, 0xBF, 0x00, 0x00, 0x00, 0xC0,
   0xFF, 0x04, 0x00, 0x00, 0x00, 0x08, 0xC0, 0x00, 0x00, 0x20, 0x40, 0x00, 0x00,
   0x00, 0xC0, 0xFF, 0x04, 0x00, 0x00, 0x00, 0x08, 0xC0, 0x00, 0x00, 0xE0, 0xBF,
   0x00, 0x00, 0x00, 0xC0, 0xFF, 0x05, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
   0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
   0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
   0x00, 0x00, 0x01, 0x00, 0x03, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F,
   0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00,
   0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00,
   0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03,
   0x00, 0x05, 0x00, 0x04, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80,
   0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00,
   0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x05, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xBF, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xBF, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xBF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
   0x3F, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x07, 0x00,
   0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
   0xBF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xBF,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xBF, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00,
   0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
   0x00, 0x06, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xBF, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xBF, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xBF, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x07, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x80, 0xBF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x80, 0xBF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
   0xBF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00,
   0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x00,
   0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x03,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
   0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F,
   0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x07, 0x00, 0x03,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xBF, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xBF, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xBF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
   0x07, 0x00, 0x05, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
   0xBF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xBF,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xBF, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00,
   0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00,
   0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x42, 0x6F, 0x78, 0x30, 0x31, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C,
   0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 0x05, 0x00,
   0x06, 0x00, 0x07, 0x00, 0x08, 0x00, 0x09, 0x00, 0x0A, 0x00, 0x0B, 0x00, 0xFF,
   0x00, 0x00, 0x00, 0x00, 0xC0, 0x41, 0x00, 0x00, 0x80, 0x3F, 0x1E, 0x00, 0x00,
   0x00, 0x02, 0x00, 0x08, 0x6A, 0x6F, 0x69, 0x6E, 0x74, 0x31, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xDB, 0x0F, 0x49, 0x40, 0xF5, 0x45, 0xAA, 0x3D, 0xDB, 0x0F,
   0x49, 0x40, 0x00, 0x00, 0x80, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0,
   0x3F, 0x00, 0x00, 0x00, 0x00, 0x08, 0x6A, 0x6F, 0x69, 0x6E, 0x74, 0x32, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6A,
   0x6F, 0x69, 0x6E, 0x74, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5F,
   0xAA, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF,
   0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
   0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00,
   0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
   0x00
};

SUITE(Ms3dModel)
{
   TEST(BasicRead)
   {
      cAutoIPtr<IReader> pReader;
      CHECK_EQUAL(S_OK, MemReaderCreate(g_boxModel, _countof(g_boxModel), false, &pReader));

      cAutoIPtr<IModel> pModel(reinterpret_cast<IModel*>(cMs3dModel::Load(pReader)));
      CHECK(!!pModel);
   }
}


#endif // HAVE_UNITTESTPP

////////////////////////////////////////////////////////////////////////////////
