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

#endif // !INCLUDED_MODELTYPES_H
