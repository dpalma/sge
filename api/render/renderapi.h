///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDERAPI_H
#define INCLUDED_RENDERAPI_H

#include "renderdll.h"
#include "tech/comtools.h"

#include "tech/rect.h"

#ifdef _MSC_VER
#pragma once
#endif

class cRay;

F_DECLARE_HANDLE(HWND);

struct _XDisplay;
typedef struct _XDisplay Display;
typedef unsigned long XID;
typedef XID Window;

F_DECLARE_INTERFACE(IImage);

F_DECLARE_INTERFACE(IRenderer);
F_DECLARE_INTERFACE_GUID(IRenderTarget, "B962C170-8A3A-409e-8031-04CD3E7C44AD");
F_DECLARE_INTERFACE_GUID(IRenderCamera, "5065B828-D331-478f-ABA2-8AB171418499");
F_DECLARE_INTERFACE(IRenderFont);

///////////////////////////////////////////////////////////////////////////////

#define kRT_CgProgram   _T("CgProgram")
#define kRT_CgEffect    _T("CgEffect")
#define kRT_GlTexture   _T("GlTexture")

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

enum ePrimitiveType
{
   kPT_Lines,
   kPT_LineStrip,
   kPT_Triangles,
   kPT_TriangleStrip,
   kPT_TriangleFan,
};

typedef enum ePrimitiveType tPrimitiveType;

enum eFillMode
{
   kFM_Point,
   kFM_Wireframe,
   kFM_Solid,
};

enum eRenderState
{
   kRS_AlphaTestEnable,
   kRS_AlphaBlendEnable,
   kRS_FillMode,
};

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IRenderer
//

interface IRenderer : IUnknown
{
   virtual tResult CreateRenderTarget(HWND hWnd, IRenderTarget * * ppRenderTarget) = 0;
   virtual tResult CreateRenderTarget(Display * display, Window window, IRenderTarget * * ppRenderTarget) = 0;

   virtual tResult GetRenderTarget(IRenderTarget * * ppRenderTarget) = 0;
   virtual tResult SetRenderTarget(IRenderTarget * pRenderTarget) = 0;

   virtual tResult SetRenderState(eRenderState state, ulong value) = 0;
   virtual tResult GetRenderState(eRenderState state, ulong * pValue) = 0;

   virtual tResult BeginScene() = 0;
   virtual tResult EndScene() = 0;

   virtual tResult CreateTexture(IImage * pImage, bool bAutoGenMipMaps, void * * ppTexture) = 0;

   virtual tResult SetVertexFormat(const sVertexElement * pVertexElements, uint nVertexElements) = 0;
   virtual tResult SetIndexFormat(eIndexFormat indexFormat) = 0;

   virtual tResult SubmitVertices(const void * pVertices, uint nVertices) = 0;

   virtual tResult SetDiffuseColor(const float diffuse[4]) = 0;

   virtual tResult SetTexture(uint textureUnit, const void * texture) = 0;
   virtual tResult SetTexture(uint textureUnit, const tChar * pszTexture) = 0;

   virtual tResult Render(ePrimitiveType primitive, uint startIndex, uint nIndices) = 0;
   virtual tResult RenderIndexed(ePrimitiveType primitive, const void * pIndices, uint nIndices) = 0;

   virtual tResult CreateFont(const tChar * pszFont, int fontPointSize, uint flags, IRenderFont * * ppFont) = 0;

   virtual tResult Begin2D(int width, int height) = 0;
   virtual tResult End2D() = 0;

   virtual tResult PushMatrix(const float matrix[16]) = 0;
   virtual tResult PopMatrix() = 0;

   virtual tResult GetCamera(IRenderCamera * * ppCamera) = 0;
   virtual tResult SetCamera(IRenderCamera * pCamera) = 0;

   virtual void PushScissorRect(const tRecti & rect) = 0;
   virtual void PopScissorRect() = 0;
};

RENDER_API tResult RendererCreate();


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IRenderTarget
//

interface IRenderTarget : IUnknown
{
   virtual void SwapBuffers() = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IRenderCamera
//

interface IRenderCamera : IUnknown
{
   virtual const float * GetViewMatrix() const = 0;
   virtual tResult GetViewMatrix(float viewMatrix[16]) const = 0;
   virtual tResult SetViewMatrix(const float viewMatrix[16]) = 0;

   virtual const float * GetProjectionMatrix() const = 0;
   virtual tResult GetProjectionMatrix(float projMatrix[16]) const = 0;
   virtual tResult SetProjectionMatrix(const float projMatrix[16]) = 0;

   // Perpective projection parameters

   virtual float GetFOV() const = 0;
   virtual void SetFOV(float fov) = 0;

   virtual float GetAspect() const = 0;
   virtual void SetAspect(float aspect) = 0;
   virtual void SetAutoAspect() = 0;

   virtual tResult GetNearFar(float * pZNear, float * pZFar) const = 0;
   virtual tResult SetNearFar(float zNear, float zFar) = 0;

   // Composite matrices

   virtual tResult GetViewProjectionMatrix(float viewProjMatrix[16]) const = 0;
   virtual tResult GetViewProjectionInverseMatrix(float viewProjInvMatrix[16]) const = 0;

   virtual tResult ScreenToNormalizedDeviceCoords(int sx, int sy, float * pndx, float * pndy) const = 0;
   virtual tResult GeneratePickRay(float ndx, float ndy, cRay * pRay) const = 0;

   inline tResult GenerateScreenPickRay(int sx, int sy, cRay * pRay) const
   {
      float ndx, ndy;
      if (ScreenToNormalizedDeviceCoords(sx, sy, &ndx, &ndy) == S_OK
         && GeneratePickRay(ndx, ndy, pRay) == S_OK)
      {
         return S_OK;
      }
      return E_FAIL;
   }
};

RENDER_API tResult RenderCameraCreate(IRenderCamera * * ppRenderCamera);


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RENDERAPI_H
