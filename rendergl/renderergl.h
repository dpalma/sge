////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDERERGL_H
#define INCLUDED_RENDERERGL_H

#include "render/renderapi.h"

#include "tech/globalobjdef.h"

#include <map>

#ifdef HAVE_CG
#include <Cg/Cg.h>
#endif

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_HANDLE(HDC);
F_DECLARE_HANDLE(HGLRC);

typedef struct __GLXcontextRec * GLXContext;

F_DECLARE_INTERFACE(IReader);

typedef unsigned int GLenum;

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRendererGL
//

class cRendererGL : public cComObject2<IMPLEMENTS(IRenderer), IMPLEMENTS(IGlobalObject)>
{
   enum
   {
      kMaxVertexElements = 16,
      kMaxBlendMatrices = 64,
   };

public:
   cRendererGL();
   virtual ~cRendererGL();

   DECLARE_NAME(Renderer)
   DECLARE_CONSTRAINTS()

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

   virtual tResult PushMatrix(const float matrix[16]);
   virtual tResult PopMatrix();

   virtual tResult GetCamera(IRenderCamera * * ppCamera);
   virtual tResult SetCamera(IRenderCamera * pCamera);

private:
#ifdef HAVE_CG
   static void CgErrorHandler(CGcontext cgContext, CGerror cgError, void * pData);
#endif

#ifdef _WIN32
   HWND m_hWnd;
   HDC m_hDC;
   HGLRC m_hGLRC;
#else
   Display * m_display;
   GLXContext m_context;
#endif

   bool m_bInScene;

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
   GLenum m_glIndexFormat;

   typedef std::map<uint, IRenderFont *> tFontMap;
   tFontMap m_fontMap;

   cAutoIPtr<IRender2D> m_pRender2D;

   cAutoIPtr<IRenderCamera> m_pCamera;
};

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RENDERERGL_H
