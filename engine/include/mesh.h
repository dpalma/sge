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
F_DECLARE_INTERFACE(IVertexDeclaration);
F_DECLARE_INTERFACE(IVertexBuffer);
F_DECLARE_INTERFACE(IIndexBuffer);
F_DECLARE_INTERFACE(IMaterial);

F_DECLARE_INTERFACE(IMesh);
F_DECLARE_INTERFACE(ISubMesh);
F_DECLARE_INTERFACE(ISkeleton);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IMesh
//

interface IMesh : IUnknown
{
   virtual void GetAABB(tVec3 * pMaxs, tVec3 * pMins) const = 0;

   virtual void Render(IRenderDevice * pRenderDevice) const = 0;

   virtual uint GetVertexCount() const = 0;
   virtual tResult GetVertexBuffer(IVertexBuffer * * ppVertexBuffer) = 0;

   virtual tResult LockVertexBuffer(void * * ppData) = 0;
   virtual tResult UnlockVertexBuffer() = 0;

   virtual tResult AddMaterial(IMaterial * pMaterial) = 0;
   virtual tResult FindMaterial(const char * pszName, IMaterial * * ppMaterial) const = 0;
   virtual uint GetMaterialCount() const = 0;
   virtual tResult GetMaterial(uint index, IMaterial * * ppMaterial) const = 0;

   virtual tResult AddSubMesh(ISubMesh * pSubMesh) = 0;
   virtual uint GetSubMeshCount() const = 0;
   virtual tResult GetSubMesh(uint index, ISubMesh * * ppSubMesh) const = 0;

   virtual tResult AttachSkeleton(ISkeleton * pSkeleton) = 0;
   virtual tResult GetSkeleton(ISkeleton * * ppSkeleton) = 0;
};

///////////////////////////////////////

ENGINE_API IMesh * MeshCreate();

ENGINE_API IMesh * MeshCreate(uint nVertices, 
                              uint options, 
                              IVertexDeclaration * pVertexDecl, 
                              IRenderDevice * pRenderDevice);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ISubMesh
//

interface ISubMesh : IUnknown
{
   virtual const char * GetMaterialName() const = 0;
   virtual void SetMaterialName(const char * pszMaterialName) = 0;

   // Used only to cache a pointer to the named material. Use SetMaterialName
   // to assign a material to a submesh.
   virtual tResult GetMaterial(IMaterial * * ppMaterial) = 0;
   virtual tResult SetMaterial(IMaterial * pMaterial) = 0;

   virtual uint GetVertexCount() const = 0;
   virtual tResult GetVertexBuffer(IVertexBuffer * * ppVertexBuffer) = 0;

   virtual tResult LockVertexBuffer(void * * ppData) = 0;
   virtual tResult UnlockVertexBuffer() = 0;

   virtual uint GetIndexCount() const = 0;
   virtual tResult GetIndexBuffer(IIndexBuffer * * ppIndexBuffer) = 0;

   virtual tResult LockIndexBuffer(void * * ppData) = 0;
   virtual tResult UnlockIndexBuffer() = 0;
};

///////////////////////////////////////

ENGINE_API ISubMesh * SubMeshCreate(uint nFaces, uint nVertices, 
                                    IVertexDeclaration * pVertexDecl, 
                                    IRenderDevice * pRenderDevice);

ENGINE_API ISubMesh * SubMeshCreate(uint nFaces, IRenderDevice * pRenderDevice);

///////////////////////////////////////////////////////////////////////////////

ENGINE_API IMesh * MeshLoad(IResourceManager * pResMgr, IRenderDevice * pRenderDevice, const char * pszMesh);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MESH_H
