///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCENEMESH_H
#define INCLUDED_SCENEMESH_H

#include "boundingvolume.h"
#include "scenegroup.h"
#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IMesh);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneMesh
//

class cSceneMesh : public cSceneTransformGroup
{
   cSceneMesh(const cSceneMesh &);
   const cSceneMesh & operator =(const cSceneMesh &);

public:
   cSceneMesh();
   virtual ~cSceneMesh();

   bool SetMesh(const char * pszMesh);
   IMesh * AccessMesh();

   virtual void Render();

   virtual const cBoundingVolume * GetBoundingVolume() const;

private:
   cAutoIPtr<IMesh> m_pMesh;
   tVec3 m_centroid;
   mutable cBoundingSphere m_bounds;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCENEMESH_H
