///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MODELTYPES_H
#define INCLUDED_MODELTYPES_H

#include "enginedll.h"

#include "tech/quat.h"
#include "tech/vec3.h"

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// STRUCT: sModelVertex
//
// A vertex with a single influencing bone (and no weights)

struct sModelVertex
{
   tVec3::value_type u, v;
   tVec3 normal;
   tVec3 pos;
   float bone; // TODO: call these bones or joints?
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


///////////////////////////////////////////////////////////////////////////////
//
// STRUCT: sModelMesh
//

struct sModelMesh
{
   int primitive;
   int materialIndex;
   uint indexStart;
   uint nIndices;
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


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MODELTYPES_H
