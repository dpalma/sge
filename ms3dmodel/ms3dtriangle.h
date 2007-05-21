///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MS3DTRIANGLE_H
#define INCLUDED_MS3DTRIANGLE_H

#include "tech/readwriteapi.h"

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dTriangle
//

class cMs3dTriangle
{
   friend class cReadWriteOps<cMs3dTriangle>;

public:
   cMs3dTriangle();
   cMs3dTriangle(const cMs3dTriangle & other);
   ~cMs3dTriangle();

   const cMs3dTriangle & operator =(const cMs3dTriangle &);

   enum Index { Zero = 0, One = 1, Two = 2 };

   uint16 GetVertexIndex(Index index) const;
   const float * GetVertexNormal(Index index) const;
   float GetS(Index index) const;
   float GetT(Index index) const;

private:
   uint16 m_flags;
   uint16 m_vertexIndices[3];
   float m_vertexNormals[3][3];
   float m_s[3];
   float m_t[3];
   byte m_smoothingGroup;
   byte m_groupIndex;
};

///////////////////////////////////////

inline uint16 cMs3dTriangle::GetVertexIndex(Index index) const
{
   return m_vertexIndices[index];
}

///////////////////////////////////////

inline const float * cMs3dTriangle::GetVertexNormal(Index index) const
{
   return m_vertexNormals[index];
}

///////////////////////////////////////

inline float cMs3dTriangle::GetS(Index index) const
{
   return m_s[index];
}

///////////////////////////////////////

inline float cMs3dTriangle::GetT(Index index) const
{
   return m_t[index];
}

///////////////////////////////////////

template <>
class cReadWriteOps<cMs3dTriangle>
{
public:
   static tResult Read(IReader * pReader, cMs3dTriangle * pVertex);
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MS3DTRIANGLE_H
