///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDERAPI_H
#define INCLUDED_RENDERAPI_H

#include "renderdll.h"
#include "comtools.h"

#include "color.h"
#include "rect.h"

#ifdef _MSC_VER
#pragma once
#endif

template <typename T> class cAxisAlignedBox;
typedef class cAxisAlignedBox<float> tAxisAlignedBox;

F_DECLARE_INTERFACE(IImage);

F_DECLARE_INTERFACE(IRenderer);
F_DECLARE_INTERFACE(IRender2D);
F_DECLARE_INTERFACE(IRenderFont);

///////////////////////////////////////////////////////////////////////////////

#define kRT_CgProgram   _T("CgProgram")
#define kRT_CgEffect    _T("CgEffect")
#define kRT_GlTexture   _T("GlTexture")

///////////////////////////////////////////////////////////////////////////////

RENDER_API tResult GlTextureCreateMipMapped(IImage * pImage, uint * pTexId);

///////////////////////////////////////////////////////////////////////////////

enum eIndexFormat
{
   kIF_16Bit,
   kIF_32Bit,
};

enum eVertexElementUsage
{
   kVEU_Position,
   kVEU_Normal,
   kVEU_Color,
   kVEU_TexCoord,
   kVEU_Weight,
   kVEU_Index,
};

enum eVertexElementType
{
   kVET_Float1,
   kVET_Float2,
   kVET_Float3,
   kVET_Float4,
   kVET_Color,
   kVET_UnsignedByte4,
   kVET_Short2,
   kVET_Short4,
};

struct sVertexElement
{
   eVertexElementUsage usage;
   eVertexElementType type;
   byte usageIndex;
   uint offset;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IRenderer
//

enum ePrimitiveType
{
   kPT_Lines,
   kPT_LineStrip,
   kPT_Triangles,
   kPT_TriangleStrip,
   kPT_TriangleFan
};

typedef enum ePrimitiveType tPrimitiveType;

interface IRenderer : IUnknown
{
   virtual tResult BeginScene() = 0;
   virtual tResult EndScene() = 0;

   virtual tResult CreateTexture(IImage * pImage, bool bAutoGenMipMaps, void * * ppTexture) = 0;

   virtual tResult SetVertexFormat(const sVertexElement * pVertexElements, uint nVertexElements) = 0;
   virtual tResult SetIndexFormat(eIndexFormat indexFormat) = 0;

   virtual tResult SubmitVertices(const void * pVertices, uint nVertices) = 0;

   virtual tResult SetDiffuseColor(const float diffuse[4]) = 0;

   virtual tResult SetTexture(uint textureUnit, const tChar * pszTexture) = 0;

   virtual tResult Render(ePrimitiveType primitive, const void * pIndices, uint nIndices) = 0;

   virtual tResult CreateFont(const tChar * pszFont, int fontPointSize, uint flags, IRenderFont * * ppFont) = 0;

   virtual tResult Begin2D(int width, int height, IRender2D * * ppRender2D) = 0;
   virtual tResult End2D() = 0;
};

///////////////////////////////////////

RENDER_API tResult RendererCreate();

RENDER_API tResult RendererResourceRegister();

///////////////////////////////////////

RENDER_API void RenderWireFrame(const tAxisAlignedBox & box, const cColor & color);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IRender2D
//

interface IRender2D : IUnknown
{
   virtual tResult GetViewportSize(int * pWidth, int * pHeight) const = 0;

   virtual void PushScissorRect(const tRect & rect) = 0;
   virtual void PopScissorRect() = 0;

   virtual void RenderSolidRect(const tRect & rect, const cColor & color) = 0;
   virtual void RenderBeveledRect(const tRect & rect, int bevel,
                                  const cColor & topLeft,
                                  const cColor & bottomRight,
                                  const cColor & face) = 0;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RENDERAPI_H
