///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MS3DREAD_H
#define INCLUDED_MS3DREAD_H

#include "ms3d.h"

#include "readwriteapi.h"
#include "vec3.h"

#include <map>
#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IReader);

struct sMs3dVertex
{
   tVec3::value_type u, v;
   tVec3 normal;
   tVec3 pos;
   float bone;
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dVertexMapper
//

class cMs3dVertexMapper
{
public:
   cMs3dVertexMapper(const ms3d_vertex_t * pVertices, size_t nVertices);
   cMs3dVertexMapper(const std::vector<ms3d_vertex_t> & vertices);

   uint MapVertex(uint originalIndex, const float normal[3], float s, float t);

   const void * GetVertexData() const;
   uint GetVertexCount() const;

private:
   uint m_nOriginalVertices;
   std::vector<sMs3dVertex> m_vertices;
   std::vector<bool> m_haveVertex;
   std::map<uint, uint> m_remap;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dGroup
//

class cMs3dGroup
{
   friend class cReadWriteOps<cMs3dGroup>;

public:
   cMs3dGroup();
   cMs3dGroup(const cMs3dGroup & other);
   ~cMs3dGroup();

   const cMs3dGroup & operator =(const cMs3dGroup & other);

   byte GetFlags() const;
   const char * GetName() const;
   size_t GetNumTriangles() const;
   uint GetTriangle(int index) const;
   const std::vector<uint16> & GetTriangles() const;
   uint8 GetMaterialIndex() const;

private:
   byte flags;
   char name[32];
   std::vector<uint16> triangleIndices;
   uint8 materialIndex;
};

///////////////////////////////////////

inline byte cMs3dGroup::GetFlags() const
{
   return flags;
}

///////////////////////////////////////

inline const char * cMs3dGroup::GetName() const
{
   return name;
}

///////////////////////////////////////

inline size_t cMs3dGroup::GetNumTriangles() const
{
   return triangleIndices.size();
}

///////////////////////////////////////

inline uint cMs3dGroup::GetTriangle(int index) const
{
   return triangleIndices[index];
}

///////////////////////////////////////

inline const std::vector<uint16> & cMs3dGroup::GetTriangles() const
{
   return triangleIndices;
}

///////////////////////////////////////

inline uint8 cMs3dGroup::GetMaterialIndex() const
{
   return materialIndex;
}

///////////////////////////////////////

template <>
class cReadWriteOps<cMs3dGroup>
{
public:
   static tResult Read(IReader * pReader, cMs3dGroup * pGroup);
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dJoint
//

class cMs3dJoint
{
   friend class cReadWriteOps<cMs3dJoint>;

   void operator =(const cMs3dJoint &);

public:
   cMs3dJoint();
   cMs3dJoint(const cMs3dJoint & other);
   ~cMs3dJoint();

   byte GetFlags() const;
   const char * GetName() const;
   const char * GetParentName() const;
   const float * GetRotation() const;
   const float * GetPosition() const;
   const std::vector<ms3d_keyframe_rot_t> & GetKeyFramesRot() const;
   const std::vector<ms3d_keyframe_pos_t> & GetKeyFramesTrans() const;

private:
   byte flags;
   char name[32];
   char parentName[32];
   float rotation[3];
   float position[3];
   std::vector<ms3d_keyframe_rot_t> keyFramesRot;
   std::vector<ms3d_keyframe_pos_t> keyFramesTrans;
};

///////////////////////////////////////

inline byte cMs3dJoint::GetFlags() const
{
   return flags;
}

///////////////////////////////////////

inline const char * cMs3dJoint::GetName() const
{
   return name;
}

///////////////////////////////////////

inline const char * cMs3dJoint::GetParentName() const
{
   return parentName;
}

///////////////////////////////////////

inline const float * cMs3dJoint::GetRotation() const
{
   return rotation;
}

///////////////////////////////////////

inline const float * cMs3dJoint::GetPosition() const
{
   return position;
}

///////////////////////////////////////

inline const std::vector<ms3d_keyframe_rot_t> & cMs3dJoint::GetKeyFramesRot() const
{
   return keyFramesRot;
}

///////////////////////////////////////

inline const std::vector<ms3d_keyframe_pos_t> & cMs3dJoint::GetKeyFramesTrans() const
{
   return keyFramesTrans;
}

///////////////////////////////////////

template <>
class cReadWriteOps<cMs3dJoint>
{
public:
   static tResult Read(IReader * pReader, cMs3dJoint * pJoint);
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MS3DREAD_H
