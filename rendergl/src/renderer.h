////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDERER_H
#define INCLUDED_RENDERER_H

#include "renderapi.h"

#include "globalobjdef.h"

#include <map>

#ifdef HAVE_CG
#include <Cg/Cg.h>
#endif

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IReader);

typedef unsigned int GLenum;

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRenderer
//

class cRenderer : public cComObject2<IMPLEMENTS(IRenderer), IMPLEMENTS(IGlobalObject)>
{
   enum
   {
      kMaxVertexElements = 16,
      kMaxBlendMatrices = 64,
   };

public:
   cRenderer();
   virtual ~cRenderer();

   DECLARE_NAME(Renderer)
   DECLARE_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

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

private:
   tResult Initialize();
#ifdef HAVE_CG
   static void CgErrorHandler(CGcontext cgContext, CGerror cgError, void * pData);
#endif

   bool m_bInitialized;
   bool m_bInScene;

#ifdef HAVE_CG
   friend void * CgProgramLoad(IReader * pReader, void * typeParam);
   friend void * CgEffectLoad(IReader * pReader, void * typeParam);

   CGcontext m_cgContext;
   CGerrorHandlerFunc m_oldCgErrorHandler;
   void * m_pOldCgErrHandlerData;
   CGprofile m_cgProfile;
#endif

   sVertexElement m_vertexElements[kMaxVertexElements];
   uint m_nVertexElements; // How many of the vertex elements above are valid?
   uint m_vertexSize; // Size, in bytes, of a vertex in the current format

   eIndexFormat m_indexFormat;
   GLenum m_glIndexFormat;

   typedef std::map<uint, IRenderFont *> tFontMap;
   tFontMap m_fontMap;

   cAutoIPtr<IRender2D> m_pRender2D;
};

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RENDERER_H
