///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MESH_H
#define INCLUDED_MESH_H

#include "combase.h"

#ifdef _MSC_VER
#pragma once
#endif

// forward declarations
template <typename T> class cVec3;
typedef class cVec3<float> tVec3;
F_DECLARE_INTERFACE(IResourceManager);

F_DECLARE_INTERFACE(IMesh);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IMesh
//

interface IMesh : IUnknown
{
   virtual void GetAABB(tVec3 * pMaxs, tVec3 * pMins) const = 0;

   virtual void Render() const = 0;
};

///////////////////////////////////////////////////////////////////////////////

IMesh * MeshLoad(IResourceManager * pResMgr, const char * pszMesh);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MESH_H
