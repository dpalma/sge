////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "renderer.h"
#include "color.h"

#include "vec3.h"

#include <GL/glew.h>

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
 , m_nVertexElements(0)
 , m_vertexSize(0)
 , m_indexFormat(kIF_16Bit)
 , m_glIndexFormat(0)
 , m_nBlendMatrices(0)
{
   SetIndexFormat(kIF_16Bit);
}

////////////////////////////////////////

cRenderer::~cRenderer()
{
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

tResult cRenderer::SubmitVertices(void * pVertices, uint nVertices)
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

tResult cRenderer::SetBlendMatrices(const tMatrix4 * pMatrices, uint nMatrices)
{
   if (nMatrices >= kMaxBlendMatrices)
   {
      return E_INVALIDARG;
   }

   if (pMatrices == NULL || nMatrices == 0)
   {
#ifdef _DEBUG
      memset(m_blendMatrices, 0xBC, sizeof(m_blendMatrices));
#endif
      m_nBlendMatrices = 0;
   }
   else
   {
      memcpy(m_blendMatrices, pMatrices, nMatrices * sizeof(tMatrix4));
      m_nBlendMatrices = nMatrices;
   }

   return S_OK;
}

////////////////////////////////////////

tResult cRenderer::GetBlendMatrices(tMatrix4 * pMatrices, uint nMatrices) const
{
   if (pMatrices == NULL)
   {
      return E_POINTER;
   }

   if (nMatrices == 0)
   {
      return E_INVALIDARG;
   }

   uint nCopy = Min(nMatrices, m_nBlendMatrices);
   memcpy(pMatrices, m_blendMatrices, nCopy * sizeof(tMatrix4));
   return (nCopy < m_nBlendMatrices) ? S_FALSE : S_OK;
}

////////////////////////////////////////

tResult cRenderer::Render(ePrimitiveType primitive, void * pIndices, uint nIndices)
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

   glDisable(GL_DITHER);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);

   m_bInitialized = true;
   return S_OK;
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

////////////////////////////////////////////////////////////////////////////////
