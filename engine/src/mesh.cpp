///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "mesh.h"
#include "skeleton.h"
#include "render.h"
#include "material.h"
#include "vec3.h"
#include "resmgr.h"
#include "readwriteapi.h"
#include "filespec.h"

#include <vector>
#include <algorithm>
#include <functional>
#include <cstring>
#include <cfloat>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

IMesh * Load3ds(IRenderDevice * pRenderDevice, IReader * pReader);
IMesh * LoadMs3d(IRenderDevice * pRenderDevice, IReader * pReader);

///////////////////////////////////////////////////////////////////////////////

static tResult CalculateAABB(uint nVertices, IVertexBuffer * pVertexBuffer,
                             tVec3 * pMax, tVec3 * pMin)
{
   Assert(pVertexBuffer != NULL);

   tResult result = E_FAIL;

   static const uint vertexDeclTypeSizes[] =
   {
      1 * sizeof(float), // kVDT_Float1
      2 * sizeof(float), // kVDT_Float2
      3 * sizeof(float), // kVDT_Float3
      4 * sizeof(float), // kVDT_Float4
      4 * sizeof(unsigned char), // kVDT_UnsignedByte4
      2 * sizeof(short), // kVDT_Short2
      4 * sizeof(short), // kVDT_Short4
   };

   cAutoIPtr<IVertexDeclaration> pVertexDecl;
   if (pVertexBuffer->GetVertexDeclaration(&pVertexDecl) == S_OK)
   {
      uint posOffset = 0, structSize;
      sVertexElement elements[32];
      int nElements = _countof(elements);

      if (pVertexDecl->GetStructSize(&structSize, NULL) == S_OK
         && pVertexDecl->GetElements(elements, &nElements) == S_OK)
      {
         for (int i = 0; i < nElements; i++)
         {
            if (elements[i].usage == kVDU_Position)
            {
               break;
            }
            else
            {
               posOffset += vertexDeclTypeSizes[elements[i].type];
            }
         }

         byte * pVertexData;
         if (pVertexBuffer->Lock((void**)&pVertexData) == S_OK)
         {
            tVec3 max(FLT_MIN, FLT_MIN, FLT_MIN);
            tVec3 min(FLT_MAX, FLT_MAX, FLT_MAX);

            for (uint i = 0; i < nVertices; i++, pVertexData += structSize)
            {
               float * pPos = (float *)(pVertexData + posOffset);

               if (max.x < pPos[0])
                  max.x = pPos[0];
               if (min.x > pPos[0])
                  min.x = pPos[0];

               if (max.y < pPos[1])
                  max.y = pPos[1];
               if (min.y > pPos[1])
                  min.y = pPos[1];

               if (max.z < pPos[2])
                  max.z = pPos[2];
               if (min.z > pPos[2])
                  min.z = pPos[2];
            }

            if (pMax != NULL)
               *pMax = max;
            if (pMin != NULL)
               *pMin = min;

            pVertexBuffer->Unlock();

            result = S_OK;
         }
      }
   }

   return result;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMesh
//

class cMesh : public cComObject<IMPLEMENTS(IMesh)>
{
   cMesh(const cMesh &); // un-implemented
   const cMesh & operator=(const cMesh &); // un-implemented

public:
   cMesh();
   cMesh(uint nVertices, IVertexBuffer * pVertexBuffer);
   virtual ~cMesh();

   virtual void GetAABB(tVec3 * pMaxs, tVec3 * pMins) const;
   virtual void Render(IRenderDevice * pRenderDevice) const;
   virtual uint GetVertexCount() const;
   virtual tResult GetVertexBuffer(IVertexBuffer * * ppVertexBuffer);
   virtual tResult LockVertexBuffer(void * * ppData);
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

private:
   uint m_nVerts;
   cAutoIPtr<IVertexBuffer> m_pVertexBuffer;

   typedef std::vector<IMaterial *> tMaterials;
   tMaterials m_materials;

   typedef std::vector<ISubMesh *> tSubMeshes;
   tSubMeshes m_subMeshes;

   cAutoIPtr<ISkeleton> m_pSkeleton;
};

///////////////////////////////////////

cMesh::cMesh()
 : m_nVerts(0)
{
}

///////////////////////////////////////

cMesh::cMesh(uint nVertices, IVertexBuffer * pVertexBuffer)
 : m_nVerts(nVertices)
{
   if (pVertexBuffer != NULL)
   {
      m_pVertexBuffer = pVertexBuffer;
      m_pVertexBuffer->AddRef();
   }
}

///////////////////////////////////////

cMesh::~cMesh()
{
   std::for_each(m_materials.begin(), m_materials.end(), CTInterfaceMethodRef(&IUnknown::Release));
   m_materials.clear();

   std::for_each(m_subMeshes.begin(), m_subMeshes.end(), CTInterfaceMethodRef(&IUnknown::Release));
   m_subMeshes.clear();
}

///////////////////////////////////////

void cMesh::GetAABB(tVec3 * pMax, tVec3 * pMin) const
{
   tVec3 max(FLT_MIN, FLT_MIN, FLT_MIN);
   tVec3 min(FLT_MAX, FLT_MAX, FLT_MAX);

   if (!m_pVertexBuffer)
   {
      tSubMeshes::const_iterator iter;
      for (iter = m_subMeshes.begin(); iter != m_subMeshes.end(); iter++)
      {
         tVec3 maxSub, minSub;

         cAutoIPtr<IVertexBuffer> pVertexBuffer;
         if ((*iter)->GetVertexBuffer(&pVertexBuffer) == S_OK)
         {
            if (CalculateAABB((*iter)->GetVertexCount(), pVertexBuffer,
                              &maxSub, &minSub) == S_OK)
            {
               if (max.x < maxSub.x)
                  max.x = maxSub.x;
               if (min.x > minSub.x)
                  min.x = minSub.x;

               if (max.y < maxSub.y)
                  max.y = maxSub.y;
               if (min.y > minSub.y)
                  min.y = minSub.y;

               if (max.z < maxSub.z)
                  max.z = maxSub.z;
               if (min.z > minSub.z)
                  min.z = minSub.z;
            }
         }
      }
   }
   else
   {
      CalculateAABB(GetVertexCount(), 
         const_cast<IVertexBuffer *>(m_pVertexBuffer.operator->()), 
         &max, &min);
   }

   if (pMax != NULL)
      *pMax = max;
   if (pMin != NULL)
      *pMin = min;
}

///////////////////////////////////////

class cRenderSubMesh
{
public:
   cRenderSubMesh(IMesh * pMesh, IRenderDevice * pRenderDevice);

   void operator()(ISubMesh * pSubMesh);

private:
   cAutoIPtr<IMesh> m_pMesh;
   cAutoIPtr<IRenderDevice> m_pRenderDevice;
};

///////////////////////////////////////

cRenderSubMesh::cRenderSubMesh(IMesh * pMesh, IRenderDevice * pRenderDevice)
 : m_pMesh(pMesh),
   m_pRenderDevice(pRenderDevice)
{
   Assert(pMesh && pRenderDevice);
   pMesh->AddRef();
   pRenderDevice->AddRef();
};

///////////////////////////////////////

void cRenderSubMesh::operator()(ISubMesh * pSubMesh)
{
   cAutoIPtr<IMaterial> pMaterial;
   if (pSubMesh->GetMaterial(&pMaterial) != S_OK || !pMaterial)
   {
      if (m_pMesh->FindMaterial(pSubMesh->GetMaterialName(), &pMaterial) == S_OK)
      {
         pSubMesh->SetMaterial(pMaterial);
      }
   }

   AssertMsg(!!pMaterial, "SubMesh object has an invalid material!");

   cAutoIPtr<IIndexBuffer> pIndexBuffer;
   if (pSubMesh->GetIndexBuffer(&pIndexBuffer) == S_OK)
   {
      cAutoIPtr<IVertexBuffer> pVertexBuffer;
      if (pSubMesh->GetVertexBuffer(&pVertexBuffer) == S_OK)
      {
         m_pRenderDevice->Render(kRP_Triangles, pMaterial, 
            pSubMesh->GetIndexCount(), pIndexBuffer,
            0, pVertexBuffer);
      }
   }
}

///////////////////////////////////////

void cMesh::Render(IRenderDevice * pRenderDevice) const
{
   // No shared vertex buffer, render each sub-mesh with its own vertex buffer
   if (!m_pVertexBuffer)
   {
      std::for_each(m_subMeshes.begin(), m_subMeshes.end(),
         cRenderSubMesh(const_cast<cMesh *>(this), pRenderDevice));
   }
   else
   {
      tSubMeshes::const_iterator iter;
      for (iter = m_subMeshes.begin(); iter != m_subMeshes.end(); iter++)
      {
         cAutoIPtr<IMaterial> pMaterial;
         if ((*iter)->GetMaterial(&pMaterial) != S_OK || !pMaterial)
         {
            if (FindMaterial((*iter)->GetMaterialName(), &pMaterial) == S_OK)
            {
               (*iter)->SetMaterial(pMaterial);
            }
         }

         AssertMsg(!!pMaterial, "SubMesh object has an invalid material!");

         cAutoIPtr<IIndexBuffer> pIndexBuffer;
         if ((*iter)->GetIndexBuffer(&pIndexBuffer) == S_OK)
         {
            pRenderDevice->Render(kRP_Triangles, pMaterial, 
               (*iter)->GetIndexCount(), pIndexBuffer,
               0, const_cast<IVertexBuffer *>(m_pVertexBuffer.operator->()));
         }
      }
   }
}

///////////////////////////////////////

uint cMesh::GetVertexCount() const
{
   return m_nVerts;
}

///////////////////////////////////////

tResult cMesh::GetVertexBuffer(IVertexBuffer * * ppVertexBuffer)
{
   if (ppVertexBuffer == NULL)
   {
      return E_FAIL;
   }

   if (!m_pVertexBuffer)
   {
      return S_FALSE;
   }
   else
   {
      *ppVertexBuffer = m_pVertexBuffer;
      (*ppVertexBuffer)->AddRef();
      return S_OK;
   }
}

///////////////////////////////////////

tResult cMesh::LockVertexBuffer(void * * ppData)
{
   if (!!m_pVertexBuffer)
   {
      return m_pVertexBuffer->Lock(ppData);
   }
   else
   {
      return S_FALSE;
   }
}

///////////////////////////////////////

tResult cMesh::UnlockVertexBuffer()
{
   if (!!m_pVertexBuffer)
   {
      return m_pVertexBuffer->Unlock();
   }
   else
   {
      return S_FALSE;
   }
}

///////////////////////////////////////

tResult cMesh::AddMaterial(IMaterial * pMaterial)
{
   if (pMaterial == NULL || FindMaterial(pMaterial->GetName(), NULL) == S_OK)
      return E_FAIL;
   m_materials.push_back(pMaterial);
   pMaterial->AddRef();
   return S_OK;
}

///////////////////////////////////////

tResult cMesh::FindMaterial(const char * pszName, IMaterial * * ppMaterial) const
{
   if (pszName == NULL || pszName[0] == 0)
      return E_FAIL;
   tMaterials::const_iterator iter;
   for (iter = m_materials.begin(); iter != m_materials.end(); iter++)
   {
      if (strcmp(pszName, (*iter)->GetName()) == 0)
      {
         if (ppMaterial != NULL)
         {
            *ppMaterial = *iter;
            (*ppMaterial)->AddRef();
         }
         return S_OK;
      }
   }
   return S_FALSE;
}

///////////////////////////////////////

uint cMesh::GetMaterialCount() const
{
   return m_materials.size();
}

///////////////////////////////////////

tResult cMesh::GetMaterial(uint index, IMaterial * * ppMaterial) const
{
   if ((index < m_materials.size()) && (ppMaterial != NULL))
   {
      *ppMaterial = m_materials[index];
      (*ppMaterial)->AddRef();
      return S_OK;
   }
   return E_FAIL;
}

///////////////////////////////////////

tResult cMesh::AddSubMesh(ISubMesh * pSubMesh)
{
   if (pSubMesh != NULL)
   {
      m_subMeshes.push_back(pSubMesh);
      pSubMesh->AddRef();
      return S_OK;
   }
   return E_FAIL;
}

///////////////////////////////////////

uint cMesh::GetSubMeshCount() const
{
   return m_subMeshes.size();
}

///////////////////////////////////////

tResult cMesh::GetSubMesh(uint index, ISubMesh * * ppSubMesh) const
{
   if ((index < m_subMeshes.size()) && (ppSubMesh != NULL))
   {
      *ppSubMesh = m_subMeshes[index];
      (*ppSubMesh)->AddRef();
      return S_OK;
   }
   return E_FAIL;
}

///////////////////////////////////////

tResult cMesh::AttachSkeleton(ISkeleton * pSkeleton)
{
   SafeRelease(m_pSkeleton);
   m_pSkeleton = pSkeleton;
   if (m_pSkeleton)
   {
      m_pSkeleton->AddRef();
   }
   return S_OK;
}

///////////////////////////////////////

tResult cMesh::GetSkeleton(ISkeleton * * ppSkeleton)
{
   if (ppSkeleton != NULL)
   {
      *ppSkeleton = static_cast<ISkeleton *>(m_pSkeleton);
      if (*ppSkeleton != NULL)
      {
         (*ppSkeleton)->AddRef();
         return S_OK;
      }
      else
      {
         return S_FALSE;
      }
   }
   return E_FAIL;
}

///////////////////////////////////////

IMesh * MeshCreate()
{
   return static_cast<IMesh *>(new cMesh);
}

///////////////////////////////////////

IMesh * MeshCreate(uint nVertices, 
                   IVertexDeclaration * pVertexDecl, 
                   IRenderDevice * pRenderDevice)
{
   if ((nVertices > 0) && (pVertexDecl != NULL) && (pRenderDevice != NULL))
   {
      cAutoIPtr<IVertexBuffer> pVertexBuffer;
      if (pRenderDevice->CreateVertexBuffer(nVertices, pVertexDecl, kMP_Auto, &pVertexBuffer) == S_OK)
      {
         return static_cast<IMesh *>(new cMesh(nVertices, pVertexBuffer));
      }
   }
   return NULL;
}

///////////////////////////////////////////////////////////////////////////////

IMesh * MeshLoad(IResourceManager * pResMgr, IRenderDevice * pRenderDevice, const char * pszMesh)
{
   typedef IMesh * (* tMeshLoadFn)(IRenderDevice *, IReader *);

   static const struct
   {
      const char * ext;
      tMeshLoadFn pfn;
   }
   meshFileLoaders[] =
   {
      { "3ds", Load3ds },
      { "ms3d", LoadMs3d },
   };

   cFileSpec meshFile(pszMesh);

   for (int i = 0; i < _countof(meshFileLoaders); i++)
   {
      if (stricmp(meshFileLoaders[i].ext, meshFile.GetFileExt()) == 0)
      {
         cAutoIPtr<IReader> pReader = pResMgr->Find(meshFile.GetName());
         if (!pReader)
         {
            return NULL;
         }

         IMesh * pMesh = (*meshFileLoaders[i].pfn)(pRenderDevice, pReader);
         if (pMesh != NULL)
         {
            return pMesh;
         }
      }
   }

   DebugMsg1("Unsupported mesh file format: \"%s\"\n", meshFile.GetFileExt());
   return NULL;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cMeshTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cMeshTests);
      CPPUNIT_TEST(TestSave);
   CPPUNIT_TEST_SUITE_END();

   void TestSave();
};

CPPUNIT_TEST_SUITE_REGISTRATION(cMeshTests);

void cMeshTests::TestSave()
{
	cAutoIPtr<IMesh> pMesh = MeshCreate();
	if (!!pMesh)
	{
	}
}

#endif

///////////////////////////////////////////////////////////////////////////////
