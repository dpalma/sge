////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ms3dgroup.h"
#include "ms3dheader.h"
#include "ms3djoint.h"
#include "ms3dtriangle.h"
#include "ms3d.h"
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

LOG_DEFINE_CHANNEL(ModelMs3d);

#define LocalMsg(msg)                  DebugMsgEx(ModelMs3d,msg)
#define LocalMsg1(msg,a)               DebugMsgEx1(ModelMs3d,msg,(a))
#define LocalMsg2(msg,a,b)             DebugMsgEx2(ModelMs3d,msg,(a),(b))
#define LocalMsg3(msg,a,b,c)           DebugMsgEx3(ModelMs3d,msg,(a),(b),(c))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(ModelMs3d,(cond),msg)
#define LocalMsgIf1(cond,msg,a)        DebugMsgIfEx1(ModelMs3d,(cond),msg,(a))
#define LocalMsgIf2(cond,msg,a,b)      DebugMsgIfEx2(ModelMs3d,(cond),msg,(a),(b))
#define LocalMsgIf3(cond,msg,a,b,c)    DebugMsgIfEx3(ModelMs3d,(cond),msg,(a),(b),(c))

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

void * ModelMs3dLoad(IReader * pReader)
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

   vector<cMs3dVertex> ms3dVerts;
   if (ReadVector<cMs3dVertex, uint16>(pReader, &ms3dVerts) != S_OK)
   {
      return NULL;
   }

   LocalMsg1("%d Vertices\n", ms3dVerts.size());

   //////////////////////////////
   // Read the triangles

   uint16 nTriangles;
   if (pReader->Read(&nTriangles, sizeof(nTriangles)) != S_OK
      || nTriangles == 0)
   {
      return NULL;
   }

   LocalMsg1("%d Triangles\n", nTriangles);

   vector<ms3d_triangle_t> tris(nTriangles);
   if (pReader->Read(&tris[0], nTriangles * sizeof(ms3d_triangle_t)) != S_OK)
   {
      return NULL;
   }

   //////////////////////////////
   // Re-map the vertices based on the triangles because Milkshape file
   // triangles contain some vertex info.

   // TODO: clean up this vertex mapping code !!!!!!!

   vector<sModelVertex> vertices;
   vertices.resize(ms3dVerts.size());

   cVertexMapper vertexMapper(ms3dVerts);

   typedef multimap<uint, uint> tVertexMap;
   tVertexMap vertexMap;

   vector<ms3d_triangle_t>::const_iterator iter;
   for (iter = tris.begin(); iter != tris.end(); iter++)
   {
      for (int k = 0; k < 3; k++)
      {
         vertexMapper.MapVertex(
            iter->vertexIndices[k], 
            iter->vertexNormals[k], 
            iter->s[k], 
            iter->t[k]);

         uint index = iter->vertexIndices[k];
         if (vertexMap.find(index) == vertexMap.end())
         {
            vertices[index].u = iter->s[k];
            vertices[index].v = 1 - iter->t[k];
            vertices[index].normal = iter->vertexNormals[k];
            vertices[index].pos = ms3dVerts[index].GetPosition();
            vertices[index].bone = ms3dVerts[index].GetBone();
            vertexMap.insert(make_pair(index,index));
         }
         else
         {
            sModelVertex vert = vertices[index];
            vert.u = iter->s[k];
            vert.v = 1 - iter->t[k];
            vert.normal = iter->vertexNormals[k];
            uint iVertMatch = ~0u;
            tVertexMap::iterator viter = vertexMap.lower_bound(index);
            tVertexMap::iterator vend = vertexMap.upper_bound(index);
            for (; viter != vend; viter++)
            {
               uint index2 = viter->second;
               if (index2 != index)
               {
                  const sModelVertex & vert1 = vert;
                  const sModelVertex & vert2 = vertices[index2];
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
                  const sModelVertex & vert2 = vertices[viter->second];
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
                  vertices.push_back(vert);
                  vertexMap.insert(make_pair(index,vertices.size()-1));
               }
            }
         }
      }
   }

//   DebugMsg2("Mapped vertex array has %d members (originally %d)\n", vertices.size(), ms3dVerts.size());

   //////////////////////////////
   // Read the groups

   uint16 nGroups;
   if (pReader->Read(&nGroups, sizeof(nGroups)) != S_OK)
   {
      return NULL;
   }

   LocalMsg1("%d Groups\n", nGroups);

   uint i;
   cMs3dGroup groups[MAX_GROUPS];
   for (i = 0; i < nGroups; i++)
   {
      if (pReader->Read(&groups[i]) != S_OK)
      {
         return NULL;
      }
   }

   //////////////////////////////
   // Prepare the groups for the model

   vector<sModelMesh> meshes2(nGroups);
   vector<uint16> indices;

   for (i = 0; i < nGroups; i++)
   {
      const cMs3dGroup & group = groups[i];

      vector<uint16> mappedIndices;
      for (uint j = 0; j < group.GetNumTriangles(); j++)
      {
         const ms3d_triangle_t & tri = tris[group.GetTriangle(j)];
         for (int k = 0; k < 3; k++)
         {
            mappedIndices.push_back(vertexMapper.MapVertex(
               tri.vertexIndices[k], tri.vertexNormals[k], 
               tri.s[k], tri.t[k]));
         }
      }

      meshes2[i].primitive = kPT_Triangles;
      meshes2[i].materialIndex = group.GetMaterialIndex();
      meshes2[i].indexStart = indices.size();
      meshes2[i].nIndices = mappedIndices.size();

      indices.insert(indices.end(), mappedIndices.begin(), mappedIndices.end());
   }

   //////////////////////////////
   // Read the materials

   uint16 nMaterials;
   if (pReader->Read(&nMaterials, sizeof(nMaterials)) != S_OK)
   {
      return NULL;
   }

   LocalMsg1("%d Materials\n", nMaterials);

   vector<sModelMaterial> materials(nMaterials);

   if (nMaterials > 0)
   {
      for (i = 0; i < nMaterials; i++)
      {
         ms3d_material_t ms3dMat;
         if (pReader->Read(&ms3dMat, sizeof(ms3d_material_t)) != S_OK)
         {
            return NULL;
         }

         memcpy(materials[i].diffuse, ms3dMat.diffuse, sizeof(materials[i].diffuse));

         // MilkShape stores texture file names as "./texture.bmp" so run
         // the texture name through cFileSpec to fix it up.
         cStr texture;
         cFileSpec(ms3dMat.texture).GetFileNameNoExt(&texture);

         _tcscpy(materials[i].szTexture, texture.c_str());
      }
   }

   //////////////////////////////
   // Read the animation info

   float animationFPS;
   float currentTime;
   int nTotalFrames;
   if (pReader->Read(&animationFPS, sizeof(animationFPS)) != S_OK
      || pReader->Read(&currentTime, sizeof(currentTime)) != S_OK
      || pReader->Read(&nTotalFrames, sizeof(nTotalFrames)) != S_OK)
   {
      return NULL;
   }

   LocalMsg1("%d Animation Frames\n", nTotalFrames);
   LocalMsg1("Animation FPS = %f\n", animationFPS);

   //////////////////////////////
   // Read the joints

   uint16 nJoints;
   if (pReader->Read(&nJoints, sizeof(nJoints)) != S_OK)
   {
      return NULL;
   }

   LocalMsg1("%d Joints\n", nJoints);

   vector<sModelJoint> joints(nJoints);
   vector< vector<sModelKeyFrame> > jointKeyFrames(nJoints);

   if (nJoints > 0)
   {
      vector<cMs3dJoint> ms3dJoints(nJoints);

      map<cStr, int> jointNameMap;

      for (i = 0; i < nJoints; i++)
      {
         if (pReader->Read(&ms3dJoints[i]) != S_OK)
         {
            return NULL;
         }

         jointNameMap.insert(make_pair(ms3dJoints[i].GetName(), i));
      }

      vector<cMs3dJoint>::iterator iter = ms3dJoints.begin();
      vector<cMs3dJoint>::iterator end = ms3dJoints.end();
      for (i = 0; iter != end; iter++, i++)
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

         AssertMsg(iter->GetKeyFramesRot().size() == iter->GetKeyFramesTrans().size(),
            _T("Should have been rejected by cMs3dJoint reader"));

         vector<sModelKeyFrame> keyFrames(iter->GetKeyFramesRot().size());

         const vector<ms3d_keyframe_rot_t> & keyFramesRot = iter->GetKeyFramesRot();
         const vector<ms3d_keyframe_pos_t> & keyFramesTrans = iter->GetKeyFramesTrans();
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

            int frame = FloatToInt(static_cast<float>(keyFrames[j].time) * animationFPS);
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

   vector<sModelAnimationDesc> animDescs;

   int subVersion = 0;
   if (pReader->Read(&subVersion, sizeof(subVersion)) == S_OK
      && subVersion == 1)
   {
      {
         int nGroupComments = 0;
         if (pReader->Read(&nGroupComments, sizeof(nGroupComments)) == S_OK
            && nGroupComments > 0)
         {
            for (int i = 0; i < nGroupComments; i++)
            {
               int index, length;
               if (pReader->Read(&index, sizeof(index)) != S_OK
                  || pReader->Read(&length, sizeof(length)) != S_OK)
               {
                  return NULL;
               }
               
               char * pszTemp = (char *)alloca((length + 1) * sizeof(char));
               if (pReader->Read(pszTemp, length * sizeof(char)) != S_OK)
               {
                  return NULL;
               }
               pszTemp[length] = 0;
            }
         }
      }

      {
         int nMaterialComments = 0;
         if (pReader->Read(&nMaterialComments, sizeof(nMaterialComments)) == S_OK
            && nMaterialComments > 0)
         {
            for (int i = 0; i < nMaterialComments; i++)
            {
               int index, length;
               if (pReader->Read(&index, sizeof(index)) != S_OK
                  || pReader->Read(&length, sizeof(length)) != S_OK)
               {
                  return NULL;
               }
               
               char * pszTemp = (char *)alloca((length + 1) * sizeof(char));
               if (pReader->Read(pszTemp, length * sizeof(char)) != S_OK)
               {
                  return NULL;
               }
               pszTemp[length] = 0;
            }
         }
      }

      {
         int nJointComments = 0;
         if (pReader->Read(&nJointComments, sizeof(nJointComments)) == S_OK
            && nJointComments > 0)
         {
            for (int i = 0; i < nJointComments; i++)
            {
               int index, length;
               if (pReader->Read(&index, sizeof(index)) != S_OK
                  || pReader->Read(&length, sizeof(length)) != S_OK)
               {
                  return NULL;
               }
               
               char * pszTemp = (char *)alloca((length + 1) * sizeof(char));
               if (pReader->Read(pszTemp, length * sizeof(char)) != S_OK)
               {
                  return NULL;
               }
               pszTemp[length] = 0;
            }
         }
      }

      {
         int hasModelComment = 0;
         if (pReader->Read(&hasModelComment, sizeof(hasModelComment)) == S_OK
            && hasModelComment == 1)
         {
            int length;
            if (pReader->Read(&length, sizeof(length)) != S_OK)
            {
               return NULL;
            }
            
            char * pszTemp = (char *)alloca((length + 1) * sizeof(char));
            if (pReader->Read(pszTemp, length * sizeof(char)) != S_OK)
            {
               return NULL;
            }
            pszTemp[length] = 0;

            ParseAnimDescs(pszTemp, &animDescs);
         }
      }
   }

   float totalAnimTime = static_cast<float>(nTotalFrames) / animationFPS;

   if (!animDescs.empty())
   {
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
            vector<sModelKeyFrame>::const_iterator kfIter = keyFrames.begin();
            for (int iKeyFrame = 0; kfIter != keyFrames.end(); iKeyFrame++, kfIter++)
            {
               uint frame = FloatToInt(static_cast<float>(kfIter->time) * animationFPS);
               if (frame == animDesc.start)
               {
                  iStart = iKeyFrame;
               }
               if (frame >= animDesc.start && LOG_IS_CHANNEL_ENABLED(ModelMs3d))
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

///////////////////////////////////////

void ModelMs3dUnload(void * pData)
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
   return pResourceManager->RegisterFormat(kRT_Model, _T("ms3d"), ModelMs3dLoad, NULL, ModelMs3dUnload);
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

      cAutoIPtr<IModel> pModel(reinterpret_cast<IModel*>(ModelMs3dLoad(pReader)));
      CHECK(!!pModel);
   }
}


#endif // HAVE_UNITTESTPP

////////////////////////////////////////////////////////////////////////////////
