////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDERER_H
#define INCLUDED_RENDERER_H

#include "renderapi.h"

#include "globalobjdef.h"

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
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult BeginScene();
   virtual tResult EndScene();

   virtual tResult CreateTexture(IImage * pImage, bool bAutoGenMipMaps, void * * ppTexture);

   virtual tResult SetVertexFormat(const sVertexElement * pVertexElements, uint nVertexElements);
   virtual tResult SetIndexFormat(eIndexFormat indexFormat);

   virtual tResult SubmitVertices(const void * pVertices, uint nVertices);

   virtual tResult SetDiffuseColor(const float diffuse[4]);

   virtual tResult SetTexture(uint textureUnit, const tChar * pszTexture);

   virtual tResult Render(ePrimitiveType primitive, const void * pIndices, uint nIndices);

private:
   tResult Initialize();
   static void CgErrorCallback();

   bool m_bInitialized;
   bool m_bInScene;

#ifdef HAVE_CG
   CGcontext m_cgContext;
   CGerrorCallbackFunc m_oldCgErrorCallback;
   CGprofile m_cgProfile;
#endif

   sVertexElement m_vertexElements[kMaxVertexElements];
   uint m_nVertexElements; // How many of the vertex elements above are valid?
   uint m_vertexSize; // Size, in bytes, of a vertex in the current format

   eIndexFormat m_indexFormat;
   GLenum m_glIndexFormat;
};

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RENDERER_H
