////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "rendererdx.h"
#include "renderfontapi.h"

#include "sys.h"

#include "axisalignedbox.h"
#include "color.h"
#include "resourceapi.h"
#include "techhash.h"
#include "techmath.h"
#include "vec3.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>

#ifdef HAVE_CG
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include <CgFX/ICgFX.h>
#include <CgFX/ICgFXEffect.h>
#endif

#include <cstring>

#include "dbgalloc.h" // must be last header

////////////////////////////////////////////////////////////////////////////////

extern const uint kVertexFVF;

extern tResult Render2DCreateDX(IDirect3DDevice9 * pD3dDevice, IRender2D * * ppRender2D);

extern tResult RenderFontCreateD3DX(const tChar * pszFont, int fontPointSize, uint flags, IDirect3DDevice9 * pD3dDevice, IRenderFont * * ppFont);

////////////////////////////////////////////////////////////////////////////////

// Ensure the vertex element types can be used as lookup table indices
AssertOnce(kVET_Float1 == 0);
AssertOnce(kVET_Float2 == 1);
AssertOnce(kVET_Float3 == 2);
AssertOnce(kVET_Float4 == 3);
AssertOnce(kVET_Color == 4);
AssertOnce(kVET_UnsignedByte4 == 5);
AssertOnce(kVET_Short2 == 6);
AssertOnce(kVET_Short4 == 7);

static uint GetVertexSize(const sVertexElement * pElements, uint nElements)
{
   static const uint vertexElementSizes[] =
   {
      1 * sizeof(float),         // kVET_Float1
      2 * sizeof(float),         // kVET_Float2
      3 * sizeof(float),         // kVET_Float3
      4 * sizeof(float),         // kVET_Float4
      sizeof(uint32),            // kVET_Color
      4 * sizeof(unsigned char), // kVET_UnsignedByte4
      2 * sizeof(short),         // kVET_Short2
      4 * sizeof(short),         // kVET_Short4
   };
   uint vertexSize = 0;
   for (uint i = 0; i < nElements; i++)
   {
      Assert((uint)pElements[i].type < _countof(vertexElementSizes));
      vertexSize += vertexElementSizes[pElements[i].type];
   }
   return vertexSize;
}

////////////////////////////////////////////////////////////////////////////////

inline D3DPRIMITIVETYPE GetD3dPrimitive(ePrimitiveType primitive)
{
   static const D3DPRIMITIVETYPE d3dPrimitiveTable[] =
   {
      D3DPT_LINELIST,         // kRP_Lines,
      D3DPT_LINESTRIP,        // kRP_LineStrip,
      D3DPT_TRIANGLELIST,     // kRP_Triangles
      D3DPT_TRIANGLESTRIP,    // kRP_TriangleStrip
      D3DPT_TRIANGLEFAN,      // kRP_TriangleFan
   };
   Assert((uint)primitive < _countof(d3dPrimitiveTable));
   return d3dPrimitiveTable[primitive];
}

////////////////////////////////////////////////////////////////////////////////

static bool ValidateIndices(const uint16 * pIndices, uint nIndices, uint nVertices)
{
   for (uint i = 0; i < nIndices; i++)
   {
      if (pIndices[i] >= nVertices)
      {
         ErrorMsg2("INDEX %d OUTSIDE OF VERTEX ARRAY (size %d)!!!\n", pIndices[i], nVertices);
         return false;
      }
   }
   return true;
}


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRendererDX
//

////////////////////////////////////////

cRendererDX::cRendererDX()
 : m_bInScene(false)
#ifdef HAVE_CG
 , m_cgContext(NULL)
 , m_oldCgErrorCallback(NULL)
 , m_cgProfile(CG_PROFILE_UNKNOWN)
#endif
 , m_nVertexElements(0)
 , m_vertexSize(0)
 , m_indexFormat(kIF_16Bit)
{
   SetIndexFormat(kIF_16Bit);
}

////////////////////////////////////////

cRendererDX::~cRendererDX()
{
}

////////////////////////////////////////

tResult cRendererDX::Init()
{
   return S_OK;
}

////////////////////////////////////////

tResult cRendererDX::Term()
{
#ifdef HAVE_CG
   if (m_cgContext != NULL)
   {
      if (m_oldCgErrorCallback != NULL)
      {
         cgSetErrorCallback(m_oldCgErrorCallback);
         m_oldCgErrorCallback = NULL;
      }

      cgDestroyContext(m_cgContext);
      m_cgContext = NULL;

      m_cgProfile = CG_PROFILE_UNKNOWN;
   }
#endif

   tFontMap::iterator iter = m_fontMap.begin();
   for (; iter != m_fontMap.end(); ++iter)
   {
      SafeRelease(iter->second);
   }
   m_fontMap.clear();

   return S_OK;
}

////////////////////////////////////////

tResult cRendererDX::BeginScene()
{
   if (!m_pD3dDevice)
   {
      if (SysGetDirect3DDevice9(&m_pD3dDevice) != S_OK)
      {
         return E_FAIL;
      }

#ifdef HAVE_CG
      if (m_cgContext == NULL)
      {
         m_cgContext = cgCreateContext();
         Assert(m_oldCgErrorCallback == NULL);
         m_oldCgErrorCallback = cgGetErrorCallback();
         cgSetErrorCallback(CgErrorCallback);
         Assert(m_cgProfile == CG_PROFILE_UNKNOWN);
         m_cgProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
      }
#endif

      if (Render2DCreateDX(m_pD3dDevice, &m_pRender2D) != S_OK)
      {
         return E_FAIL;
      }
   }

   if (!!m_pD3dDevice)
   {
      AssertMsg(!m_bInScene, "Cannot nest BeginScene/EndScene calls");
      if (!m_bInScene)
      {
         m_bInScene = true;
         m_pD3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1, 0);
         return S_OK;
      }
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cRendererDX::EndScene()
{
   if (m_bInScene)
   {
      m_bInScene = false;
      m_pD3dDevice->EndScene();
      m_pD3dDevice->Present(NULL, NULL, NULL, NULL);
      return S_OK;
   }
   return E_FAIL;
}

////////////////////////////////////////

tResult cRendererDX::CreateTexture(IImage * pImage, bool bAutoGenMipMaps, void * * ppTexture)
{
   if (pImage == NULL || ppTexture == NULL)
   {
      return E_POINTER;
   }

   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cRendererDX::SetVertexFormat(const sVertexElement * pVertexElements, uint nVertexElements)
{
   if (nVertexElements >= kMaxVertexElements)
   {
      return E_INVALIDARG;
   }

   if (pVertexElements == NULL || nVertexElements == 0)
   {
#ifdef _DEBUG
      memset(m_vertexElements, 0xBC, sizeof(m_vertexElements));
#endif
      m_nVertexElements = 0;
      m_vertexSize = 0;
   }
   else
   {
      memcpy(m_vertexElements, pVertexElements, nVertexElements * sizeof(sVertexElement));
      m_nVertexElements = nVertexElements;
      m_vertexSize = GetVertexSize(pVertexElements, nVertexElements);
   }

   return S_OK;
}

////////////////////////////////////////

tResult cRendererDX::SetIndexFormat(eIndexFormat indexFormat)
{
   if (!m_pD3dDevice)
   {
      return E_FAIL;
   }
   m_indexFormat = indexFormat;
   if (indexFormat == kIF_16Bit)
   {
      return S_OK;
   }
   else if (indexFormat == kIF_32Bit)
   {
      return S_OK;
   }
   return E_FAIL;
}

////////////////////////////////////////

tResult cRendererDX::SubmitVertices(const void * pVertices, uint nVertices)
{
   if (pVertices == NULL)
   {
      return E_POINTER;
   }

   if (nVertices == 0)
   {
      return E_INVALIDARG;
   }

   if (m_vertexSize == 0)
   {
      return E_FAIL;
   }

   // TODO

   return S_OK;
}

////////////////////////////////////////

tResult cRendererDX::SetDiffuseColor(const float diffuse[4])
{
   if (diffuse == NULL)
   {
      return E_POINTER;
   }
   // TODO
   return S_OK;
}

////////////////////////////////////////

tResult cRendererDX::SetTexture(uint textureUnit, const tChar * pszTexture)
{
   if (textureUnit >= 8)
   {
      return E_INVALIDARG;
   }
   if (pszTexture == NULL)
   {
      return E_POINTER;
   }

   // TODO
   //UseGlobal(ResourceManager);
   //if (pResourceManager->Load(pszTexture, kRT_D3dTexture, NULL, (void**)&pD3dTexture) == S_OK)
   //{
   //   return S_OK;
   //}

   return E_FAIL;
}

////////////////////////////////////////

tResult cRendererDX::Render(ePrimitiveType primitive, const void * pIndices, uint nIndices)
{
   if (pIndices == NULL)
   {
      return E_POINTER;
   }

   if (nIndices == 0)
   {
      return E_INVALIDARG;
   }

   if (!m_bInScene || !m_pD3dDevice)
   {
      return E_FAIL;
   }

   uint nVertices = 0;
   uint nPrimitives = 0; // TODO

   if (m_pD3dDevice->DrawIndexedPrimitiveUP(GetD3dPrimitive(primitive), 0, nVertices, nPrimitives, pIndices,
      (m_indexFormat == kIF_16Bit) ? D3DFMT_INDEX16 : D3DFMT_INDEX32, NULL, 0) != D3D_OK)
   {
      return E_FAIL;
   }

   return S_OK;
}

////////////////////////////////////////

#undef CreateFont
tResult cRendererDX::CreateFont(const tChar * pszFont, int fontPointSize, uint flags, IRenderFont * * ppFont)
{
   uint h = Hash(pszFont, _tcslen(pszFont) * sizeof(tChar));
   h = hash(reinterpret_cast<byte *>(&fontPointSize), sizeof(fontPointSize), h);
   h = hash(reinterpret_cast<byte *>(&flags), sizeof(flags), h);

   tFontMap::iterator f = m_fontMap.find(h);
   if (f != m_fontMap.end())
   {
      *ppFont = CTAddRef(f->second);
      return S_OK;
   }

   tResult result = RenderFontCreateD3DX(pszFont, fontPointSize, flags, m_pD3dDevice, ppFont);

   if (result == S_OK)
   {
      m_fontMap[h] = CTAddRef(*ppFont);
   }

   return result;
}

////////////////////////////////////////

tResult cRendererDX::Begin2D(int width, int height, IRender2D * * ppRender2D)
{
   if (ppRender2D == NULL)
   {
      return E_POINTER;
   }

   if (!m_pStateBlock)
   {
      m_pD3dDevice->BeginStateBlock();

      m_pD3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
      m_pD3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
      m_pD3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

      D3DVIEWPORT9 viewport;
      m_pD3dDevice->GetViewport(&viewport);

      // TODO: Call should use the following parameters: (0, width, height, 0, -99999, 99999);

      D3DXMATRIX ortho;
      D3DXMatrixOrthoOffCenterLH(&ortho,
         0, static_cast<float>(viewport.Width),
         static_cast<float>(viewport.Height), 0,
         viewport.MinZ, viewport.MaxZ);
      m_pD3dDevice->SetTransform(D3DTS_PROJECTION, &ortho);

      m_pD3dDevice->SetFVF(kVertexFVF);

      m_pD3dDevice->EndStateBlock(&m_pStateBlock);
   }

   if (!!m_pStateBlock)
   {
      m_pStateBlock->Apply();
   }

   return m_pRender2D.GetPointer(ppRender2D);
}

////////////////////////////////////////

tResult cRendererDX::End2D()
{
   return S_OK;
}

////////////////////////////////////////

void cRendererDX::CgErrorCallback()
{
#ifdef HAVE_CG
   CGerror lastError = cgGetError();
   if (lastError)
   {
      ErrorMsg(cgGetErrorString(lastError));
      const char * pszListing = cgGetLastListing(g_CgContext);
      if (pszListing != NULL)
      {
         ErrorMsg1("   %s\n", pszListing);
      }
   }
#endif
}

////////////////////////////////////////////////////////////////////////////////

tResult RendererCreate()
{
   cAutoIPtr<IRenderer> p(static_cast<IRenderer*>(new cRendererDX));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IRenderer, p);
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CG

CGprofile g_CgProfile = CG_PROFILE_UNKNOWN;

void * CgProgramLoad(IReader * pReader)
{
   if (pReader == NULL)
   {
      return NULL;
   }

   // Must get the Cg context first
   CGcontext cgContext = CgGetContext();
   if (cgContext == NULL)
   {
      return NULL;
   }

   if (g_CgProfile == CG_PROFILE_UNKNOWN)
   {
      g_CgProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
      if (g_CgProfile == CG_PROFILE_UNKNOWN)
      {
         return NULL;
      }
   }

   ulong length = 0;
   if (pReader->Seek(0, kSO_End) == S_OK
      && pReader->Tell(&length) == S_OK
      && pReader->Seek(0, kSO_Set) == S_OK)
   {
      cAutoBuffer autoBuffer;
      char stackBuffer[256];
      char * pBuffer = NULL;

      if (length >= 32768)
      {
         WarnMsg1("Sanity check failure loading Cg program %d bytes long\n", length);
         return NULL;
      }

      if (length < sizeof(stackBuffer))
      {
         pBuffer = stackBuffer;
      }
      else
      {
         if (autoBuffer.Malloc(sizeof(char) * (length + 1), (void**)&pBuffer) != S_OK)
         {
            return NULL;
         }
      }

      if (pReader->Read(pBuffer, length) == S_OK)
      {
         pBuffer[length] = 0;

         CGprogram program = cgCreateProgram(cgContext, CG_SOURCE, pBuffer, g_CgProfile, NULL, NULL);
         if (program != NULL)
         {
            cgGLLoadProgram(program);
            return program;
         }
      }
   }

   return NULL;
}

void CgProgramUnload(void * pData)
{
   CGprogram program = reinterpret_cast<CGprogram>(pData);
   if (program != NULL)
   {
      cgDestroyProgram(program);
   }
}

#endif // HAVE_CG

///////////////////////////////////////////////////////////////////////////////

tResult RendererResourceRegister()
{
   UseGlobal(ResourceManager);
   if (!!pResourceManager)
   {
#ifdef HAVE_CG
      if (pResourceManager->RegisterFormat(kRT_CgProgram, _T("cg"), CgProgramLoad, NULL, CgProgramUnload) == S_OK
//         && pResourceManager->RegisterFormat(kRT_CgEffect, _T("fx"), CgEffectLoad, NULL, CgEffectUnload) == S_OK
         )
      {
         return S_OK;
      }
#endif
   }
   return E_FAIL;
}

////////////////////////////////////////////////////////////////////////////////

const sVertexElement g_wireFrameVertex[] =
{
   { kVEU_Position,  kVET_Float3,   0, 0 },
};

void RenderWireFrame(const tAxisAlignedBox & box, const cColor & color)
{
   const tVec3 & mins = box.GetMins();
   const tVec3 & maxs = box.GetMaxs();

   tVec3 verts[] =
   {
      tVec3(mins.x, mins.y, mins.z),
      tVec3(maxs.x, mins.y, mins.z),
      tVec3(maxs.x, mins.y, maxs.z),
      tVec3(mins.x, mins.y, maxs.z),

      tVec3(mins.x, maxs.y, mins.z),
      tVec3(maxs.x, maxs.y, mins.z),
      tVec3(maxs.x, maxs.y, maxs.z),
      tVec3(mins.x, maxs.y, maxs.z),

   };

   const uint16 indices[] =
   {
      0, 1,
      1, 2,
      2, 3,
      3, 0,

      4, 5,
      5, 6,
      6, 7,
      7, 4,

      0, 4,
      1, 5,
      2, 6,
      3, 7,
   };

   UseGlobal(Renderer);
   pRenderer->SetDiffuseColor(color.GetPointer());
   pRenderer->SetVertexFormat(g_wireFrameVertex, _countof(g_wireFrameVertex));
   pRenderer->SubmitVertices(&verts[0], _countof(verts));
   pRenderer->SetIndexFormat(kIF_16Bit);
   pRenderer->Render(kPT_Lines, indices, _countof(indices));
}

////////////////////////////////////////////////////////////////////////////////
