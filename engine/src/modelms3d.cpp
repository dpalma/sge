////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "model.h"
#include "ms3dread.h"
#include "ms3d.h"
#include "renderapi.h"

#include "vec4.h"
#include "matrix4.h"
#include "resourceapi.h"
#include "readwriteapi.h"
#include "globalobj.h"
#include "filespec.h"
#include "techmath.h"

#include <algorithm>
#include <cfloat>
#include <map>
#include <stack>

#include "dbgalloc.h" // must be last header


////////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(ModelMs3d);

#if 0
#define LocalMsg(ind,msg)           DebugMsgEx2(ModelMs3d, "%*s" msg, (ind),"")
#define LocalMsg1(ind,msg,a)        DebugMsgEx3(ModelMs3d, "%*s" msg, (ind),"",(a))
#define LocalMsg2(ind,msg,a,b)      DebugMsgEx4(ModelMs3d, "%*s" msg, (ind),"",(a),(b))
#define LocalMsg3(ind,msg,a,b,c)    DebugMsgEx5(ModelMs3d, "%*s" msg, (ind),"",(a),(b),(c))
#else
#define LocalMsg(ind,msg)           DebugMsgEx(ModelMs3d,msg)
#define LocalMsg1(ind,msg,a)        DebugMsgEx1(ModelMs3d,msg,(a))
#define LocalMsg2(ind,msg,a,b)      DebugMsgEx2(ModelMs3d,msg,(a),(b))
#define LocalMsg3(ind,msg,a,b,c)    DebugMsgEx3(ModelMs3d,msg,(a),(b),(c))
#endif

////////////////////////////////////////////////////////////////////////////////

template <typename CONTAINER>
void ParseAnimDescs(const tChar * pszAnimString, CONTAINER * pContainer)
{
   cTokenizer<cStr> strTok;
   if (strTok.Tokenize(pszAnimString, _T("\n")) > 0)
   {
      std::vector<cStr> & animStrings = strTok.m_tokens;

      std::vector<cStr>::iterator iter = animStrings.begin();
      for (; iter != animStrings.end(); iter++)
      {
         cStr & animString = *iter;

         TrimLeadingSpace(&animString);
         TrimTrailingSpace(&animString);

         cTokenizer<cStr> strTok2;
         if (strTok2.Tokenize(iter->c_str()) == 3)
         {
            std::vector<cStr> & temp = strTok2.m_tokens;

            static const struct
            {
               eModelAnimationType type;
               const char * pszType;
            }
            animTypes[] =
            {
               { kMAT_Walk, "walk" },
               { kMAT_Run, "run" },
               { kMAT_Death, "death" },
               { kMAT_Attack, "attack" },
               { kMAT_Damage, "damage" },
               { kMAT_Idle, "idle" },
            };

            const cStr & animType = temp[2];

            for (int j = 0; j < _countof(animTypes); j++)
            {
               if (animType.compare(animTypes[j].pszType) == 0)
               {
                  sModelAnimationDesc animDesc;
                  animDesc.type = animTypes[j].type;
#ifdef __GNUC__
                  animDesc.start = strtol(temp[0].c_str(), NULL, 10);
                  animDesc.end = strtol(temp[1].c_str(), NULL, 10);
#else
                  animDesc.start = _ttoi(temp[0].c_str());
                  animDesc.end = _ttoi(temp[1].c_str());
#endif
                  animDesc.fps = 0;
                  if (animDesc.start > 0 || animDesc.end > 0)
                  {
                     pContainer->push_back(animDesc);
                  }
                  break;
               }
            }
         }
      }
   }
}

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


static const char g_ms3dId[] = "MS3D000000";
static const int g_ms3dVer = 4; // ms3d files this version or later are supported

void * ModelMs3dLoad(IReader * pReader)
{
   if (pReader == NULL)
   {
      return NULL;
   }

   static const int kIndent = 3;
   uint indent = 0;

   LocalMsg(indent, "Loading MS3D file...\n");

   indent += kIndent;

   //////////////////////////////
   // Read the header

   ms3d_header_t header;
   if (pReader->Read(&header, sizeof(header)) != S_OK ||
      memcmp(g_ms3dId, header.id, _countof(header.id)) != 0 ||
      header.version < g_ms3dVer)
   {
      ErrorMsg("Bad MS3D file header\n");
      return NULL;
   }

   //////////////////////////////
   // Read the vertices

   uint16 nVertices;
   if (pReader->Read(&nVertices, sizeof(nVertices)) != S_OK
      || nVertices == 0)
   {
      return NULL;
   }

   LocalMsg1(indent, "%d Vertices\n", nVertices);

   std::vector<ms3d_vertex_t> ms3dVerts(nVertices);
   if (pReader->Read(&ms3dVerts[0], nVertices * sizeof(ms3d_vertex_t)) != S_OK)
   {
      return NULL;
   }

   //////////////////////////////
   // Read the triangles

   uint16 nTriangles;
   if (pReader->Read(&nTriangles, sizeof(nTriangles)) != S_OK
      || nTriangles == 0)
   {
      return NULL;
   }

   LocalMsg1(indent, "%d Triangles\n", nTriangles);

   std::vector<ms3d_triangle_t> tris(nTriangles);
   if (pReader->Read(&tris[0], nTriangles * sizeof(ms3d_triangle_t)) != S_OK)
   {
      return NULL;
   }

   //////////////////////////////
   // Re-map the vertices based on the triangles because Milkshape file
   // triangles contain some vertex info.

   // TODO: clean up this vertex mapping code !!!!!!!

   std::vector<sModelVertex> vertices;
   vertices.resize(nVertices);

   cMs3dVertexMapper vertexMapper(ms3dVerts);

   typedef std::multimap<uint, uint> tVertexMap;
   tVertexMap vertexMap;

   std::vector<ms3d_triangle_t>::const_iterator iter;
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
            vertices[index].pos = ms3dVerts[index].vertex;
            vertices[index].bone = ms3dVerts[index].boneId;
            vertexMap.insert(std::make_pair(index,index));
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
                  vertexMap.insert(std::make_pair(index,vertices.size()-1));
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

   LocalMsg1(indent, "%d Groups\n", nGroups);

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

   tModelMeshes meshes(nGroups);

   for (i = 0; i < nGroups; i++)
   {
      const cMs3dGroup & group = groups[i];

      std::vector<uint16> mappedIndices;
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

      meshes[i] = cModelMesh(kPT_Triangles, mappedIndices, group.GetMaterialIndex());
   }

   //////////////////////////////
   // Read the materials

   uint16 nMaterials;
   if (pReader->Read(&nMaterials, sizeof(nMaterials)) != S_OK)
   {
      return NULL;
   }

   LocalMsg1(indent, "%d Materials\n", nMaterials);

   std::vector<cModelMaterial> materials(nMaterials);

   if (nMaterials > 0)
   {
      for (i = 0; i < nMaterials; i++)
      {
         ms3d_material_t ms3dMat;
         if (pReader->Read(&ms3dMat, sizeof(ms3d_material_t)) != S_OK)
         {
            return NULL;
         }

         // MilkShape stores texture file names as "./texture.bmp" so run
         // the texture name through cFileSpec to fix it up.
         cStr texture;
         cFileSpec(ms3dMat.texture).GetFileNameNoExt(&texture);

         materials[i] = cModelMaterial(ms3dMat.diffuse, texture.c_str());
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

   LocalMsg1(indent, "%d Animation Frames\n", nTotalFrames);
   LocalMsg1(indent, "Animation FPS = %f\n", animationFPS);

   //////////////////////////////
   // Read the joints

   uint16 nJoints;
   if (pReader->Read(&nJoints, sizeof(nJoints)) != S_OK)
   {
      return NULL;
   }

   LocalMsg1(indent, "%d Joints\n", nJoints);

   std::vector<sModelJoint> joints(nJoints);
   std::vector< std::vector<sModelKeyFrame> > jointKeyFrames(nJoints);

   if (nJoints > 0)
   {
      indent += kIndent;

      std::vector<cMs3dJoint> ms3dJoints(nJoints);

      std::map<cStr, int> jointNameMap;

      for (i = 0; i < nJoints; i++)
      {
         if (pReader->Read(&ms3dJoints[i]) != S_OK)
         {
            return NULL;
         }

         jointNameMap.insert(std::make_pair(ms3dJoints[i].GetName(), i));
      }

      std::vector<cMs3dJoint>::iterator iter = ms3dJoints.begin();
      std::vector<cMs3dJoint>::iterator end = ms3dJoints.end();
      for (i = 0; iter != end; iter++, i++)
      {
         LocalMsg1(indent, "Joint %d\n", i);

         int parentIndex = -1;

         if (strlen(iter->GetParentName()) > 0)
         {
            std::map<cStr, int>::iterator found = jointNameMap.find(iter->GetParentName());
            if (found != jointNameMap.end())
            {
               parentIndex = found->second;
            }
         }

         tMatrix4 mt, mr, local;
         MatrixTranslate(iter->GetPosition()[0], iter->GetPosition()[1], iter->GetPosition()[2], &mt);
         MatrixFromAngles(tVec3(iter->GetRotation()), &mr);
         mt.Multiply(mr, &local);

         AssertMsg(iter->GetKeyFramesRot().size() == iter->GetKeyFramesTrans().size(),
            _T("Should have been rejected by cMs3dJoint reader"));

         tModelKeyFrames keyFrames(iter->GetKeyFramesRot().size());

         indent += kIndent;

         const std::vector<ms3d_keyframe_rot_t> & keyFramesRot = iter->GetKeyFramesRot();
         const std::vector<ms3d_keyframe_pos_t> & keyFramesTrans = iter->GetKeyFramesTrans();
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
            LocalMsg3(indent, "Key frame %d at %.3f is #%d\n", j, keyFrames[j].time, frame);
         }

         indent -= kIndent;

         jointKeyFrames[i].resize(keyFrames.size());
         std::copy(keyFrames.begin(), keyFrames.end(), jointKeyFrames[i].begin());

         joints[i].localTransform = local;
         joints[i].parentIndex = parentIndex;
      }

      indent -= kIndent;
   }

   {
      int iRootJoint = -1;
      tModelJoints::const_iterator iter = joints.begin();
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
   if (ModelSkeletonCreate(joints, &pSkeleton) != S_OK)
   {
      ErrorMsg("Failed to create skeleton for model\n");
      return NULL;
   }

   //////////////////////////////
   // Read the comments, if present (MilkShape versions 1.7+)

   std::vector<sModelAnimationDesc> animDescs;

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
      LocalMsg1(indent, "%d Animation Sequences\n", animDescs.size());
      indent += kIndent;

      std::vector<sModelAnimationDesc>::const_iterator iter, end;
      for (iter = animDescs.begin(), end = animDescs.end(); iter != end; iter++)
      {
         const sModelAnimationDesc & animDesc = *iter;

         LocalMsg3(indent, "%d: %d, %d\n", animDesc.type, animDesc.start, animDesc.end);

         bool bError = false;
         std::vector<IModelKeyFrameInterpolator*> interpolators;
         for (uint i = 0; i < jointKeyFrames.size(); i++)
         {
            const std::vector<sModelKeyFrame> & keyFrames = jointKeyFrames[i];

            int iStart = -1, iEnd = -1;
            std::vector<sModelKeyFrame>::const_iterator kfIter = keyFrames.begin();
            for (int iKeyFrame = 0; kfIter != keyFrames.end(); iKeyFrame++, kfIter++)
            {
               uint frame = FloatToInt(static_cast<float>(kfIter->time) * animationFPS);
               if (frame == animDesc.start)
               {
                  iStart = iKeyFrame;
               }
               if (frame >= animDesc.end)
               {
                  iEnd = iKeyFrame;
                  break;
               }
            }

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

         std::for_each(interpolators.begin(), interpolators.end(), CTInterfaceMethod(&IUnknown::Release));
      }

      indent -= kIndent;
   }

   //////////////////////////////
   // Create the model

   cModel * pModel = NULL;
   if (nJoints > 0)
   {
      if (cModel::Create(vertices, materials, meshes, pSkeleton, &pModel) == S_OK)
      {
         return pModel;
      }
   }
   else
   {
      if (cModel::Create(vertices, materials, meshes, &pModel) == S_OK)
      {
         return pModel;
      }
   }

   return NULL;
}

///////////////////////////////////////

void ModelMs3dUnload(void * pData)
{
   cModel * pModel = reinterpret_cast<cModel*>(pData);
   delete pModel;
}


////////////////////////////////////////////////////////////////////////////////

tResult ModelMs3dResourceRegister()
{
   UseGlobal(ResourceManager);
   return pResourceManager->RegisterFormat(kRT_Model, _T("ms3d"), ModelMs3dLoad, NULL, ModelMs3dUnload);
}

////////////////////////////////////////////////////////////////////////////////
