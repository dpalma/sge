///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_TILEDGROUND_H
#define INCLUDED_TILEDGROUND_H

#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

class cHeightMap;

F_DECLARE_INTERFACE(IRenderDevice);
F_DECLARE_INTERFACE(IVertexBuffer);
F_DECLARE_INTERFACE(IIndexBuffer);
F_DECLARE_INTERFACE(IMaterial);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTiledGround
//

class cTiledGround
{
public:
   cTiledGround();
   ~cTiledGround();

   bool Init(IRenderDevice * pRenderDevice, cHeightMap * pHeightMap, const char * pszTexture);

   int GetVertexCount() const { return m_nVerts; }
   int GetIndexCount() const { return m_nIndices; }

   IVertexBuffer * AccessVertexBuffer() { return m_pVertexBuffer; }
   IIndexBuffer * AccessIndexBuffer() { return m_pIndexBuffer; }
   IMaterial * AccessMaterial() { return m_pMaterial; }

private:
   cAutoIPtr<IVertexBuffer> m_pVertexBuffer;
   cAutoIPtr<IIndexBuffer> m_pIndexBuffer;
   cAutoIPtr<IMaterial> m_pMaterial;
   int m_nVerts, m_nIndices;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TILEDGROUND_H
