///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ms3dmodel/ms3dmodel.h"

#include "engine/engineapi.h"
#include "engine/modelapi.h"
#include "engine/modelreadwrite.h"

#include "tech/configapi.h"
#include "tech/dictionaryapi.h"
#include "tech/filepath.h"
#include "tech/filespec.h"
#include "tech/globalobj.h"
#include "tech/quat.inl"
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
   cModelAnimation();
   cModelAnimation(eModelAnimationType type, const vector< vector<sModelKeyFrame> > & keyFrameVectors);

   eModelAnimationType GetAnimationType() const { return m_type; }

//private:
   eModelAnimationType m_type;
   vector< vector<sModelKeyFrame> > m_keyFrameVectors;
};

////////////////////////////////////////

cModelAnimation::cModelAnimation()
{
}

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

static tResult ExtractAnimation(const sModelAnimationDesc & animDesc,
                                float animFps,
                                const vector< vector<sModelKeyFrame> > & keyFrames,
                                cModelAnimation * pAnim)
{
   if (pAnim == NULL)
   {
      return E_POINTER;
   }

   pAnim->m_type = animDesc.type;

   vector< vector<sModelKeyFrame> > & animKeyFrames = pAnim->m_keyFrameVectors;
   animKeyFrames.resize(keyFrames.size());

   vector< vector<sModelKeyFrame> >::const_iterator iter2 = keyFrames.begin(), end2 = keyFrames.end();
   for (; iter2 != end2; ++iter2)
   {
      const vector<sModelKeyFrame> & jointKeyFrames = *iter2;

      vector<sModelKeyFrame> & jointAnimKeyFrames = animKeyFrames[iter2 - keyFrames.begin()];

      int iStart = -1, iEnd = -1;
      for (uint j = 0; j < jointKeyFrames.size(); ++j)
      {
         const sModelKeyFrame & keyFrame = jointKeyFrames[j];
         uint frame = FloatToInt(keyFrame.time * animFps);
         if (frame == animDesc.start)
         {
            iStart = j;
         }
         if (frame >= animDesc.start)
         {
            jointAnimKeyFrames.push_back(keyFrame);
         }
         if (frame >= animDesc.end)
         {
            iEnd = j;
            break;
         }
      }
   }

   return S_OK;
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

   Assert(joints.size() == keyFrames.size());

   vector<sModelAnimationDesc> animDescs;
   ParseAnimDescs(ms3dModel.GetModelComment().c_str(), &animDescs);

   const vector<cMs3dJoint> & ms3dJoints = ms3dModel.GetJoints();

   vector<cModelAnimation> modelAnimations(animDescs.size());

   vector<sModelAnimationDesc>::iterator iter = animDescs.begin(), end = animDescs.end();
   for (; iter != end; ++iter)
   {
      cModelAnimation & modelAnimation = modelAnimations[iter - animDescs.begin()];
      ExtractAnimation(*iter, ms3dModel.GetAnimationFPS(), keyFrames, &modelAnimation);
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
         cModelChunk<cModelAnimation> animChunk(MODEL_ANIMATION_SEQUENCE_CHUNK, *iter);
         result = pWriter->Write(animChunk);
         if (result != S_OK)
         {
            break;
         }
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
         cFilePath outputPath;
         outputModel.GetPath(&outputPath);

         UseGlobal(ResourceManager);
         pResourceManager->AddDirectory(outputPath.CStr());

         IModel * pModel = NULL;
         pResourceManager->Load(outputModel.GetFileName(), kRT_Model, NULL, reinterpret_cast<void**>(&pModel));

         result = EXIT_SUCCESS;
      }
   }

   MainTerm();

   return result;
}

///////////////////////////////////////////////////////////////////////////////
