///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MODELTYPES_H
#define INCLUDED_MODELTYPES_H

#include "enginedll.h"

#include "quat.h"
#include "vec3.h"
#include "readwriteapi.h"

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// STRUCT: sModelVertex
//

struct sModelVertex
{
   tVec3::value_type u, v;
   tVec3 normal;
   tVec3 pos;
   float bone; // TODO: call these bones or joints?
};

template <>
class ENGINE_API cReadWriteOps<sModelVertex>
{
public:
   static tResult Read(IReader * pReader, sModelVertex * pModelVertex);
   static tResult Write(IWriter * pWriter, const sModelVertex & modelVertex);
};


///////////////////////////////////////////////////////////////////////////////
//
// STRUCT: sBlendedVertex
//
// For software vertex blending: bone indices not needed after blending

struct sBlendedVertex
{
   tVec3::value_type u, v;
   tVec3 normal;
   tVec3 pos;
};


///////////////////////////////////////////////////////////////////////////////
//
// STRUCT: sModelMaterial
//

#define kMaxMaterialTextureSize 32

struct sModelMaterial
{
   float diffuse[4];
   float ambient[4];
   float specular[4];
   float emissive[4];
   float shininess;
   char szTexture[kMaxMaterialTextureSize];
};

template <>
class ENGINE_API cReadWriteOps<sModelMaterial>
{
public:
   static tResult Read(IReader * pReader, sModelMaterial * pModelMaterial);
   static tResult Write(IWriter * pWriter, const sModelMaterial & modelMaterial);
};


///////////////////////////////////////////////////////////////////////////////
//
// STRUCT: sModelJoint
//

struct sModelJoint
{
   int parentIndex;
   tVec3 localTranslation;
   tQuat localRotation;
};

template <>
class ENGINE_API cReadWriteOps<sModelJoint>
{
public:
   static tResult Read(IReader * pReader, sModelJoint * pModelJoint);
   static tResult Write(IWriter * pWriter, const sModelJoint & modelJoint);
};


///////////////////////////////////////////////////////////////////////////////
//
// STRUCT: sModelKeyFrame
//

struct sModelKeyFrame
{
   double time;
   tVec3 translation;
   tQuat rotation;
};

template <>
class ENGINE_API cReadWriteOps<sModelKeyFrame>
{
public:
   static tResult Read(IReader * pReader, sModelKeyFrame * pModelKeyFrame);
   static tResult Write(IWriter * pWriter, const sModelKeyFrame & modelKeyFrame);
};


///////////////////////////////////////////////////////////////////////////////

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
   ((uint)(byte)(ch0) | ((uint)(byte)(ch1) << 8) | \
   ((uint)(byte)(ch2) << 16) | ((uint)(byte)(ch3) << 24 ))
#endif

#define MODEL_FILE_ID_CHUNK                  MAKEFOURCC('S','G','M','D')
#define MODEL_VERSION_CHUNK                  MAKEFOURCC('V','R','S','N')
#define MODEL_VERTEX_ARRAY_CHUNK             MAKEFOURCC('V','R','T','A')
#define MODEL_MESH_ARRAY_CHUNK               MAKEFOURCC('M','S','H','A')
#define MODEL_MATERIAL_ARRAY_CHUNK           MAKEFOURCC('M','T','L','A')
#define MODEL_SKELETON_CHUNK                 MAKEFOURCC('S','K','E','L')
#define MODEL_ANIMATION_SEQUENCE_CHUNK       MAKEFOURCC('A','N','I','M')

const uint kModelChunkHeaderSize = (2 * sizeof(uint)); // chunk type and length


////////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cModelChunk
//

typedef void * NoChunkData;

template <typename T>
class cModelChunk
{
   friend class cReadWriteOps< cModelChunk<T> >;

public:
   cModelChunk();
   cModelChunk(uint chunkId);
   cModelChunk(uint chunkId, const T & chunkData);

   uint GetChunkId() const { return m_chunkId; }
   uint GetChunkLength() const { return m_chunkLength; }
   bool NoChunkData() const { return m_bNoChunkData; }
   const T & GetChunkData() const { return m_chunkData; }

private:
   uint m_chunkId, m_chunkLength;
   bool m_bNoChunkData;
   T m_chunkData;
};

////////////////////////////////////////

template <typename T>
cModelChunk<T>::cModelChunk()
 : m_chunkId(0)
 , m_bNoChunkData(true)
 , m_chunkLength(0)
{
}

////////////////////////////////////////

template <typename T>
cModelChunk<T>::cModelChunk(uint chunkId)
 : m_chunkId(chunkId)
 , m_bNoChunkData(true)
 , m_chunkLength(0)
{
}

////////////////////////////////////////

template <typename T>
cModelChunk<T>::cModelChunk(uint chunkId, const T & chunkData)
 : m_chunkId(chunkId)
 , m_chunkLength(0)
 , m_bNoChunkData(false)
 , m_chunkData(chunkData)
{
}


////////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cReadWriteOps< cModelChunk<T> >
//

template <typename T>
class cReadWriteOps< cModelChunk<T> >
{
public:
   static tResult Read(IReader * pReader, cModelChunk<T> * pModelChunk);
   static tResult Write(IWriter * pWriter, const cModelChunk<T> & modelChunk);
};

////////////////////////////////////////

template <typename T>
tResult cReadWriteOps< cModelChunk<T> >::Read(IReader * pReader, cModelChunk<T> * pModelChunk)
{
   if (pReader == NULL || pModelChunk == NULL)
   {
      return E_POINTER;
   }

   tResult result = E_FAIL;

   if (pReader->Read(&pModelChunk->m_chunkId) == S_OK
      && pReader->Read(&pModelChunk->m_chunkLength) == S_OK)
   {
      if (pModelChunk->GetChunkLength() == kModelChunkHeaderSize)
      {
         pModelChunk->m_bNoChunkData = true;
         result = S_OK;
      }
      else if (pReader->Read(&pModelChunk->m_chunkData) == S_OK)
      {
         pModelChunk->m_bNoChunkData = false;
         result = S_OK;
      }
   }

   return result;
}

////////////////////////////////////////

template <typename T>
tResult cReadWriteOps< cModelChunk<T> >::Write(IWriter * pWriter, const cModelChunk<T> & modelChunk)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   tResult result = E_FAIL;

   ulong start = 0, end = 0;
   if (pWriter->Tell(&start) == S_OK                                 // get the chunk's start position
      && pWriter->Write(modelChunk.GetChunkId()) == S_OK             // write the chunk identifier
      && pWriter->Write(static_cast<uint>(0)) == S_OK                // zero will be replaced by chunk size
      && (modelChunk.NoChunkData()                                   // possibly no data, or
         || pWriter->Write(modelChunk.GetChunkData()) == S_OK)       // write the chunk data
      && pWriter->Tell(&end) == S_OK                                 // get the file position after the data
      && pWriter->Seek(start + sizeof(uint), kSO_Set) == S_OK        // seek back to the chunk size (the zero)
      && pWriter->Write(static_cast<uint>(end - start)) == S_OK      // replace the zero with the chunk length
      && pWriter->Seek(end, kSO_Set) == S_OK)                        // seek back to the end of the chunk
   {
      result = S_OK;
   }

   return result;
}


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MODELTYPES_H
