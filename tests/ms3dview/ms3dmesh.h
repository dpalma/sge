///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MS3DMESH_H
#define INCLUDED_MS3DMESH_H

#include "meshapi.h"

#include "readwriteapi.h"
#include "matrix4.h"
#include "vec3.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IResourceManager);
F_DECLARE_INTERFACE(IRenderDevice);
F_DECLARE_INTERFACE(ISkeleton);

//////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dMesh
//

class cMs3dMesh : public cComObject<IMPLEMENTS(IMesh)>
{
   cMs3dMesh(const cMs3dMesh &); // private, un-implemented
   const cMs3dMesh & operator =(const cMs3dMesh &); // private, un-implemented

public:
   cMs3dMesh();
   ~cMs3dMesh();

   virtual void GetAABB(tVec3 * pMaxs, tVec3 * pMins) const;
   virtual void Render(IRenderDevice * pRenderDevice) const;
   virtual uint GetVertexCount() const;
   virtual tResult GetVertexBuffer(IVertexBuffer * * ppVertexBuffer);
   virtual tResult LockVertexBuffer(uint lock, void * * ppData);
   virtual tResult UnlockVertexBuffer();
   virtual tResult AddMaterial(IMaterial * pMaterial);
   virtual tResult FindMaterial(const char * pszName, IMaterial * * ppMaterial) const;
   virtual uint GetMaterialCount() const;
   virtual tResult GetMaterial(uint index, IMaterial * * ppMaterial) const;
   virtual tResult AddSubMesh(ISubMesh * pSubMesh);
   virtual uint GetSubMeshCount() const;
   virtual tResult GetSubMesh(uint index, ISubMesh * * ppSubMesh) const;
   virtual tResult AttachSkeleton(ISkeleton * pSkeleton);
   virtual tResult GetSkeleton(ISkeleton * * ppSkeleton);

   tResult Load(const char * pszMesh, IRenderDevice * pRenderDevice, IResourceManager * pResourceManager);

   void SetFrame(float percent);

   // using software or vertex program rendering?
   bool IsRenderingSoftware() const;

private:
   void Prepare();

   cAutoIPtr<IMesh> m_pInnerMesh;

   mutable bool m_bPrepared;

   std::vector<tMatrix4> m_boneMatrices;
};

inline bool cMs3dMesh::IsRenderingSoftware() const
{
   return true;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MS3DMESH_H
