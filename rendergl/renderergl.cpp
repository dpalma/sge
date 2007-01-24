////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "renderergl.h"
#include "render/renderfontapi.h"

#include "platform/sys.h"

#include "tech/axisalignedbox.h"
#include "tech/color.h"
#include "tech/configapi.h"
#include "tech/imageapi.h"
#include "tech/matrix4.h"
#include "tech/ray.h"
#include "tech/readwriteapi.h"
#include "tech/resourceapi.h"
#include "tech/techhash.h"
#include "tech/techmath.h"
#include "tech/vec3.h"

#include <GL/glew.h>

#ifdef _WIN32
#include <GL/wglew.h>
#else
#include <GL/glxew.h>
#endif

#ifdef HAVE_CG
#include <Cg/cgGL.h>
#endif

#include <cstring>

#include "tech/dbgalloc.h" // must be last header

////////////////////////////////////////////////////////////////////////////////

extern tResult RenderFontCreateGL(const tChar * pszFont, int pointSize, uint flags, IRenderFont * * ppFont);
extern tResult RenderFontCreateFTGL(const tChar * pszFont, int fontPointSize, uint flags, IRenderFont * * ppFont);

extern tResult Render2DCreateGL(IRender2D * * ppRender2D);

extern tResult GlTextureCreate(IImage * pImage, uint * pTexId);
extern tResult GlTextureCreateMipMapped(IImage * pImage, uint * pTexId);

////////////////////////////////////////////////////////////////////////////////

static bool g_bHaveValidGlContext = false; // HACK: This variable's very existence is a hack

////////////////////////////////////////////////////////////////////////////////

inline GLenum GetGlType(eVertexElementType type)
{
   static const GLenum glTypeTable[] =
   {
      GL_FLOAT,         // kVET_Float1
      GL_FLOAT,         // kVET_Float2
      GL_FLOAT,         // kVET_Float3
      GL_FLOAT,         // kVET_Float4
      GL_UNSIGNED_BYTE, // kVET_Color
      GL_UNSIGNED_BYTE, // kVET_UnsignedByte4
      GL_SHORT,         // kVET_Short2
      GL_SHORT,         // kVET_Short4
   };
   Assert((int)type < _countof(glTypeTable));
   return glTypeTable[type];
}

inline GLint CountComponents(eVertexElementType type)
{
   static const GLint countComponentsTable[] =
   {
      1, // kVET_Float1
      2, // kVET_Float2
      3, // kVET_Float3
      4, // kVET_Float4
      4, // kVET_Color
      4, // kVET_UnsignedByte4
      2, // kVET_Short2
      4, // kVET_Short4
   };
   Assert((int)type < _countof(countComponentsTable));
   return countComponentsTable[type];
}

static void SubmitVertexData(const sVertexElement * elements,
                             uint nElements, uint vertexSize,
                             const byte * pVertexData)
{
   Assert(elements != NULL);
   Assert(nElements > 0);
   Assert(vertexSize > 0);

   /*
   GLenum ilFormat = 0;
   uint ilStride = 0;

   eGlVertexSubmission glvs = GlGetVertexSubmission(elements, nElements, &ilFormat, &ilStride);

   if (glvs == kGlInterleavedArrays)
   {
      glInterleavedArrays(ilFormat, ilStride, pVertexData);
   }
   else if (glvs == kGlVertexArrays)
   */
   {
      glDisableClientState(GL_EDGE_FLAG_ARRAY);
      glDisableClientState(GL_INDEX_ARRAY);
      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_NORMAL_ARRAY);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glDisableClientState(GL_COLOR_ARRAY);

      for (uint i = 0; i < nElements; i++)
      {
         GLint nComponents = CountComponents(elements[i].type);
         GLenum type = GetGlType(elements[i].type);

         switch (elements[i].usage)
         {
            case kVEU_Position:
            {
               glEnableClientState(GL_VERTEX_ARRAY);
               glVertexPointer(nComponents, type, vertexSize, pVertexData + elements[i].offset);
               break;
            }

            case kVEU_Normal:
            {
               glEnableClientState(GL_NORMAL_ARRAY);
               glNormalPointer(type, vertexSize, pVertexData + elements[i].offset);
               break;
            }

            case kVEU_TexCoord:
            {
               glEnableClientState(GL_TEXTURE_COORD_ARRAY);
               glClientActiveTextureARB(GL_TEXTURE0_ARB + elements[i].usageIndex);
               glTexCoordPointer(nComponents, type, vertexSize, pVertexData + elements[i].offset);
               break;
            }

            case kVEU_Color:
            {
               glEnableClientState(GL_COLOR_ARRAY);
               glColorPointer(nComponents, type, vertexSize, pVertexData + elements[i].offset);
               break;
            }
         }
      }
   }
}

////////////////////////////////////////////////////////////////////////////////

// Ensure the vertex element types can be used as lookup table indices
AssertAtCompileTime(kVET_Float1 == 0);
AssertAtCompileTime(kVET_Float2 == 1);
AssertAtCompileTime(kVET_Float3 == 2);
AssertAtCompileTime(kVET_Float4 == 3);
AssertAtCompileTime(kVET_Color == 4);
AssertAtCompileTime(kVET_UnsignedByte4 == 5);
AssertAtCompileTime(kVET_Short2 == 6);
AssertAtCompileTime(kVET_Short4 == 7);

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

inline GLenum GetGlPrimitive(ePrimitiveType primitive)
{
   static const GLenum glPrimitiveTable[] =
   {
      GL_LINES,               // kRP_Lines,
      GL_LINE_STRIP,          // kRP_LineStrip,
      GL_TRIANGLES,           // kRP_Triangles
      GL_TRIANGLE_STRIP,      // kRP_TriangleStrip
      GL_TRIANGLE_FAN,        // kRP_TriangleFan
   };
   Assert((uint)primitive < _countof(glPrimitiveTable));
   return glPrimitiveTable[primitive];
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

void * CgProgramLoad(IReader * pReader, void * typeParam)
{
   if (pReader == NULL)
   {
      return NULL;
   }

   cRendererGL * pRenderer = reinterpret_cast<cRendererGL *>(typeParam);

   // Must get the Cg context first
   CGcontext cgContext = pRenderer->m_cgContext;
   if (cgContext == NULL)
   {
      return NULL;
   }

   CGprofile cgProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
   if (cgProfile == CG_PROFILE_UNKNOWN)
   {
      return NULL;
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

            CGprogram program = cgCreateProgram(cgContext, CG_SOURCE, pBuffer, cgProfile, NULL, NULL);
            if (program != NULL)
            {
               cgGLLoadProgram(program);
               return program;
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


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CG

void * CgEffectLoad(IReader * pReader, void * typeParam)
{
   if (pReader == NULL)
   {
      return NULL;
   }

   cRendererGL * pRenderer = reinterpret_cast<cRendererGL *>(typeParam);

   // Must get the Cg context first
   CGcontext cgContext = pRenderer->m_cgContext;
   if (cgContext == NULL)
   {
      return NULL;
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

            CGeffect effect = cgCreateEffect(cgContext, pBuffer, NULL);
            if (effect != NULL)
            {
               return effect;
            }
         }
      }
   }

   return NULL;
}

void CgEffectUnload(void * pData)
{
   CGeffect effect = reinterpret_cast<CGeffect>(pData);
   if (effect != NULL)
   {
      cgDestroyEffect(effect);
   }
}

#endif // HAVE_CG


///////////////////////////////////////////////////////////////////////////////

void * GlTextureFromImage(void * pData, int dataLength, void * param)
{
   IImage * pImage = reinterpret_cast<IImage*>(pData);

   uint texId;
   if (GlTextureCreateMipMapped(pImage, &texId) == S_OK)
   {
      return reinterpret_cast<void*>(texId);
   }

   return NULL;
}

void GlTextureUnload(void * pData)
{
   if (g_bHaveValidGlContext)
   {
      uint tex = reinterpret_cast<uint>(pData);
      if (glIsTexture(tex))
      {
         glDeleteTextures(1, &tex);
      }
   }
}


////////////////////////////////////////////////////////////////////////////////

void MainWindowDestroyCallback()
{
   UseGlobal(Renderer);
   if (!!pRenderer)
   {
      pRenderer->DestroyContext();
   }
}


///////////////////////////////////////////////////////////////////////////////
// Create a default OpenGL context using the Win32 function ChoosePixelFormat

#ifdef _WIN32
static int CreateDefaultContext(HWND hWnd, int * pBpp, HDC * phDC, HGLRC * phGLRC)
{
   Assert(IsWindow(hWnd));

   *phDC = GetDC(hWnd);
   if (!*phDC)
   {
      return 0;
   }

   if (*pBpp == 0)
   {
      *pBpp = GetDeviceCaps(*phDC, BITSPIXEL);
   }

   PIXELFORMATDESCRIPTOR pfd;
   memset(&pfd, 0, sizeof(pfd));
   pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
   pfd.nVersion = 1;
   pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
   pfd.iPixelType = PFD_TYPE_RGBA;
   pfd.cColorBits = *pBpp;
   pfd.cDepthBits = *pBpp;
   pfd.cStencilBits = *pBpp;
   pfd.dwLayerMask = PFD_MAIN_PLANE;

   int pixelFormat = ChoosePixelFormat(*phDC, &pfd);
   while ((pixelFormat == 0) && (pfd.cStencilBits > 0))
   {
      pfd.cStencilBits /= 2;
      pixelFormat = ChoosePixelFormat(*phDC, &pfd);
   }

   if (pixelFormat == 0)
   {
      ErrorMsg("Unable to find a suitable pixel format\n");
      ReleaseDC(hWnd, *phDC);
      *phDC = NULL;
      *phGLRC = NULL;
      return 0;
   }

   if (!SetPixelFormat(*phDC, pixelFormat, &pfd))
   {
      ErrorMsg1("SetPixelFormat failed with error %d\n", GetLastError());
      ReleaseDC(hWnd, *phDC);
      *phDC = NULL;
      *phGLRC = NULL;
      return 0;
   }

   *phGLRC = wglCreateContext(*phDC);
   if ((*phGLRC) == NULL)
   {
      ErrorMsg1("wglCreateContext failed with error 0x%04x\n", glGetError());
      ReleaseDC(hWnd, *phDC);
      *phDC = NULL;
      *phGLRC = NULL;
      return 0;
   }

   return pixelFormat;
}
#endif // _WIN32


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRendererGL
//

////////////////////////////////////////

BEGIN_CONSTRAINTS(cRendererGL)
   AFTER_GUID(IID_IResourceManager)
END_CONSTRAINTS()

////////////////////////////////////////

cRendererGL::cRendererGL()
#ifdef _WIN32
 : m_hWnd(NULL)
 , m_hDC(NULL)
 , m_hGLRC(NULL)
#else
 : m_display(NULL)
 , m_context(NULL)
#endif
 , m_bInScene(false)
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
 , m_glIndexFormat(0)
{
   SetIndexFormat(kIF_16Bit);
}

////////////////////////////////////////

cRendererGL::~cRendererGL()
{
}

////////////////////////////////////////

tResult cRendererGL::Init()
{
   SysSetDestroyCallback(MainWindowDestroyCallback);

   if (Render2DCreateGL(&m_pRender2D) != S_OK)
   {
      return E_FAIL;
   }

   if (RenderCameraCreate(&m_pCamera) != S_OK)
   {
      return E_FAIL;
   }

   UseGlobal(ResourceManager);
   if (!!pResourceManager)
   {
      if (pResourceManager->RegisterFormat(kRT_GlTexture, kRT_Image, NULL, NULL, GlTextureFromImage, GlTextureUnload) != S_OK)
      {
         return E_FAIL;
      }

#ifdef HAVE_CG
      if (pResourceManager->RegisterFormat(kRT_CgProgram, _T("cg"), CgProgramLoad, NULL, CgProgramUnload, this) != S_OK
         || pResourceManager->RegisterFormat(kRT_CgEffect, _T("fx"), CgEffectLoad, NULL, CgEffectUnload, this) != S_OK)
      {
         return E_FAIL;
      }
#endif
   }

   return S_OK;
}

////////////////////////////////////////

tResult cRendererGL::Term()
{
   tFontMap::iterator iter = m_fontMap.begin();
   for (; iter != m_fontMap.end(); ++iter)
   {
      SafeRelease(iter->second);
   }
   m_fontMap.clear();

   DestroyContext();

   return S_OK;
}

////////////////////////////////////////

tResult cRendererGL::CreateContext()
{
#ifdef _WIN32
   return CreateContext(SysGetMainWindow());
#else
   return CreateContext(SysGetDisplay(), SysGetMainWindow());
#endif
}

////////////////////////////////////////

tResult cRendererGL::CreateContext(HWND hWnd)
{
#ifdef _WIN32
   if (!IsWindow(hWnd))
   {
      return E_INVALIDARG;
   }

   if ((m_hWnd != NULL) || (m_hDC != NULL) || (m_hGLRC != NULL))
   {
      WarnMsg("OpenGL renderer context already created\n");
      return S_FALSE;
   }

   int bpp = 0;
   if (CreateDefaultContext(hWnd, &bpp, &m_hDC, &m_hGLRC) == 0)
   {
      ErrorMsg("An error occurred creating the GL context\n");
      return E_FAIL;
   }

   m_hWnd = hWnd;

   if (!wglMakeCurrent(m_hDC, m_hGLRC))
   {
      GLenum glError = glGetError();
      ErrorMsg1("wglMakeCurrent failed with error 0x%04x\n", glError);
      DestroyContext();
      return E_FAIL;
   }

   if (glewInit() != GLEW_OK)
   {
      ErrorMsg("GLEW library failed to initialize\n");
      DestroyContext();
      return E_FAIL;
   }

#ifdef HAVE_CG
   if (m_cgContext == NULL)
   {
      m_cgContext = cgCreateContext();

      Assert(m_oldCgErrorHandler == NULL && m_pOldCgErrHandlerData == NULL);
      m_oldCgErrorHandler = cgGetErrorHandler(&m_pOldCgErrHandlerData);
      cgSetErrorHandler(CgErrorHandler, static_cast<void*>(this));

      Assert(m_cgProfileVertex == CG_PROFILE_UNKNOWN);
      m_cgProfileVertex = cgGLGetLatestProfile(CG_GL_VERTEX);

      Assert(m_cgProfileFragment == CG_PROFILE_UNKNOWN);
      m_cgProfileFragment = cgGLGetLatestProfile(CG_GL_FRAGMENT);
   }
#endif

   if (ConfigIsTrue(_T("disable_vsync")))
   {
      if (wglewIsSupported("WGL_EXT_swap_control"))
      {
         int swapInterval = wglGetSwapIntervalEXT();
         wglSwapIntervalEXT(0);
         InfoMsg1("Changed swap interval from %d to 0\n", swapInterval);
      }
      else
      {
         WarnMsg("WGL_EXT_swap_control extension not supported\n");
      }
   }

   g_bHaveValidGlContext = true;

   glDisable(GL_DITHER);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);

   return S_OK;
#else
   return E_NOTIMPL;
#endif
}

////////////////////////////////////////

tResult cRendererGL::CreateContext(Display * display, Window window)
{
#ifdef _WIN32
   return E_NOTIMPL;
#else
   if (display == NULL)
   {
      return E_POINTER;
   }

   if (window == 0)
   {
      return E_INVALIDARG;
   }

   if (m_context != NULL)
   {
      WarnMsg("GL renderer context already created\n");
      return S_FALSE;
   }

   XWindowAttributes attr = {0};
   XGetWindowAttributes(display, window, &attr);

   if (attr.visual == NULL)
   {
      ErrorMsg("Unable to determine visual for window\n");
      return E_FAIL;
   }

   XVisualInfo vinfoTemplate = {0};
   vinfoTemplate.visual = attr.visual;
   vinfoTemplate.visualid = XVisualIDFromVisual(attr.visual);

   int nVisualInfos = 0;
   XVisualInfo * pVisualInfo = XGetVisualInfo(display, VisualIDMask, &vinfoTemplate, &nVisualInfos);

   if (pVisualInfo == NULL)
   {
      return E_FAIL;
   }

   m_context = glXCreateContext(display, pVisualInfo, None, GL_TRUE);

   XFree(pVisualInfo);

   if (m_context == NULL)
   {
      ErrorMsg("Could not create glX context\n");
      return E_FAIL;
   }

   m_display = display;

   glXMakeCurrent(display, window, m_context);

   if (glewInit() != GLEW_OK)
   {
      ErrorMsg("GLEW library failed to initialize\n");
      DestroyContext();
      return E_FAIL;
   }

   g_bHaveValidGlContext = true;

   glDisable(GL_DITHER);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);

   return S_OK;
#endif
}

////////////////////////////////////////

tResult cRendererGL::DestroyContext()
{
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

#ifdef _WIN32
   if (m_hDC != NULL)
   {
      wglMakeCurrent(m_hDC, NULL);
      ReleaseDC(m_hWnd, m_hDC);
      m_hDC = NULL;
   }

   if (m_hGLRC != NULL)
   {
      wglDeleteContext(m_hGLRC);
      m_hGLRC = NULL;
   }

   m_hWnd = NULL;
#else
   if (m_context != NULL)
   {
      glXDestroyContext(m_display, m_context);
      m_display = NULL;
      m_context = NULL;
   }
#endif

   g_bHaveValidGlContext = false;

   return S_OK;
}

////////////////////////////////////////

static const GLuint g_glPolygonModes[] =
{
   GL_POINT,   // kFM_Point
   GL_LINE,    // kFM_Wireframe
   GL_FILL,    // kFM_Solid
};

tResult cRendererGL::SetRenderState(eRenderState state, ulong value)
{
   bool bFoundRenderState = true;

   switch (state)
   {
      case kRS_AlphaTestEnable:
      {
         if (value)
         {
            glEnable(GL_ALPHA);
         }
         else
         {
            glDisable(GL_ALPHA);
         }
         break;
      }

      case kRS_AlphaBlendEnable:
      {
         if (value)
         {
            glEnable(GL_BLEND);
         }
         else
         {
            glDisable(GL_BLEND);
         }
         break;
      }

      case kRS_FillMode:
      {
         if (value != kFM_Point && value != kFM_Wireframe && value != kFM_Solid)
         {
            return E_INVALIDARG;
         }
         glPolygonMode(GL_FRONT_AND_BACK, g_glPolygonModes[value]);
         break;
      }

      default:
      {
         bFoundRenderState = false;
         break;
      }
   }

   if (bFoundRenderState)
   {
      return (glGetError() == GL_NO_ERROR) ? S_OK : E_FAIL;
   }
   else
   {
      return E_INVALIDARG;
   }
}

////////////////////////////////////////

tResult cRendererGL::GetRenderState(eRenderState state, ulong * pValue)
{
   if (pValue == NULL)
   {
      return E_POINTER;
   }

   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cRendererGL::BeginScene()
{
   if (!m_pCamera)
   {
      return E_FAIL;
   }

   AssertMsg(!m_bInScene, "Cannot nest BeginScene/EndScene calls");
   if (!m_bInScene)
   {
      m_bInScene = true;
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

      glMatrixMode(GL_PROJECTION);
      glLoadMatrixf(m_pCamera->GetProjectionMatrix());

      glMatrixMode(GL_MODELVIEW);
      glLoadMatrixf(m_pCamera->GetViewMatrix());

      return S_OK;
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cRendererGL::EndScene()
{
   if (m_bInScene)
   {
      m_bInScene = false;
#ifdef _WIN32
      if (m_hDC != NULL)
      {
         Verify(SwapBuffers(m_hDC));
      }
#else
      glXSwapBuffers(SysGetDisplay(), SysGetMainWindow());
#endif
      return S_OK;
   }
   return E_FAIL;
}

////////////////////////////////////////

tResult cRendererGL::CreateTexture(IImage * pImage, bool bAutoGenMipMaps, void * * ppTexture)
{
   if (pImage == NULL || ppTexture == NULL)
   {
      return E_POINTER;
   }

   uint textureId = 0;
   tResult result = bAutoGenMipMaps
      ? GlTextureCreateMipMapped(pImage, &textureId)
      : GlTextureCreate(pImage, &textureId);

   if (result == S_OK)
   {
      *ppTexture = reinterpret_cast<void*>(textureId);
   }

   return result;
}

////////////////////////////////////////

tResult cRendererGL::SetVertexFormat(const sVertexElement * pVertexElements, uint nVertexElements)
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

tResult cRendererGL::SetIndexFormat(eIndexFormat indexFormat)
{
   m_indexFormat = indexFormat;
   if (indexFormat == kIF_16Bit)
   {
      m_glIndexFormat = GL_UNSIGNED_SHORT;
   }
   else if (indexFormat == kIF_32Bit)
   {
      m_glIndexFormat = GL_UNSIGNED_INT;
   }
   else
   {
      return E_FAIL;
   }
   return S_OK;
}

////////////////////////////////////////

tResult cRendererGL::SubmitVertices(const void * pVertices, uint nVertices)
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

   SubmitVertexData(m_vertexElements, m_nVertexElements, m_vertexSize,
      static_cast<const byte *>(pVertices));
   return S_OK;
}

////////////////////////////////////////

tResult cRendererGL::SetDiffuseColor(const float diffuse[4])
{
   if (diffuse == NULL)
   {
      return E_POINTER;
   }
   glEnable(GL_COLOR_MATERIAL);
   glColorMaterial(GL_FRONT, GL_DIFFUSE);
   glColor4fv(diffuse);
   return S_OK;
}

////////////////////////////////////////

tResult cRendererGL::SetTexture(uint textureUnit, const void * texture)
{
   if (textureUnit >= 8)
   {
      return E_INVALIDARG;
   }
   uint textureId = reinterpret_cast<uint>(texture);
   glActiveTextureARB(GL_TEXTURE0 + textureUnit);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, textureId);
   return S_OK;
}

////////////////////////////////////////

tResult cRendererGL::SetTexture(uint textureUnit, const tChar * pszTexture)
{
   if (textureUnit >= 8)
   {
      return E_INVALIDARG;
   }
   if (pszTexture == NULL)
   {
      return E_POINTER;
   }
   uint textureId;
   UseGlobal(ResourceManager);
   if (pResourceManager->Load(pszTexture, kRT_GlTexture, NULL, (void**)&textureId) == S_OK)
   {
      glActiveTextureARB(GL_TEXTURE0 + textureUnit);
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, textureId);
      return S_OK;
   }
   return E_FAIL;
}

////////////////////////////////////////

tResult cRendererGL::Render(ePrimitiveType primitive, const void * pIndices, uint nIndices)
{
   if (pIndices == NULL)
   {
      return E_POINTER;
   }

   if (nIndices == 0)
   {
      return E_INVALIDARG;
   }

   if (!m_bInScene)
   {
      return E_FAIL;
   }

   glDrawElements(GetGlPrimitive(primitive), nIndices, m_glIndexFormat, pIndices);
   return S_OK;
}

////////////////////////////////////////

#undef CreateFont
tResult cRendererGL::CreateFont(const tChar * pszFont, int fontPointSize, uint flags, IRenderFont * * ppFont)
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

   tResult result = RenderFontCreateFTGL(pszFont, fontPointSize, flags, ppFont);
   if (result != S_OK)
   {
      result = RenderFontCreateGL(pszFont, fontPointSize, flags, ppFont);
   }

   if (result == S_OK)
   {
      m_fontMap[h] = CTAddRef(*ppFont);
   }

   return result;
}

////////////////////////////////////////

tResult cRendererGL::Begin2D(int width, int height, IRender2D * * ppRender2D)
{
   if (ppRender2D == NULL)
   {
      return E_POINTER;
   }

   glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);

   glDisable(GL_DEPTH_TEST);
   glDisable(GL_LIGHTING);
   glDisable(GL_CULL_FACE);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, static_cast<GLdouble>(width), static_cast<GLdouble>(height), 0, -99999, 99999);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   return m_pRender2D.GetPointer(ppRender2D);
}

////////////////////////////////////////

tResult cRendererGL::End2D()
{
   glPopAttrib();
   return S_OK;
}

////////////////////////////////////////

tResult cRendererGL::PushMatrix(const float matrix[16])
{
   if (matrix == NULL)
   {
      return E_POINTER;
   }

   glPushMatrix();
   glMultMatrixf(matrix);
   return S_OK;
}

////////////////////////////////////////

tResult cRendererGL::PopMatrix()
{
   glPopMatrix();
#ifdef _DEBUG
   return (glGetError() == GL_NO_ERROR) ? S_OK : E_FAIL;
#else
   return S_OK;
#endif
}

///////////////////////////////////////

tResult cRendererGL::GetCamera(IRenderCamera * * ppCamera)
{
   return m_pCamera.GetPointer(ppCamera);
}

////////////////////////////////////////

tResult cRendererGL::SetCamera(IRenderCamera * pCamera)
{
   SafeRelease(m_pCamera);
   m_pCamera = CTAddRef(pCamera);
   return S_OK;
}

////////////////////////////////////////

#ifdef HAVE_CG
void cRendererGL::CgErrorHandler(CGcontext cgContext, CGerror cgError, void * pData)
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
   cAutoIPtr<IRenderer> p(static_cast<IRenderer*>(new cRendererGL));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IRenderer, p);
}


////////////////////////////////////////////////////////////////////////////////
