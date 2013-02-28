///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MS3DVERTEX_H
#define INCLUDED_MS3DVERTEX_H

#include "ms3dmodeldll.h"

#include "tech/readwriteapi.h"

#ifdef _MSC_VER
#pragma once
#endif

struct sModelVertex;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dVertex
//

class MS3DMODEL_API cMs3dVertex
{
   friend class cReadWriteOps<cMs3dVertex>;

public:
   cMs3dVertex();
   cMs3dVertex(float x, float y, float z, int8 boneId);
   cMs3dVertex(float x, float y, float z, float s, float t, int8 boneId);
   cMs3dVertex(const cMs3dVertex & other);
   ~cMs3dVertex();

   const cMs3dVertex & operator =(const cMs3dVertex &);

   void ToModelVertex(sModelVertex * pModelVertex) const;

   const float * GetPosition() const;
   int8 GetBone() const;
   const float * cMs3dVertex::GetNormal() const;
   float GetS() const;
   float GetT() const;

   enum ValidFields
   {
      None = 0,
      Position = 1,
      Normal = 2,
      BoneId = 4,
      TexCoords = 8,
   };

   bool HasPosition() const;
   bool HasNormal() const;
   bool HasBoneId() const;
   bool HasTexCoords() const;

private:
   float m_vertex[3];
   int8 m_boneId;
   float m_normal[3];
   float m_s, m_t;
   uint m_validFields;
};

///////////////////////////////////////

inline const float * cMs3dVertex::GetPosition() const
{
   return m_vertex;
}

///////////////////////////////////////

inline int8 cMs3dVertex::GetBone() const
{
   return m_boneId;
}

///////////////////////////////////////

inline const float * cMs3dVertex::GetNormal() const
{
   return m_normal;
}

///////////////////////////////////////

inline float cMs3dVertex::GetS() const
{
   return m_s;
}

///////////////////////////////////////

inline float cMs3dVertex::GetT() const
{
   return m_t;
}

///////////////////////////////////////

inline bool cMs3dVertex::HasPosition() const
{
   return ((m_validFields & Position) != 0);
}

///////////////////////////////////////

inline bool cMs3dVertex::HasNormal() const
{
   return ((m_validFields & Normal) != 0);
}

///////////////////////////////////////

inline bool cMs3dVertex::HasBoneId() const
{
   return ((m_validFields & BoneId) != 0);
}

///////////////////////////////////////

inline bool cMs3dVertex::HasTexCoords() const
{
   return ((m_validFields & TexCoords) != 0);
}

///////////////////////////////////////

template <>
class MS3DMODEL_API cReadWriteOps<cMs3dVertex>
{
public:
   static tResult Read(IReader * pReader, cMs3dVertex * pVertex);
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MS3DVERTEX_H
