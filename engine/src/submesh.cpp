///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "mesh.h"
#include "render.h"
#include "material.h"
#include "comtools.h"
#include "str.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSubMesh
//

class cSubMesh : public cComObject<IMPLEMENTS(ISubMesh)>
{
   cSubMesh(const cSubMesh &); // un-implemented
   const cSubMesh & operator=(const cSubMesh &); // un-implemented

   cSubMesh(uint nFaces, uint nVertices,
            IIndexBuffer * pIndexBuffer, 
            IVertexBuffer * pVertexBuffer,
            IVertexDeclaration * pVertexDecl);

public:
   friend ISubMesh * SubMeshCreate(uint nFaces, uint nVertices,
                                   IVertexDeclaration * pVertexDecl,
                                   IRenderDevice * pRenderDevice);
   friend ISubMesh * SubMeshCreate(uint nFaces, uint usage, IRenderDevice * pRenderDevice);
   ~cSubMesh();

   virtual const char * GetMaterialName() const;
   virtual void SetMaterialName(const char * pszMaterialName);

   virtual tResult GetMaterial(IMaterial * * ppMaterial);
   virtual tResult SetMaterial(IMaterial * pMaterial);

   virtual uint GetVertexCount() const
   {
      return m_nVerts;
   }

   virtual tResult GetVertexBuffer(IVertexBuffer * * ppVertexBuffer);

   virtual tResult LockVertexBuffer(uint lock, void * * ppData);
   virtual tResult UnlockVertexBuffer();

   virtual uint GetIndexCount() const
   {
      return m_nIndices;
   }

   virtual tResult GetIndexBuffer(IIndexBuffer * * ppIndexBuffer);

   virtual tResult LockIndexBuffer(uint lock, void * * ppData);
   virtual tResult UnlockIndexBuffer();

private:
   IVertexBuffer * AccessVertexBuffer() { return m_pVertexBuffer; }
   IIndexBuffer * AccessIndexBuffer() { return m_pIndexBuffer; }

   cStr m_materialName;
   cAutoIPtr<IMaterial> m_pMaterial;
   uint m_nVerts;
   uint m_nIndices;
   cAutoIPtr<IIndexBuffer> m_pIndexBuffer;
   cAutoIPtr<IVertexBuffer> m_pVertexBuffer;
};

///////////////////////////////////////

cSubMesh::cSubMesh(uint nFaces, uint nVertices,
                   IIndexBuffer * pIndexBuffer,
                   IVertexBuffer * pVertexBuffer,
                   IVertexDeclaration * pVertexDecl)
 : m_nVerts(nVertices),
   m_nIndices(nFaces * 3),
   m_pIndexBuffer(pIndexBuffer),
   m_pVertexBuffer(pVertexBuffer)
{
   if (pIndexBuffer != NULL)
      pIndexBuffer->AddRef();
   if (pVertexBuffer != NULL)
      pVertexBuffer->AddRef();
}

///////////////////////////////////////

ISubMesh * SubMeshCreate(uint nFaces, uint nVertices,
                         IVertexDeclaration * pVertexDecl,
                         IRenderDevice * pRenderDevice)
{
   if (nVertices == 0 || nFaces == 0 || pVertexDecl == NULL || pRenderDevice == NULL)
      return NULL;

   cAutoIPtr<IIndexBuffer> pIndexBuffer;
   if (pRenderDevice->CreateIndexBuffer(3 * nFaces, kBU_Default, kIBF_16Bit, kBP_Auto, &pIndexBuffer) == S_OK)
   {
      cAutoIPtr<IVertexBuffer> pVertexBuffer;
      if (pRenderDevice->CreateVertexBuffer(nVertices, kBU_Default, pVertexDecl, kBP_Auto, &pVertexBuffer) == S_OK)
      {
         return static_cast<ISubMesh *>(new cSubMesh(nFaces, nVertices, pIndexBuffer, pVertexBuffer, NULL));
      }
   }

   return NULL;
}

///////////////////////////////////////

ISubMesh * SubMeshCreate(uint nFaces, uint usage, IRenderDevice * pRenderDevice)
{
   if ((nFaces > 0) && (pRenderDevice != NULL))
   {
      cAutoIPtr<IIndexBuffer> pIndexBuffer;
      if (pRenderDevice->CreateIndexBuffer(3 * nFaces, usage, kIBF_16Bit, kBP_Auto, &pIndexBuffer) == S_OK)
      {
         return static_cast<ISubMesh *>(new cSubMesh(nFaces, 0, pIndexBuffer, NULL, NULL));
      }
   }
   return NULL;
}

///////////////////////////////////////

cSubMesh::~cSubMesh()
{
}

///////////////////////////////////////

const char * cSubMesh::GetMaterialName() const
{
   return m_materialName;
}

///////////////////////////////////////

void cSubMesh::SetMaterialName(const char * pszMaterialName)
{
   m_materialName = pszMaterialName ? pszMaterialName : "";
   // When the assigned material name changes, get rid of the cached
   // IMaterial interface pointer.
   SetMaterial(NULL);
}

///////////////////////////////////////

tResult cSubMesh::GetMaterial(IMaterial * * ppMaterial)
{
   if (ppMaterial != NULL && !!m_pMaterial)
   {
      *ppMaterial = m_pMaterial;
      (*ppMaterial)->AddRef();
      return S_OK;
   }
   return E_FAIL;
}

///////////////////////////////////////

tResult cSubMesh::SetMaterial(IMaterial * pMaterial)
{
   SafeRelease(m_pMaterial);
   if (pMaterial != NULL
      && (strcmp(pMaterial->GetName(), GetMaterialName()) == 0))
   {
      m_pMaterial = pMaterial;
      pMaterial->AddRef();
      return S_OK;
   }
   return E_FAIL;
}

///////////////////////////////////////

tResult cSubMesh::GetVertexBuffer(IVertexBuffer * * ppVertexBuffer)
{
   if (!m_pVertexBuffer || ppVertexBuffer == NULL)
   {
      return E_FAIL;
   }
   else
   {
      *ppVertexBuffer = AccessVertexBuffer();
      (*ppVertexBuffer)->AddRef();
      return S_OK;
   }
}

///////////////////////////////////////

tResult cSubMesh::LockVertexBuffer(uint lock, void * * ppData)
{
   if (AccessVertexBuffer())
   {
      return AccessVertexBuffer()->Lock(lock, ppData);
   }
   else
   {
      return E_FAIL;
   }
}

///////////////////////////////////////

tResult cSubMesh::UnlockVertexBuffer()
{
   if (AccessVertexBuffer())
   {
      return AccessVertexBuffer()->Unlock();
   }
   else
   {
      return E_FAIL;
   }
}

///////////////////////////////////////

tResult cSubMesh::GetIndexBuffer(IIndexBuffer * * ppIndexBuffer)
{
   if (!m_pIndexBuffer || ppIndexBuffer == NULL)
   {
      return E_FAIL;
   }
   else
   {
      *ppIndexBuffer = AccessIndexBuffer();
      (*ppIndexBuffer)->AddRef();
      return S_OK;
   }
}

///////////////////////////////////////

tResult cSubMesh::LockIndexBuffer(uint lock, void * * ppData)
{
   if (AccessIndexBuffer())
   {
      return AccessIndexBuffer()->Lock(lock, ppData);
   }
   else
   {
      return E_FAIL;
   }
}

///////////////////////////////////////

tResult cSubMesh::UnlockIndexBuffer()
{
   if (AccessIndexBuffer())
   {
      return AccessIndexBuffer()->Unlock();
   }
   else
   {
      return E_FAIL;
   }
}

///////////////////////////////////////////////////////////////////////////////
