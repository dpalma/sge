///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MESH_H
#define INCLUDED_MESH_H

#include "enginedll.h"
#include "combase.h"

#ifdef _MSC_VER
#pragma once
#endif

// forward declarations
template <typename T> class cVec3;
typedef class cVec3<float> tVec3;
F_DECLARE_INTERFACE(IResourceManager);
F_DECLARE_INTERFACE(IRenderDevice);
F_DECLARE_INTERFACE(IMaterial);

F_DECLARE_INTERFACE(IMesh);
F_DECLARE_INTERFACE(ISubMesh);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IMesh
//

interface IMesh : IUnknown
{
   virtual void GetAABB(tVec3 * pMaxs, tVec3 * pMins) const = 0;

   virtual void Render(IRenderDevice * pRenderDevice) const = 0;

   virtual tResult AddMaterial(IMaterial * pMaterial) = 0;
   virtual tResult FindMaterial(const char * pszName, IMaterial * * ppMaterial) = 0;
};

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ISubMesh
//

interface ISubMesh : IUnknown
{
};

///////////////////////////////////////////////////////////////////////////////

ENGINE_API IMesh * MeshLoad(IResourceManager * pResMgr, IRenderDevice * pRenderDevice, const char * pszMesh);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MESH_H
