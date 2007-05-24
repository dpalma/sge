///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MS3DMODEL_H
#define INCLUDED_MS3DMODEL_H

#include "ms3dmodeldll.h"

#include "ms3djoint.h"
#include "ms3dmaterial.h"

#include "tech/comtools.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

struct sModelVertex;

F_DECLARE_INTERFACE(IModel);
F_DECLARE_INTERFACE(IReader);

template class MS3DMODEL_API std::allocator<cMs3dMaterial>;
template class MS3DMODEL_API std::allocator<cMs3dJoint>;

template class MS3DMODEL_API std::vector<cMs3dMaterial>;
template class MS3DMODEL_API std::vector<cMs3dJoint>;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dModel
//

class MS3DMODEL_API cMs3dModel
{
   cMs3dModel(const cMs3dModel &);
   const cMs3dModel & operator =(const cMs3dModel &);

public:
   cMs3dModel();
   ~cMs3dModel();

   IModel * Read(IReader * pReader);

   static void * Load(IReader * pReader);
   static void Unload(void * pData);

private:
   std::vector<cMs3dMaterial> ms3dMaterials;
   float m_animationFPS;
   float m_currentTime;
   int m_nTotalFrames;
   std::vector<cMs3dJoint> ms3dJoints;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MS3DMODEL_H
