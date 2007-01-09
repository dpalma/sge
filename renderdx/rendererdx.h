////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDERERDX_H
#define INCLUDED_RENDERERDX_H

#include "render/renderapi.h"

#include "tech/globalobjdef.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif

#ifdef HAVE_CG
#include <Cg/Cg.h>
#endif

F_DECLARE_HANDLE(HINSTANCE);

F_DECLARE_INTERFACE(IDirect3D9);
F_DECLARE_INTERFACE(IDirect3DDevice9);
F_DECLARE_INTERFACE(IDirect3DStateBlock9);

F_DECLARE_INTERFACE(IReader);

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRendererDX
//

class cRendererDX : public cComObject2<IMPLEMENTS(IRenderer), IMPLEMENTS(IGlobalObject)>
{
   enum
   {
      kMaxVertexElements = 16,
      kMaxBlendMatrices = 64,
   };

public:
   cRendererDX();
   virtual ~cRendererDX();

   DECLARE_NAME(Renderer)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult CreateContext();
   virtual tResult CreateContext(HWND hWnd);
   virtual tResult CreateContext(Display * display, Window window);
   virtual tResult DestroyContext();

   virtual tResult SetRenderState(eRenderState state, ulong value);
   virtual tResult GetRenderState(eRenderState state, ulong * pValue);

   virtual tResult BeginScene();
   virtual tResult EndScene();

   virtual tResult CreateTexture(IImage * pImage, bool bAutoGenMipMaps, void * * ppTexture);

   virtual tResult SetVertexFormat(const sVertexElement * pVertexElements, uint nVertexElements);
   virtual tResult SetIndexFormat(eIndexFormat indexFormat);

   virtual tResult SubmitVertices(const void * pVertices, uint nVertices);

   virtual tResult SetDiffuseColor(const float diffuse[4]);

   virtual tResult SetTexture(uint textureUnit, const void * texture);
   virtual tResult SetTexture(uint textureUnit, const tChar * pszTexture);

   virtual tResult Render(ePrimitiveType primitive, const void * pIndices, uint nIndices);

   virtual tResult CreateFont(const tChar * pszFont, int fontPointSize, uint flags, IRenderFont * * ppFont);

   virtual tResult Begin2D(int width, int height, IRender2D * * ppRender2D);
   virtual tResult End2D();

   virtual tResult GetViewMatrix(float viewMatrix[16]) const;
   virtual tResult SetViewMatrix(const float viewMatrix[16]);

   virtual tResult GetProjectionMatrix(float projMatrix[16]) const;
   virtual tResult SetProjectionMatrix(const float projMatrix[16]);

   virtual tResult GetViewProjectionMatrix(float viewProjMatrix[16]) const;
   virtual tResult GetViewProjectionInverseMatrix(float viewProjInvMatrix[16]) const;

   virtual tResult PushMatrix(const float matrix[16]);
   virtual tResult PopMatrix();

   virtual tResult ScreenToNormalizedDeviceCoords(int sx, int sy, float * pndx, float * pndy) const;
   virtual tResult GeneratePickRay(float ndx, float ndy, cRay * pRay) const;

private:
#ifdef HAVE_CG
   static void CgErrorHandler(CGcontext cgContext, CGerror cgError, void * pData);
#endif

   bool m_bInScene;

   HWND m_hWnd;

   HINSTANCE m_hD3dLib;
   cAutoIPtr<IDirect3D9> m_pD3d;
   cAutoIPtr<IDirect3DDevice9> m_pD3dDevice;

#ifdef HAVE_CG
   friend void * CgProgramLoad(IReader * pReader, void * typeParam);
   friend void * CgEffectLoad(IReader * pReader, void * typeParam);

   CGcontext m_cgContext;
   CGerrorHandlerFunc m_oldCgErrorHandler;
   void * m_pOldCgErrHandlerData;
   CGprofile m_cgProfileVertex, m_cgProfileFragment;
#endif

   sVertexElement m_vertexElements[kMaxVertexElements];
   uint m_nVertexElements; // How many of the vertex elements above are valid?
   uint m_vertexSize; // Size, in bytes, of a vertex in the current format

   eIndexFormat m_indexFormat;

   typedef std::map<uint, IRenderFont *> tFontMap;
   tFontMap m_fontMap;

   cAutoIPtr<IRender2D> m_pRender2D;
   cAutoIPtr<IDirect3DStateBlock9> m_pStateBlock;
};

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RENDERERDX_H
