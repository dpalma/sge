////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "model.h"

#include "renderapi.h"

#include "globalobj.h"
#include "resourceapi.h"
#include "readwriteapi.h"
#include "readwriteutils.h"

#include <algorithm>

#include "dbgalloc.h" // must be last header


////////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(ModelSgem);

#define LocalMsg(msg)            DebugMsgEx(ModelSgem,msg)
#define LocalMsg1(msg,a)         DebugMsgEx1(ModelSgem,msg,(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(ModelSgem,msg,(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(ModelSgem,msg,(a),(b),(c))

////////////////////////////////////////////////////////////////////////////////

void * ModelSgemLoad(IReader * pReader)
{
   if (pReader == NULL)
   {
      return NULL;
   }

   LocalMsg("Loading SGE model file...\n");

   //////////////////////////////
   // Read the header

   cModelChunk<NoChunkData> fileIdChunk;
   if (pReader->Read(&fileIdChunk) != S_OK
      || fileIdChunk.GetChunkId() != MODEL_FILE_ID_CHUNK)
   {
      ErrorMsg("Bad SGE model file header\n");
      return NULL;
   }

   cModelChunk<uint> fileVersionChunk;
   if (pReader->Read(&fileVersionChunk) != S_OK
      || fileVersionChunk.GetChunkId() != MODEL_VERSION_CHUNK
      || fileVersionChunk.GetChunkData() != 1)
   {
      ErrorMsg("Bad SGE model file version\n");
      return NULL;
   }

   //////////////////////////////

   std::vector<sModelVertex> vertices;
   if (pReader->Read(&vertices) != S_OK)
   {
      return NULL;
   }

   //////////////////////////////

   uint nMeshes = 0;
   if (pReader->Read(&nMeshes, sizeof(nMeshes)) != S_OK
      || nMeshes == 0)
   {
      return NULL;
   }

   LocalMsg1("%d Meshes\n", nMeshes);

   tModelMeshes meshes(nMeshes);

   for (uint i = 0; i < nMeshes; ++i)
   {
      static const ePrimitiveType primTypes[] = { kPT_Triangles, kPT_TriangleStrip, kPT_TriangleFan };

      int materialIndex = -1, primType = -1;
      if (pReader->Read(&materialIndex) != S_OK
         || pReader->Read(&primType) != S_OK
         || primType < 0 || primType > 2)
      {
         return NULL;
      }

      std::vector<uint16> indices;
      if (pReader->Read(&indices) != S_OK)
      {
         return NULL;
      }

      meshes[i] = cModelMesh(primTypes[primType], indices, materialIndex);
   }

   //////////////////////////////

   std::vector<sModelMaterial> materials;
   if (pReader->Read(&materials) != S_OK)
   {
      return NULL;
   }

   LocalMsg1("%d Materials\n", materials.size());

   //////////////////////////////

   std::vector<sModelJoint> joints;
   if (pReader->Read(&joints) != S_OK)
   {
      return NULL;
   }

   LocalMsg1("%d Joints\n", joints.size());

   cAutoIPtr<IModelSkeleton> pSkeleton;
   if (ModelSkeletonCreate(&joints[0], joints.size(), &pSkeleton) != S_OK)
   {
      return NULL;
   }

   //////////////////////////////

   uint nAnims = 0;
   if (pReader->Read(&nAnims, sizeof(nAnims)) != S_OK
      || nAnims == 0)
   {
      return NULL;
   }

   LocalMsg1("%d Anims\n", nAnims);

   for (uint i = 0; i < nAnims; ++i)
   {
      static const eModelAnimationType animTypes[] =
      {
         kMAT_Walk,
         kMAT_Run,
         kMAT_Death,
         kMAT_Attack,
         kMAT_Damage,
         kMAT_Idle,
      };

      int intAnimType = -1;
      if (pReader->Read(&intAnimType) != S_OK
         || intAnimType < kMAT_Walk || intAnimType > kMAT_Idle)
      {
         return NULL;
      }

      eModelAnimationType animType = animTypes[intAnimType];

      std::vector< std::vector<sModelKeyFrame> > animKeyFrameVectors;
      if (pReader->Read(&animKeyFrameVectors) != S_OK)
      {
         return NULL;
      }

      std::vector<IModelKeyFrameInterpolator*> interpolators;

      std::vector< std::vector<sModelKeyFrame> >::const_iterator iter = animKeyFrameVectors.begin();
      std::vector< std::vector<sModelKeyFrame> >::const_iterator end = animKeyFrameVectors.end();
      for (; iter != end; ++iter)
      {
         const std::vector<sModelKeyFrame> & keyFrames = *iter;

         cAutoIPtr<IModelKeyFrameInterpolator> pInterp;
         if (ModelKeyFrameInterpolatorCreate(&keyFrames[0], keyFrames.size(), &pInterp) == S_OK)
         {
            interpolators.push_back(CTAddRef(pInterp));
         }
      }

      if (!interpolators.empty())
      {
         cAutoIPtr<IModelAnimation> pAnim;
         if (ModelAnimationCreate(&interpolators[0], interpolators.size(), &pAnim) == S_OK)
         {
            pSkeleton->AddAnimation(animType, pAnim);
         }

         std::for_each(interpolators.begin(), interpolators.end(), CTInterfaceMethod(&IUnknown::Release));
      }
   }

   //////////////////////////////

   cModel * pModel = NULL;
   if (!joints.empty() && !!pSkeleton)
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

void ModelSgemUnload(void * pData)
{
   cModel * pModel = reinterpret_cast<cModel*>(pData);
   delete pModel;
}


////////////////////////////////////////////////////////////////////////////////

tResult ModelSgemResourceRegister()
{
   UseGlobal(ResourceManager);
   return pResourceManager->RegisterFormat(kRT_Model, _T("sgem"), ModelSgemLoad, NULL, ModelSgemUnload);
}

////////////////////////////////////////////////////////////////////////////////
