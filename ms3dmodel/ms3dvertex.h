///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MS3DVERTEX_H
#define INCLUDED_MS3DVERTEX_H

#include "ms3d.h"

#include "tech/readwriteapi.h"
#include "tech/vec3.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif


struct sMs3dVertex
{
   tVec3::value_type u, v;
   tVec3 normal;
   tVec3 pos;
   float bone;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dVertex
//

class cMs3dVertex
{
   friend class cReadWriteOps<cMs3dVertex>;

   void operator =(const cMs3dVertex &);

public:
   cMs3dVertex();
   cMs3dVertex(const cMs3dVertex & other);
   ~cMs3dVertex();

private:
};

///////////////////////////////////////

template <>
class cReadWriteOps<cMs3dVertex>
{
public:
   static tResult Read(IReader * pReader, cMs3dVertex * pVertex);
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MS3DVERTEX_H
