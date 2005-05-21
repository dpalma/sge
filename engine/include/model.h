///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MODEL_H
#define INCLUDED_MODEL_H

#include "enginedll.h"
#include "comtools.h"

#include "vec3.h"
#include "matrix4.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IReader);

///////////////////////////////////////////////////////////////////////////////

struct sModelVertex
{
   tVec3::value_type u, v;
   tVec3 normal;
   tVec3 pos;
   float bone;
};


#if _MSC_VER >= 1300
template class ENGINE_API cMatrix4<float>;
template class ENGINE_API std::allocator< cMatrix4<float> >;
template class ENGINE_API std::vector< cMatrix4<float> >;
template class ENGINE_API std::allocator<sModelVertex>;
template class ENGINE_API std::vector<sModelVertex>;
#else
#pragma warning(push)
#pragma warning(disable:4251)
#endif

typedef std::vector< cMatrix4<float> > tMatrices;
typedef std::vector<sModelVertex> tModelVertices;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModel
//

#define kRT_Model "Model" // resource type

class ENGINE_API cModel
{
   cModel(const cModel &);
   const cModel & operator =(const cModel &);

public:
   cModel();
   virtual ~cModel();

   void Animate(double elapsedTime);

   void Render();

   static tResult RegisterResourceFormat();

private:

   tResult PostRead();

   static void * ModelLoadMs3d(IReader * pReader);

   static void ModelUnload(void * pData);


   std::vector<sModelVertex> m_vertices;

   float m_animationTime;
   std::vector< cMatrix4<float> > m_boneMatrices;
};


#if _MSC_VER < 1300
#pragma warning(pop)
#endif


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MODEL_H
