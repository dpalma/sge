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

typedef unsigned int GLenum;

#ifdef HAVE_CG
typedef enum CGprofile;
typedef struct _CGcontext *CGcontext;
typedef struct _CGprogram *CGprogram;
typedef struct _CGparameter *CGparameter;
typedef void (*CGerrorCallbackFunc)(void);
#endif

F_DECLARE_INTERFACE(IDirect3DDevice9);
F_DECLARE_INTERFACE(IDirect3DStateBlock9);

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
   static void CgErrorCallback();

   bool m_bInScene;

   cAutoIPtr<IDirect3DDevice9> m_pD3dDevice;

#ifdef HAVE_CG
   CGcontext m_cgContext;
   CGerrorCallbackFunc m_oldCgErrorCallback;
   CGprofile m_cgProfile;
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
