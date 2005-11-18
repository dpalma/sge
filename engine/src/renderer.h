////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDERER_H
#define INCLUDED_RENDERER_H

#include "renderapi.h"

#include "globalobjdef.h"
#include "matrix4.h"

#ifdef _MSC_VER
#pragma once
#endif

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
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init() { return S_OK; }
   virtual tResult Term() { return S_OK; }

   virtual tResult BeginScene();
   virtual tResult EndScene();
   virtual tResult SetVertexFormat(const sVertexElement * pVertexElements, uint nVertexElements);
   virtual tResult SetIndexFormat(eIndexFormat indexFormat);
   virtual tResult SubmitVertices(void * pVertices, uint nVertices);
   virtual tResult SetBlendMatrices(const tMatrix4 * pMatrices, uint nMatrices);
   virtual tResult GetBlendMatrices(tMatrix4 * pMatrices, uint nMatrices) const;
   virtual tResult Render(ePrimitiveType primitive, void * pIndices, uint nIndices);

private:
   tResult Initialize();

   bool m_bInitialized;
   bool m_bInScene;

   sVertexElement m_vertexElements[kMaxVertexElements];
   uint m_nVertexElements; // How many of the vertex elements above are valid?
   uint m_vertexSize; // Size, in bytes, of a vertex in the current format

   eIndexFormat m_indexFormat;
   GLenum m_glIndexFormat;

   tMatrix4 m_blendMatrices[kMaxBlendMatrices];
   uint m_nBlendMatrices; // How many of the matrices above are valid?
};

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RENDERER_H
