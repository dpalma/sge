///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ms3dmodel/ms3dmodel.h"

#include "engine/engineapi.h"
#include "engine/modelapi.h"
#include "engine/modeltypes.h"

#include "tech/configapi.h"
#include "tech/dictionaryapi.h"
#include "tech/filespec.h"
#include "tech/globalobj.h"
#include "tech/readwriteapi.h"
#include "tech/readwriteutils.h"
#include "tech/resourceapi.h"
#include "tech/threadcallapi.h"

#include "NvTriStrip.h"

#include <vector>

#include "tech/dbgalloc.h" // must be last header

using namespace std;


///////////////////////////////////////////////////////////////////////////////

static void RegisterGlobalObjects()
{
   ResourceManagerCreate();
   ThreadCallerCreate();
}


///////////////////////////////////////////////////////////////////////////////

static tResult LoadCfgFile(const tChar * pszArgv0, IDictionary * pDict)
{
   if (pszArgv0 == NULL || pDict == NULL)
   {
      return E_POINTER;
   }

   cFileSpec cfgFile(pszArgv0);
   cfgFile.SetFileExt(_T("cfg"));

   cAutoIPtr<IDictionaryStore> pStore = DictionaryStoreCreate(cfgFile);
   if (!pStore)
   {
      return E_OUTOFMEMORY;
   }

   return pStore->Load(pDict);
}

static tResult InitGlobalConfig(int argc, tChar * argv[])
{
   Assert(argc > 0);

   LoadCfgFile(argv[0], g_pConfig);

   ParseCommandLine(argc, argv, g_pConfig);

   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////

static bool MainInit(int argc, tChar * argv[])
{
   if (InitGlobalConfig(argc, argv) != S_OK)
   {
      return false;
   }

   RegisterGlobalObjects();
   if (FAILED(StartGlobalObjects()))
   {
      ErrorMsg("One or more application-level services failed to start!\n");
      return false;
   }

   Verify(EngineRegisterResourceFormats() == S_OK);

   UseGlobal(ThreadCaller);
   if (FAILED(pThreadCaller->ThreadInit()))
   {
      return false;
   }

   return true;
}

///////////////////////////////////////////////////////////////////////////////

static void MainTerm()
{
   UseGlobal(ThreadCaller);
   pThreadCaller->ThreadTerm();

   StopGlobalObjects();
}

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelAnimation
//

class cModelAnimation
{
public:
   cModelAnimation(eModelAnimationType type, const vector< vector<sModelKeyFrame> > & keyFrameVectors);

   eModelAnimationType GetAnimationType() const { return m_type; }

//private:
   eModelAnimationType m_type;
   vector< vector<sModelKeyFrame> > m_keyFrameVectors;
};

////////////////////////////////////////

cModelAnimation::cModelAnimation(eModelAnimationType type,
                                 const vector< vector<sModelKeyFrame> > & keyFrameVectors)
 : m_type(type)
 , m_keyFrameVectors(keyFrameVectors)
{
}

////////////////////////////////////////

template <>
class cReadWriteOps<cModelAnimation>
{
public:
   static tResult Write(IWriter * pWriter, const cModelAnimation & modelAnim);
};

////////////////////////////////////////

tResult cReadWriteOps<cModelAnimation>::Write(IWriter * pWriter, const cModelAnimation & modelAnim)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (pWriter->Write(static_cast<int>(modelAnim.m_type)) == S_OK
      && pWriter->Write(modelAnim.m_keyFrameVectors) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////

static tResult ConvertModel(const cFileSpec & ms3dModelName, const cFileSpec & outputModelName)
{
   tResult result = E_FAIL;

   cAutoIPtr<IReader> pReader;
   result = FileReaderCreate(ms3dModelName, kFileModeBinary, &pReader);
   if (result != S_OK)
   {
      return result;
   }

   cMs3dModel ms3dModel;
   result = ms3dModel.Read(pReader);
   if (result != S_OK)
   {
      return result;
   }

   vector<sModelVertex> vertices;
   vector<sModelMesh> meshes;
   vector<uint16> indices;
   CompileMeshes(
      ms3dModel.GetVertices(), ms3dModel.GetTriangles(), ms3dModel.GetGroups(),
      &vertices, &meshes, &indices);

   vector<sModelMaterial> materials;
   CompileMaterials(ms3dModel.GetMaterials(), &materials);

   vector<sModelJoint> joints;
   vector< vector<sModelKeyFrame> > keyFrames;
   CompileJointsAndKeyFrames(
      ms3dModel.GetAnimationFPS(), ms3dModel.GetJoints(),
      &joints, &keyFrames);

   vector<sModelAnimationDesc> animDescs;
   ParseAnimDescs(ms3dModel.GetModelComment().c_str(), &animDescs);

   const vector<cMs3dJoint> & ms3dJoints = ms3dModel.GetJoints();

   vector<cModelAnimation> modelAnimations;

   vector<sModelAnimationDesc>::iterator iter = animDescs.begin(), end = animDescs.end();
   for (; iter != end; ++iter)
   {
      vector< vector<sModelKeyFrame> > animKeyFrames(joints.size());

      vector<cMs3dJoint>::const_iterator iter2 = ms3dJoints.begin(), end2 = ms3dJoints.end();
      for (; iter2 != end2; ++iter2)
      {
         const vector<sMs3dRotationKeyframe> & rotKeys = iter2->GetRotationKeys();
         const vector<sMs3dPositionKeyframe> & posKeys = iter2->GetPositionKeys();

         if (rotKeys.size() != posKeys.size())
         {
            continue;
         }

         vector<sModelKeyFrame> & jointAnimKeyFrames = animKeyFrames[iter2 - ms3dJoints.begin()];
         jointAnimKeyFrames.reserve(iter->end - iter->start + 1);

         int iStart = -1, iEnd = -1;
         for (uint j = 0; j < rotKeys.size(); ++j)
         {
            const sMs3dRotationKeyframe & rotKey = rotKeys[j];
            const sMs3dPositionKeyframe & posKey = posKeys[j];
            uint rotFrame = FloatToInt(rotKey.time);
            uint posFrame = FloatToInt(posKey.time);
            Assert(rotFrame == posFrame);
            if (rotFrame == iter->start)
            {
               iStart = j;
            }
            if (rotFrame >= iter->start)
            {
               sModelKeyFrame keyFrame;
               keyFrame.time = rotKey.time / ms3dModel.GetAnimationFPS();
               keyFrame.translation = tVec3(posKey.position);
               keyFrame.rotation = QuatFromEulerAngles(tVec3(rotKey.rotation));
               jointAnimKeyFrames.push_back(keyFrame);
            }
            if (rotFrame >= iter->end)
            {
               iEnd = j;
               break;
            }
         }
      }

      modelAnimations.push_back(cModelAnimation(iter->type, animKeyFrames));
   }

   cAutoIPtr<IWriter> pWriter;
   result = FileWriterCreate(outputModelName, kFileModeBinary, &pWriter);
   if (result != S_OK)
   {
      return result;
   }

   typedef cModelChunk< vector<sModelJoint> > tJointArrayChunk;
   cModelChunk<tJointArrayChunk> skeletonChunk(MODEL_SKELETON_CHUNK,
      tJointArrayChunk(MODEL_JOINT_ARRAY_CHUNK, joints));

   if (pWriter->Write(cModelChunk<NoChunkData>(MODEL_FILE_ID_CHUNK)) == S_OK
      && pWriter->Write(cModelChunk<uint>(MODEL_VERSION_CHUNK, 1)) == S_OK
      && pWriter->Write(cModelChunk< vector<sModelVertex> >(MODEL_VERTEX_ARRAY_CHUNK, vertices)) == S_OK
      && pWriter->Write(cModelChunk< vector<uint16> >(MODEL_INDEX16_ARRAY_CHUNK, indices)) == S_OK
      && pWriter->Write(cModelChunk< vector<sModelMesh> >(MODEL_MESH_ARRAY_CHUNK, meshes)) == S_OK
      && pWriter->Write(cModelChunk< vector<sModelMaterial> >(MODEL_MATERIAL_ARRAY_CHUNK, materials)) == S_OK
      && pWriter->Write(skeletonChunk) == S_OK)
   {
      result = S_OK;

      vector<cModelAnimation>::const_iterator iter = modelAnimations.begin(), end = modelAnimations.end();
      for (; iter != end; ++iter)
      {
         result = pWriter->Write(cModelChunk<cModelAnimation>(MODEL_ANIMATION_SEQUENCE_CHUNK, *iter));
      }
   }

   return result;
}

///////////////////////////////////////////////////////////////////////////////

int _tmain(int argc, tChar * argv[])
{
   int result = EXIT_FAILURE;

   if (!MainInit(argc, argv))
   {
      MainTerm();
      return result;
   }

   cStr inputModel;
   if (ConfigGet(_T("in"), &inputModel) == S_OK)
   {
      cFileSpec outputModel(inputModel.c_str());
      outputModel.SetFileExt(_T("sgem"));

      if (ConvertModel(cFileSpec(inputModel.c_str()), outputModel) == S_OK)
      {
         result = EXIT_SUCCESS;
      }
   }

   MainTerm();

   return result;
}

///////////////////////////////////////////////////////////////////////////////
