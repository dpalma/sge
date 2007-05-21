///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MS3DVERTEX_H
#define INCLUDED_MS3DVERTEX_H

#include "tech/readwriteapi.h"

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dVertex
//

class cMs3dVertex
{
   friend class cReadWriteOps<cMs3dVertex>;

public:
   cMs3dVertex();
   cMs3dVertex(const cMs3dVertex & other);
   ~cMs3dVertex();

   const cMs3dVertex & operator =(const cMs3dVertex &);

   enum Flags
   {
      None           = 0,
      Selected       = 1,
      Hidden         = 2,
      Selected2      = 4,
      Dirty          = 8,
   };

   const float * GetPosition() const;
   int8 GetBone() const;

private:
   byte m_flags;
   float m_vertex[3];
   int8 m_boneId;
   byte m_referenceCount;
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

template <>
class cReadWriteOps<cMs3dVertex>
{
public:
   static tResult Read(IReader * pReader, cMs3dVertex * pVertex);
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MS3DVERTEX_H
