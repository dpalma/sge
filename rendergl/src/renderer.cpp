////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "renderer.h"

#include "axisalignedbox.h"
#include "color.h"
#include "resourceapi.h"
#include "techmath.h"
#include "vec3.h"

#include <GL/glew.h>

#ifdef HAVE_CG
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include <CgFX/ICgFX.h>
#include <CgFX/ICgFXEffect.h>
#endif

#include <cstring>

#include "dbgalloc.h" // must be last header


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


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRenderer
//

////////////////////////////////////////

cRenderer::cRenderer()
 : m_bInitialized(false)
 , m_bInScene(false)
#ifdef HAVE_CG
 , m_cgContext(NULL)
 , m_oldCgErrorCallback(NULL)
 , m_cgProfile(CG_PROFILE_UNKNOWN)
#endif
 , m_nVertexElements(0)
 , m_vertexSize(0)
 , m_indexFormat(kIF_16Bit)
 , m_glIndexFormat(0)
{
   SetIndexFormat(kIF_16Bit);
}

////////////////////////////////////////

cRenderer::~cRenderer()
{
}

////////////////////////////////////////

tResult cRenderer::Init()
{
   return S_OK;
}

////////////////////////////////////////

tResult cRenderer::Term()
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

   return S_OK;
}

////////////////////////////////////////

tResult cRenderer::BeginScene()
{
   if (SUCCEEDED(Initialize()))
   {
      AssertMsg(!m_bInScene, "Cannot nest BeginScene/EndScene calls");
      if (!m_bInScene)
      {
         m_bInScene = true;
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
         return S_OK;
      }
   }
   return E_FAIL;
}

////////////////////////////////////////

tResult cRenderer::EndScene()
{
   if (m_bInScene)
   {
      m_bInScene = false;
      return S_OK;
   }
   return E_FAIL;
}

////////////////////////////////////////

extern tResult GlTextureCreate(IImage * pImage, uint * pTexId);

tResult cRenderer::CreateTexture(IImage * pImage, bool bAutoGenMipMaps, void * * ppTexture)
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

tResult cRenderer::SetVertexFormat(const sVertexElement * pVertexElements, uint nVertexElements)
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

tResult cRenderer::SetIndexFormat(eIndexFormat indexFormat)
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

tResult cRenderer::SubmitVertices(const void * pVertices, uint nVertices)
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

tResult cRenderer::SetDiffuseColor(const float diffuse[4])
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

tResult cRenderer::SetTexture(uint textureUnit, const tChar * pszTexture)
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

tResult cRenderer::Render(ePrimitiveType primitive, const void * pIndices, uint nIndices)
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

tResult cRenderer::Initialize()
{
   if (m_bInitialized)
   {
      return S_FALSE;
   }

   if (glewInit() != GLEW_OK)
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

   glDisable(GL_DITHER);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);

   m_bInitialized = true;
   return S_OK;
}

////////////////////////////////////////

void cRenderer::CgErrorCallback()
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
   cAutoIPtr<IRenderer> p(static_cast<IRenderer*>(new cRenderer));
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

extern tResult GlTextureResourceRegister(); // gltexture.cpp

tResult RendererResourceRegister()
{
   UseGlobal(ResourceManager);
   if (!!pResourceManager)
   {
      if (GlTextureResourceRegister() == S_OK
#ifdef HAVE_CG
         && pResourceManager->RegisterFormat(kRT_CgProgram, _T("cg"), CgProgramLoad, NULL, CgProgramUnload) == S_OK
//         && pResourceManager->RegisterFormat(kRT_CgEffect, _T("fx"), CgEffectLoad, NULL, CgEffectUnload) == S_OK
#endif
         )
      {
         return S_OK;
      }
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
