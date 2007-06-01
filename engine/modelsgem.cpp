////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "model.h"

#include "render/renderapi.h"

#include "tech/globalobj.h"
#include "tech/resourceapi.h"
#include "tech/readwriteapi.h"
#include "tech/readwriteutils.h"

#define BOOST_MEM_FN_ENABLE_STDCALL
#include <boost/mem_fn.hpp>

#include <algorithm>

#include "tech/dbgalloc.h" // must be last header

using namespace boost;
using namespace std;

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

   cModelChunk< vector<sModelVertex> > verticesChunk;
   if (pReader->Read(&verticesChunk) != S_OK
      || verticesChunk.GetChunkId() != MODEL_VERTEX_ARRAY_CHUNK)
   {
      ErrorMsg("Bad SGE model vertices\n");
      return NULL;
   }

   //////////////////////////////

   cModelChunk< vector<uint16> > indicesChunk;
   if (pReader->Read(&indicesChunk) != S_OK
      || indicesChunk.GetChunkId() != MODEL_INDEX16_ARRAY_CHUNK)
   {
      ErrorMsg("Bad SGE model indices\n");
      return NULL;
   }

   //////////////////////////////

   cModelChunk< vector<sModelMesh> > meshesChunk;
   if (pReader->Read(&meshesChunk) != S_OK
      || meshesChunk.GetChunkId() != MODEL_MESH_ARRAY_CHUNK)
   {
      ErrorMsg("Bad SGE model meshes\n");
      return NULL;
   }

   //////////////////////////////

   cModelChunk< vector<sModelMaterial> > materialsChunk;
   if (pReader->Read(&materialsChunk) != S_OK
      || materialsChunk.GetChunkId() != MODEL_MATERIAL_ARRAY_CHUNK)
   {
      ErrorMsg("Bad SGE model materials\n");
      return NULL;
   }

   LocalMsg1("%d Materials\n", materialsChunk.GetChunkData().size());

   //////////////////////////////

   cModelChunk< cModelChunk< vector<sModelJoint> > > skeletonChunk;

   if (pReader->Read(&skeletonChunk) != S_OK
      || skeletonChunk.GetChunkId() != MODEL_SKELETON_CHUNK
      || skeletonChunk.GetChunkData().GetChunkId() != MODEL_JOINT_ARRAY_CHUNK)
   {
      ErrorMsg("Bad SGE model skeleton\n");
      return NULL;
   }

   const vector<sModelJoint> & joints = skeletonChunk.GetChunkData().GetChunkData();

   LocalMsg1("%d Joints\n", joints.size());

   cAutoIPtr<IModelSkeleton> pSkeleton;
   if (ModelSkeletonCreate(&joints[0], joints.size(), &pSkeleton) != S_OK)
   {
      return NULL;
   }

   //////////////////////////////

   sModelChunkHeader chunkHeader;
   while (pReader->Read(&chunkHeader, sizeof(chunkHeader)) == S_OK)
   {
      if (chunkHeader.chunkId == MODEL_ANIMATION_SEQUENCE_CHUNK)
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

         vector< vector<sModelKeyFrame> > animKeyFrameVectors;
         if (pReader->Read(&animKeyFrameVectors) != S_OK)
         {
            return NULL;
         }

         vector<IModelKeyFrameInterpolator*> interpolators;

         vector< vector<sModelKeyFrame> >::const_iterator iter = animKeyFrameVectors.begin();
         vector< vector<sModelKeyFrame> >::const_iterator end = animKeyFrameVectors.end();
         for (; iter != end; ++iter)
         {
            const vector<sModelKeyFrame> & keyFrames = *iter;

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

            for_each(interpolators.begin(), interpolators.end(), mem_fn(&IModelKeyFrameInterpolator::Release));
         }
      }
      else
      {
         WarnMsg2("Skipping unknown chunk %d, length %d\n", chunkHeader.chunkId, chunkHeader.chunkLength);
         pReader->Seek(chunkHeader.chunkLength - sizeof(chunkHeader), kSO_Cur);
      }
   }

   //////////////////////////////

   const std::vector<sModelVertex> & verts = verticesChunk.GetChunkData();
   const std::vector<uint16> & indices = indicesChunk.GetChunkData();
   const std::vector<sModelMesh> & meshes = meshesChunk.GetChunkData();
   const std::vector<sModelMaterial> & materials = materialsChunk.GetChunkData();

   IModel * pModel = NULL;
   if (!joints.empty() && !!pSkeleton)
   {
      if (cModel::Create(&verts[0], verts.size(), &indices[0], indices.size(),
         &meshes[0], meshes.size(), &materials[0], materials.size(), pSkeleton, &pModel) == S_OK)
      {
         return pModel;
      }
   }
   else
   {
      if (cModel::Create(&verts[0], verts.size(), &indices[0], indices.size(),
         &meshes[0], meshes.size(), &materials[0], materials.size(), NULL, &pModel) == S_OK)
      {
         return pModel;
      }
   }

   return NULL;
}

///////////////////////////////////////

void ModelSgemUnload(void * pData)
{
   IModel * pModel = reinterpret_cast<IModel*>(pData);
   SafeRelease(pModel);
}


////////////////////////////////////////////////////////////////////////////////

tResult ModelSgemResourceRegister()
{
   UseGlobal(ResourceManager);
   return pResourceManager->RegisterFormat(kRT_Model, _T("sgem"), ModelSgemLoad, NULL, ModelSgemUnload);
}

////////////////////////////////////////////////////////////////////////////////
