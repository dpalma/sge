///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "engine/modelreadwrite.h"

#include "tech/readwriteutils.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include <vector>

#include "tech/dbgalloc.h" // must be last header

using namespace std;

///////////////////////////////////////////////////////////////////////////////

tResult cReadWriteOps<sModelVertex>::Read(IReader * pReader, sModelVertex * pModelVertex)
{
   if (pReader == NULL || pModelVertex == NULL)
   {
      return E_POINTER;
   }

   if (pReader->Read(&pModelVertex->u) == S_OK
      && pReader->Read(&pModelVertex->v) == S_OK
      && pReader->Read(&pModelVertex->normal) == S_OK
      && pReader->Read(&pModelVertex->pos) == S_OK
      && pReader->Read(&pModelVertex->bone) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}

tResult cReadWriteOps<sModelVertex>::Write(IWriter * pWriter, const sModelVertex & modelVertex)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (pWriter->Write(modelVertex.u) == S_OK
      && pWriter->Write(modelVertex.v) == S_OK
      && pWriter->Write(modelVertex.normal) == S_OK
      && pWriter->Write(modelVertex.pos) == S_OK
      && pWriter->Write(modelVertex.bone) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////

tResult cReadWriteOps<sModelMaterial>::Read(IReader * pReader, sModelMaterial * pModelMaterial)
{
   if (pReader == NULL || pModelMaterial == NULL)
   {
      return E_POINTER;
   }

   memset(pModelMaterial->szTexture, 0, sizeof(pModelMaterial->szTexture));

   if (pReader->Read(pModelMaterial->diffuse, sizeof(pModelMaterial->diffuse)) == S_OK
      && pReader->Read(pModelMaterial->ambient, sizeof(pModelMaterial->ambient)) == S_OK
      && pReader->Read(pModelMaterial->specular, sizeof(pModelMaterial->specular)) == S_OK
      && pReader->Read(pModelMaterial->emissive, sizeof(pModelMaterial->emissive)) == S_OK
      && pReader->Read(&pModelMaterial->shininess) == S_OK
      && pReader->Read(pModelMaterial->szTexture, sizeof(pModelMaterial->szTexture)) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}

tResult cReadWriteOps<sModelMaterial>::Write(IWriter * pWriter, const sModelMaterial & modelMaterial)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (pWriter->Write(static_cast<const void *>(modelMaterial.diffuse), sizeof(modelMaterial.diffuse)) == S_OK
      && pWriter->Write(modelMaterial.ambient, sizeof(modelMaterial.ambient)) == S_OK
      && pWriter->Write(modelMaterial.specular, sizeof(modelMaterial.specular)) == S_OK
      && pWriter->Write(modelMaterial.emissive, sizeof(modelMaterial.emissive)) == S_OK
      && pWriter->Write(modelMaterial.shininess) == S_OK
      && pWriter->Write(modelMaterial.szTexture, sizeof(modelMaterial.szTexture)) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////

tResult cReadWriteOps<sModelMesh>::Read(IReader * pReader, sModelMesh * pModelMesh)
{
   if (pReader == NULL || pModelMesh == NULL)
   {
      return E_POINTER;
   }

   if (pReader->Read(&pModelMesh->primitive) == S_OK
      && pReader->Read(&pModelMesh->materialIndex) == S_OK
      && pReader->Read(&pModelMesh->indexStart) == S_OK
      && pReader->Read(&pModelMesh->nIndices) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}

tResult cReadWriteOps<sModelMesh>::Write(IWriter * pWriter, const sModelMesh & modelMesh)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (pWriter->Write(modelMesh.primitive) == S_OK
      && pWriter->Write(modelMesh.materialIndex) == S_OK
      && pWriter->Write(modelMesh.indexStart) == S_OK
      && pWriter->Write(modelMesh.nIndices) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////

tResult cReadWriteOps<sModelJoint>::Read(IReader * pReader, sModelJoint * pModelJoint)
{
   if (pReader == NULL || pModelJoint == NULL)
   {
      return E_POINTER;
   }

   if (pReader->Read(&pModelJoint->parentIndex) == S_OK
      && pReader->Read(&pModelJoint->localTranslation) == S_OK
      && pReader->Read(&pModelJoint->localRotation) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}

tResult cReadWriteOps<sModelJoint>::Write(IWriter * pWriter, const sModelJoint & modelJoint)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (pWriter->Write(modelJoint.parentIndex) == S_OK
      && pWriter->Write(modelJoint.localTranslation) == S_OK
      && pWriter->Write(modelJoint.localRotation) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////

tResult cReadWriteOps<sModelKeyFrame>::Read(IReader * pReader, sModelKeyFrame * pModelKeyFrame)
{
   if (pReader == NULL || pModelKeyFrame == NULL)
   {
      return E_POINTER;
   }

   if (pReader->Read(&pModelKeyFrame->time) == S_OK
      && pReader->Read(&pModelKeyFrame->translation) == S_OK
      && pReader->Read(&pModelKeyFrame->rotation) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}

tResult cReadWriteOps<sModelKeyFrame>::Write(IWriter * pWriter, const sModelKeyFrame & modelKeyFrame)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (pWriter->Write(modelKeyFrame.time) == S_OK
      && pWriter->Write(modelKeyFrame.translation) == S_OK
      && pWriter->Write(modelKeyFrame.rotation) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

static bool operator ==(const sModelKeyFrame & keyFrame1, const sModelKeyFrame & keyFrame2)
{
   return keyFrame1.time == keyFrame2.time
      && keyFrame1.translation.x == keyFrame2.translation.x
      && keyFrame1.translation.y == keyFrame2.translation.y
      && keyFrame1.translation.z == keyFrame2.translation.z
      && keyFrame1.rotation.x == keyFrame2.rotation.x
      && keyFrame1.rotation.y == keyFrame2.rotation.y
      && keyFrame1.rotation.z == keyFrame2.rotation.z
      && keyFrame1.rotation.w == keyFrame2.rotation.w;
}

SUITE(ModelReadWrite)
{
   TEST(ReadWriteArrayOfArraysOfKeyFrames)
   {
      static const uint nAnims = 10;
      static const uint nKeyFrames = 20;

      vector< vector<sModelKeyFrame> > keyFrameVectors;
      keyFrameVectors.resize(nAnims);

      vector< vector<sModelKeyFrame> >::iterator iter = keyFrameVectors.begin(), end = keyFrameVectors.end();
      for (; iter != end; ++iter)
      {
         iter->resize(nKeyFrames);
         vector<sModelKeyFrame>::iterator iter2 = iter->begin(), end2 = iter->end();
         for (; iter2 != end2; ++iter2)
         {
            float index = static_cast<float>(iter2 - iter->begin());
            iter2->rotation = tQuat(0, 1, 0, index);
            iter2->translation = tVec3(0, 0, index);
            iter2->time = index;
         }
      }

      byte mem[sizeof(sModelKeyFrame) * nKeyFrames * nAnims + 64];

      {
         cAutoIPtr<IWriter> pWriter;
         CHECK_EQUAL(S_OK, MemWriterCreate(&mem[0], sizeof(mem), &pWriter));
         CHECK_EQUAL(S_OK, pWriter->Write(keyFrameVectors));
      }

      {
         cAutoIPtr<IReader> pReader;
         CHECK_EQUAL(S_OK, MemReaderCreate(&mem[0], sizeof(mem), false, &pReader));

         vector< vector<sModelKeyFrame> > readKeyFrameVectors;
         CHECK_EQUAL(S_OK, pReader->Read(&readKeyFrameVectors));

         CHECK(keyFrameVectors == readKeyFrameVectors);
      }
   }
}

#endif // HAVE_UNITTESTPP

///////////////////////////////////////////////////////////////////////////////
