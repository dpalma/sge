////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "rendererdx.h"
#include "render/renderfontapi.h"

#include "platform/sys.h"

#include "tech/axisalignedbox.h"
#include "tech/color.h"
#include "tech/readwriteapi.h"
#include "tech/resourceapi.h"
#include "tech/techhash.h"
#include "tech/techmath.h"
#include "tech/vec3.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>

#ifdef HAVE_CG
#include <Cg/cgD3D9.h>
#endif

#include <cstring>

#include "tech/dbgalloc.h" // must be last header

////////////////////////////////////////////////////////////////////////////////

extern const uint kVertexFVF;

extern tResult RenderTargetDXCreate(IDirect3DDevice9 * pD3dDevice, IRenderTarget * * ppRenderTarget);

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


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CG

// TODO: How to differentiate loading vertex vs. pixel shaders?
void * CgProgramLoad(IReader * pReader, void * typeParam)
{
   if (pReader == NULL)
   {
      return NULL;
   }

   cRendererDX * pRenderer = reinterpret_cast<cRendererDX *>(typeParam);

   // Must get the Cg context first
   CGcontext cgContext = pRenderer->m_cgContext;
   if (cgContext == NULL)
   {
      return NULL;
   }


   if (pRenderer->m_cgProfileVertex == CG_PROFILE_UNKNOWN)
   {
      pRenderer->m_cgProfileVertex = cgD3D9GetLatestVertexProfile();
      if (pRenderer->m_cgProfileVertex == CG_PROFILE_UNKNOWN)
      {
         return NULL;
      }
   }

   ulong length = 0;
   if (pReader->Seek(0, kSO_End) == S_OK
      && pReader->Tell(&length) == S_OK
      && pReader->Seek(0, kSO_Set) == S_OK)
   {
      char stackBuffer[1024];
      char * pBuffer = NULL;

      if (length >= 32768)
      {
         WarnMsg1("Sanity check failure loading Cg program %d bytes long\n", length);
         return NULL;
      }

      if (length < sizeof(stackBuffer))
      {
         pBuffer = stackBuffer;

         if (pReader->Read(pBuffer, length) == S_OK)
         {
            pBuffer[length] = 0;

            CGprogram program = cgCreateProgram(cgContext, CG_SOURCE, pBuffer, pRenderer->m_cgProfileVertex, NULL, NULL);
            if (program != NULL)
            {
               if (cgD3D9LoadProgram(program, CG_FALSE, 0) == S_OK)
               {
                  return program;
               }
               else
               {
                  cgDestroyProgram(program);
               }
            }
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


////////////////////////////////////////////////////////////////////////////////

void MainWindowDestroyCallback()
{
   UseGlobal(Renderer);
   if (!!pRenderer)
   {
      pRenderer->SetRenderTarget(NULL);
   }
}


////////////////////////////////////////////////////////////////////////////////

static tResult InitDirect3D9(HWND hWnd, HINSTANCE * phD3dLib, IDirect3D9 * * ppD3d, IDirect3DDevice9 * * ppDevice)
{
   if (!IsWindow(hWnd))
   {
      return E_INVALIDARG;
   }

   if (phD3dLib == NULL || ppD3d == NULL || ppDevice == NULL)
   {
      return E_POINTER;
   }

   HINSTANCE hD3dLib = LoadLibrary(_T("d3d9.dll"));
   if (hD3dLib == NULL)
   {
      return E_FAIL;
   }

   typedef IDirect3D9 * (WINAPI * tDirect3DCreate9Fn)(UINT);
   tDirect3DCreate9Fn pfnDirect3DCreate9 = reinterpret_cast<tDirect3DCreate9Fn>(
      GetProcAddress(hD3dLib, "Direct3DCreate9"));
   if (pfnDirect3DCreate9 == NULL)
   {
      FreeLibrary(hD3dLib);
      return E_FAIL;
   }

   cAutoIPtr<IDirect3D9> pD3d((*pfnDirect3DCreate9)(D3D_SDK_VERSION));
   if (!pD3d)
   {
      FreeLibrary(hD3dLib);
      return E_FAIL;
   }

   tResult result = E_FAIL;
   cAutoIPtr<IDirect3DDevice9> pD3dDevice;

   D3DDISPLAYMODE displayMode;
   if (pD3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode) == D3D_OK)
   {
      D3DPRESENT_PARAMETERS presentParams;
      memset(&presentParams, 0, sizeof(presentParams));
      presentParams.BackBufferCount = 1;
      presentParams.BackBufferFormat = displayMode.Format;
      presentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
      presentParams.Windowed = TRUE;
      presentParams.EnableAutoDepthStencil = TRUE;
      presentParams.AutoDepthStencilFormat = D3DFMT_D16;
      presentParams.hDeviceWindow = hWnd;
      presentParams.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL | D3DPRESENTFLAG_DEVICECLIP;

      result = pD3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
         D3DCREATE_SOFTWARE_VERTEXPROCESSING, &presentParams, &pD3dDevice);
      if (FAILED(result))
      {
         // Try reference device if failed to create hardware device
         result = pD3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING, &presentParams, &pD3dDevice);
      }
   }

   if (FAILED(result))
   {
      FreeLibrary(hD3dLib);
      ErrorMsg1("D3D error %x\n", result);
   }
   else
   {
      *phD3dLib = hD3dLib;
      *ppD3d = CTAddRef(pD3d);
      *ppDevice = CTAddRef(pD3dDevice);
   }

   return result;
}


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRendererDX
//

////////////////////////////////////////

cRendererDX::cRendererDX()
 : m_bInScene(false)
 , m_hWnd(NULL)
 , m_hD3dLib(NULL)
#ifdef HAVE_CG
 , m_cgContext(NULL)
 , m_oldCgErrorHandler(NULL)
 , m_pOldCgErrHandlerData(NULL)
 , m_cgProfileVertex(CG_PROFILE_UNKNOWN)
 , m_cgProfileFragment(CG_PROFILE_UNKNOWN)
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
   if (m_hD3dLib != NULL)
   {
      FreeLibrary(m_hD3dLib);
   }
}

////////////////////////////////////////

tResult cRendererDX::Init()
{
   SysSetDestroyCallback(MainWindowDestroyCallback);

   UseGlobal(ResourceManager);
   if (!!pResourceManager)
   {
#ifdef HAVE_CG
      if (pResourceManager->RegisterFormat(kRT_CgProgram, _T("cg"), CgProgramLoad, NULL, CgProgramUnload, this) != S_OK
//         || pResourceManager->RegisterFormat(kRT_CgEffect, _T("fx"), CgEffectLoad, NULL, CgEffectUnload) != S_OK
         )
      {
         return E_FAIL;
      }
#endif
   }

   return S_OK;
}

////////////////////////////////////////

tResult cRendererDX::Term()
{
   tFontMap::iterator iter = m_fontMap.begin();
   for (; iter != m_fontMap.end(); ++iter)
   {
      SafeRelease(iter->second);
   }
   m_fontMap.clear();

   SafeRelease(m_pTarget);

#ifdef HAVE_CG
   if (m_cgContext != NULL)
   {
      if (m_oldCgErrorHandler != NULL)
      {
         cgSetErrorHandler(m_oldCgErrorHandler, m_pOldCgErrHandlerData);
         m_oldCgErrorHandler = NULL;
         m_pOldCgErrHandlerData = NULL;
      }

      cgDestroyContext(m_cgContext);
      m_cgContext = NULL;

      m_cgProfileVertex = CG_PROFILE_UNKNOWN;
      m_cgProfileFragment = CG_PROFILE_UNKNOWN;
   }
#endif

   SafeRelease(m_pD3dDevice);
   SafeRelease(m_pD3d);

   m_hWnd = NULL;

   return S_OK;
}

////////////////////////////////////////

///////////////////////////////////////

tResult cRendererDX::CreateRenderTarget(HWND hWnd, IRenderTarget * * ppRenderTarget)
{
   if (!IsWindow(hWnd))
   {
      return E_INVALIDARG;
   }

   if (m_hWnd == NULL || m_hD3dLib == NULL || !m_pD3d || !m_pD3dDevice)
   {
      if (InitDirect3D9(hWnd, &m_hD3dLib, &m_pD3d, &m_pD3dDevice) != D3D_OK)
      {
         return E_FAIL;
      }

      m_hWnd = hWnd;

#ifdef HAVE_CG
      if (cgD3D9SetDevice(m_pD3dDevice) != S_OK)
      {
         ErrorMsg("cgD3D9SetDevice failed\n");
         return E_FAIL;
      }

      if (m_cgContext == NULL)
      {
         m_cgContext = cgCreateContext();

         Assert(m_oldCgErrorHandler == NULL && m_pOldCgErrHandlerData == NULL);
         m_oldCgErrorHandler = cgGetErrorHandler(&m_pOldCgErrHandlerData);
         cgSetErrorHandler(CgErrorHandler, static_cast<void*>(this));

         Assert(m_cgProfileVertex == CG_PROFILE_UNKNOWN);
         m_cgProfileVertex = cgD3D9GetLatestVertexProfile();

         Assert(m_cgProfileFragment == CG_PROFILE_UNKNOWN);
         m_cgProfileFragment = cgD3D9GetLatestPixelProfile();
      }
#endif

      if (Render2DCreateDX(m_pD3dDevice, &m_pRender2D) != S_OK)
      {
         return E_FAIL;
      }
   }

   tResult result = RenderTargetDXCreate(m_pD3dDevice, ppRenderTarget);

   return result;
}

///////////////////////////////////////

tResult cRendererDX::CreateRenderTarget(Display * display, Window window, IRenderTarget * * ppRenderTarget)
{
   ErrorMsg("POSIX overload of IRenderer::CreateRenderTarget not supported\n");
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cRendererDX::GetRenderTarget(IRenderTarget * * ppRenderTarget)
{
   return m_pTarget.GetPointer(ppRenderTarget);
}

////////////////////////////////////////

tResult cRendererDX::SetRenderTarget(IRenderTarget * pRenderTarget)
{
   SafeRelease(m_pTarget);
   m_pTarget = CTAddRef(pRenderTarget);
   return S_OK;
}

////////////////////////////////////////

static const long g_d3dFillModes[] =
{
   D3DFILL_POINT,       // kFM_Point
   D3DFILL_WIREFRAME,   // kFM_Wireframe
   D3DFILL_SOLID,       // kFM_Solid
};

tResult cRendererDX::SetRenderState(eRenderState state, ulong value)
{
   if (!m_pD3dDevice)
   {
      return E_FAIL;
   }

   tResult result = E_FAIL;

   switch (state)
   {
      case kRS_AlphaTestEnable:
      {
         result = m_pD3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, value);
         break;
      }

      case kRS_AlphaBlendEnable:
      {
         result = m_pD3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, value);
         break;
      }

      case kRS_FillMode:
      {
         if (value != kFM_Point && value != kFM_Wireframe && value != kFM_Solid)
         {
            return E_INVALIDARG;
         }
         result = m_pD3dDevice->SetRenderState(D3DRS_FILLMODE, g_d3dFillModes[value]);
         break;
      }

      default:
      {
         result = E_INVALIDARG;
         break;
      }
   }

   return result;
}

////////////////////////////////////////

tResult cRendererDX::GetRenderState(eRenderState state, ulong * pValue)
{
   if (pValue == NULL)
   {
      return E_POINTER;
   }

   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cRendererDX::BeginScene()
{
   if (!!m_pD3dDevice)
   {
      AssertMsg(!m_bInScene, "Cannot nest BeginScene/EndScene calls");
      if (!m_bInScene)
      {
         if (m_pD3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1, 0) == D3D_OK
            && m_pD3dDevice->BeginScene() == D3D_OK)
         {
            m_bInScene = true;
            return S_OK;
         }
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

tResult cRendererDX::SetTexture(uint textureUnit, const void * texture)
{
   return E_NOTIMPL;
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

tResult cRendererDX::Render(ePrimitiveType primitive, uint startIndex, uint nIndices)
{
   //switch (primitive)
   //{
   //   case kPT_Lines:
   //   {
   //      nIndices = nPrimitives * 2;
   //      break;
   //   }
   //   case kPT_LineStrip:
   //   {
   //      nIndices = nPrimitives + 1;
   //      break;
   //   }
   //   case kPT_Triangles:
   //   {
   //      nIndices = nPrimitives * 3;
   //      break;
   //   }
   //   case kPT_TriangleStrip:
   //   {
   //      nIndices = nPrimitives + 2;
   //      break;
   //   }
   //   case kPT_TriangleFan:
   //   {
   //      nIndices = nPrimitives + 2;
   //      break;
   //   }
   //}

   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cRendererDX::RenderIndexed(ePrimitiveType primitive, const void * pIndices, uint nIndices)
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

tResult cRendererDX::PushMatrix(const float matrix[16])
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cRendererDX::PopMatrix()
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cRendererDX::GetCamera(IRenderCamera * * ppCamera)
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cRendererDX::SetCamera(IRenderCamera * pCamera)
{
   return E_NOTIMPL;
}

////////////////////////////////////////

#ifdef HAVE_CG
void cRendererDX::CgErrorHandler(CGcontext cgContext, CGerror cgError, void * pData)
{
   if (cgError)
   {
      const char * pszCgError = cgGetErrorString(cgError);
      ErrorMsgIf1(pszCgError != NULL, "%s\n", pszCgError);
      const char * pszListing = cgGetLastListing(cgContext);
      if (pszListing != NULL)
      {
         ErrorMsg1("   %s\n", pszListing);
      }
   }
}
#endif

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


////////////////////////////////////////////////////////////////////////////////
