///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guirender.h"

#include "font.h"

// TODO: HACK: get rid of this <windows.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include "dbgalloc.h" // must be last header

const uint kNumBitmapButtonIndices = 4;
const uint kNumBitmapButtonVertices = 16; // 4 button states * 4 vertices per state

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIRenderingTools
//

///////////////////////////////////////

VERTEXDECL_BEGIN(cGUIRenderingTools::gm_vertexDecl)
   VERTEXDECL_ELEMENT(kVDU_Color, kVDT_Float4)
   VERTEXDECL_ELEMENT(kVDU_Position, kVDT_Float2)
VERTEXDECL_END()

///////////////////////////////////////

VERTEXDECL_BEGIN(cGUIRenderingTools::gm_texVertexDecl)
   VERTEXDECL_ELEMENT(kVDU_TexCoord, kVDT_Float2)
   VERTEXDECL_ELEMENT(kVDU_Color, kVDT_Float4)
   VERTEXDECL_ELEMENT(kVDU_Position, kVDT_Float2)
VERTEXDECL_END()

///////////////////////////////////////

cGUIRenderingTools::cGUIRenderingTools()
{
}

///////////////////////////////////////

cGUIRenderingTools::~cGUIRenderingTools()
{
}

///////////////////////////////////////

tResult cGUIRenderingTools::Init()
{
   return S_OK;
}

///////////////////////////////////////

tResult cGUIRenderingTools::Term()
{
   SafeRelease(m_pFont);
   SafeRelease(m_pBitmapButtonVB);
   SafeRelease(m_pBitmapButtonIB);
   SafeRelease(m_pVertexDecl);
   SafeRelease(m_pTexVertexDecl);
   SafeRelease(m_pRenderDevice);
   return S_OK;
}

///////////////////////////////////////

tResult cGUIRenderingTools::SetRenderDevice(IRenderDevice * pRenderDevice)
{
   SafeRelease(m_pRenderDevice);
   m_pRenderDevice = CTAddRef(pRenderDevice);
   return S_OK;
}

///////////////////////////////////////

tResult cGUIRenderingTools::GetRenderDevice(IRenderDevice * * ppRenderDevice)
{
   return m_pRenderDevice.GetPointer(ppRenderDevice);
}

///////////////////////////////////////

tResult cGUIRenderingTools::SetDefaultFont(IRenderFont * pFont)
{
   SafeRelease(m_pFont);
   m_pFont = CTAddRef(pFont);
   return S_OK;
}

///////////////////////////////////////

tResult cGUIRenderingTools::GetDefaultFont(IRenderFont * * ppFont)
{
   if (!m_pFont)
   {
      m_pFont = FontCreateDefault();
   }
   return m_pFont.GetPointer(ppFont);
}

///////////////////////////////////////

tResult cGUIRenderingTools::GetBitmapButtonIndexBuffer(IIndexBuffer * * ppIndexBuffer)
{
   if (ppIndexBuffer != NULL)
   {
      if (!m_pBitmapButtonIB)
      {
         if (m_pRenderDevice != NULL)
         {
            if (m_pRenderDevice->CreateIndexBuffer(kNumBitmapButtonIndices, 
                                                   kBU_Default, 
                                                   kIBF_16Bit, 
                                                   kBP_Auto, 
                                                   &m_pBitmapButtonIB) == S_OK)
            {
               uint16 * pIndexData;
               if (m_pBitmapButtonIB->Lock(kBL_Discard, (void**)&pIndexData) == S_OK)
               {
                  pIndexData[0] = 0;
                  pIndexData[1] = 1;
                  pIndexData[2] = 2;
                  pIndexData[3] = 3;
                  m_pBitmapButtonIB->Unlock();
               }
            }
         }
      }

      if (m_pBitmapButtonIB != NULL)
      {
         *ppIndexBuffer = m_pBitmapButtonIB;
         m_pBitmapButtonIB->AddRef();
         return S_OK;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIRenderingTools::GetBitmapButtonVertexBuffer(const tGUIRect & rect, 
                                                        IVertexBuffer * * ppVertexBuffer)
{
   if (ppVertexBuffer == NULL)
   {
      return E_POINTER;
   }

   if (!m_pBitmapButtonVB)
   {
      if (m_pRenderDevice != NULL)
      {
         cAutoIPtr<IVertexDeclaration> pVertexDecl;
         if (GetTexVertexDeclaration(&pVertexDecl) != S_OK)
         {
            return E_FAIL;
         }

         if (m_pRenderDevice->CreateVertexBuffer(kNumBitmapButtonVertices, 
                                                 kBU_Default, pVertexDecl, kBP_Auto, 
                                                 &m_pBitmapButtonVB) != S_OK)
         {
            return E_FAIL;
         }
      }
   }

   if (m_pBitmapButtonVB != NULL)
   {
      float w = rect.GetWidth();
      float h = rect.GetHeight();

      sGUITexVertex verts[kNumBitmapButtonVertices];

      for (int i = 0; i < _countof(verts); i++)
      {
         verts[i].rgba[0] = 1.0f;
         verts[i].rgba[1] = 1.0f;
         verts[i].rgba[2] = 1.0f;
         verts[i].rgba[3] = 1.0f;
      }

      // TODO: Modifying the shared vertex buffer like this won't work
      // if the renderer does any queuing. Should specify UI coordinates
      // in [0..1] and map them to device coordinates or something.

      // steady state
      verts[0].u = 0;
      verts[0].v = 0.5;
      verts[0].pos = tVec2(rect.left,rect.top);
      verts[1].u = 0;
      verts[1].v = 0.25;
      verts[1].pos = tVec2(rect.left,rect.bottom);
      verts[2].u = 1;
      verts[2].v = 0.25;
      verts[2].pos = tVec2(rect.right,rect.bottom);
      verts[3].u = 1;
      verts[3].v = 0.5;
      verts[3].pos = tVec2(rect.right,rect.top);

      // hovered
      verts[4].u = 0;
      verts[4].v = 0.75;
      verts[4].pos = tVec2(rect.left,rect.top);
      verts[5].u = 0;
      verts[5].v = 0.5;
      verts[5].pos = tVec2(rect.left,rect.bottom);
      verts[6].u = 1;
      verts[6].v = 0.5;
      verts[6].pos = tVec2(rect.right,rect.bottom);
      verts[7].u = 1;
      verts[7].v = 0.75;
      verts[7].pos = tVec2(rect.right,rect.top);

      // pressed
      verts[8].u = 0;
      verts[8].v = 1;
      verts[8].pos = tVec2(rect.left,rect.top);
      verts[9].u = 0;
      verts[9].v = 0.75;
      verts[9].pos = tVec2(rect.left,rect.bottom);
      verts[10].u = 1;
      verts[10].v = 0.75;
      verts[10].pos = tVec2(rect.right,rect.bottom);
      verts[11].u = 1;
      verts[11].v = 1;
      verts[11].pos = tVec2(rect.right,rect.top);

      // disabled
      verts[12].u = 0;
      verts[12].v = 0.25;
      verts[12].pos = tVec2(rect.left,rect.top);
      verts[13].u = 0;
      verts[13].v = 0;
      verts[13].pos = tVec2(rect.left,rect.bottom);
      verts[14].u = 1;
      verts[14].v = 0;
      verts[14].pos = tVec2(rect.right,rect.bottom);
      verts[15].u = 1;
      verts[15].v = 0.25;
      verts[15].pos = tVec2(rect.right,rect.top);

      void * pVertexData;
      if (m_pBitmapButtonVB->Lock(kBL_Discard, &pVertexData) == S_OK)
      {
         memcpy(pVertexData, verts, sizeof(verts));
         m_pBitmapButtonVB->Unlock();
      }

      return m_pBitmapButtonVB.GetPointer(ppVertexBuffer);
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIRenderingTools::Render3dRect(const tGUIRect & rect, int bevel, 
                                         const tGUIColor & topLeft, 
                                         const tGUIColor & bottomRight, 
                                         const tGUIColor & face)
{
   static const uint kMaxVertices = 32;
   sGUIVertex vertices[kMaxVertices];

   uint nVertices = 0;

#define FillVertex(index, x, y, color) \
   do { \
      Assert(index == nVertices); \
      Assert(nVertices < kMaxVertices); \
      vertices[nVertices].rgba[0] = color.GetRed(); \
      vertices[nVertices].rgba[1] = color.GetGreen(); \
      vertices[nVertices].rgba[2] = color.GetBlue(); \
      vertices[nVertices].rgba[3] = color.GetAlpha(); \
      vertices[nVertices].pos = tVec2((x),(y)); \
      nVertices++; \
   } while (0)

   if (bevel == 0)
   {
      FillVertex(0, rect.left, rect.top, face);
      FillVertex(1, rect.left, rect.bottom, face);
      FillVertex(2, rect.right, rect.bottom, face);

      FillVertex(3, rect.right, rect.bottom, face);
      FillVertex(4, rect.right, rect.top, face);
      FillVertex(5, rect.left, rect.top, face);
   }
   else
   {
      float x0 = rect.left;
      float x1 = rect.left + bevel;
      float x2 = rect.right - bevel;
      float x3 = rect.right;

      float y0 = rect.top;
      float y1 = rect.top + bevel;
      float y2 = rect.bottom - bevel;
      float y3 = rect.bottom;

      FillVertex(0, x0, y0, topLeft);
      FillVertex(1, x0, y3, topLeft);
      FillVertex(2, x1, y2, topLeft);

      FillVertex(3, x0, y0, topLeft);
      FillVertex(4, x1, y2, topLeft);
      FillVertex(5, x1, y1, topLeft);

      FillVertex(6, x0, y0, topLeft);
      FillVertex(7, x2, y1, topLeft);
      FillVertex(8, x3, y0, topLeft);

      FillVertex(9, x0, y0, topLeft);
      FillVertex(10, x1, y1, topLeft);
      FillVertex(11, x2, y1, topLeft);

      FillVertex(12, x0, y3, bottomRight);
      FillVertex(13, x3, y3, bottomRight);
      FillVertex(14, x1, y2, bottomRight);

      FillVertex(15, x1, y2, bottomRight);
      FillVertex(16, x3, y3, bottomRight);
      FillVertex(17, x2, y2, bottomRight);

      FillVertex(18, x3, y0, bottomRight);
      FillVertex(19, x2, y1, bottomRight);
      FillVertex(20, x3, y3, bottomRight);

      FillVertex(21, x2, y1, bottomRight);
      FillVertex(22, x2, y2, bottomRight);
      FillVertex(23, x3, y3, bottomRight);

      FillVertex(24, x1, y1, face);
      FillVertex(25, x2, y2, face);
      FillVertex(26, x2, y1, face);

      FillVertex(27, x2, y2, face);
      FillVertex(28, x1, y1, face);
      FillVertex(29, x1, y2, face);
   }

#undef FillVertex

   glEnableClientState(GL_COLOR_ARRAY);
   glColorPointer(4, GL_FLOAT, sizeof(sGUIVertex), vertices);

   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(2, GL_FLOAT, sizeof(sGUIVertex), (byte*)vertices + (4*sizeof(float)));

   glDrawArrays(GL_TRIANGLES, 0, nVertices);

#if 0
   cAutoIPtr<IVertexDeclaration> pVertexDecl;
   if (pRenderDevice->CreateVertexDeclaration(gm_vertexDecl, _countof(gm_vertexDecl), &pVertexDecl) == S_OK)
   {
      cAutoIPtr<IVertexBuffer> pVertexBuffer;
      if (pRenderDevice->CreateVertexBuffer(kNumVertices, kBU_Default, pVertexDecl, kBP_System, &pVertexBuffer) == S_OK)
      {
         void * pVertexData;
         if (pVertexBuffer->Lock(kBL_Discard, &pVertexData) == S_OK)
         {
            memcpy(pVertexData, vertices, sizeof(vertices));
            pVertexBuffer->Unlock();
         }
      }
   }
#endif

   return S_OK;
}

///////////////////////////////////////

tResult cGUIRenderingTools::GetVertexDeclaration(IVertexDeclaration * * ppVertexDecl)
{
   if (ppVertexDecl == NULL)
   {
      return E_POINTER;
   }

   if (!m_pVertexDecl)
   {
      if (m_pRenderDevice != NULL)
      {
         tResult result = m_pRenderDevice->CreateVertexDeclaration(gm_vertexDecl, 
            _countof(gm_vertexDecl), &m_pVertexDecl);
         if (FAILED(result))
         {
            return result;
         }
      }
   }

   return m_pVertexDecl.GetPointer(ppVertexDecl);
}

///////////////////////////////////////

tResult cGUIRenderingTools::GetTexVertexDeclaration(IVertexDeclaration * * ppVertexDecl)
{
   if (ppVertexDecl == NULL)
   {
      return E_POINTER;
   }

   if (!m_pTexVertexDecl)
   {
      if (m_pRenderDevice != NULL)
      {
         tResult result = m_pRenderDevice->CreateVertexDeclaration(gm_texVertexDecl, 
            _countof(gm_texVertexDecl), &m_pTexVertexDecl);
         if (FAILED(result))
         {
            return result;
         }
      }
   }

   return m_pTexVertexDecl.GetPointer(ppVertexDecl);
}

///////////////////////////////////////

void GUIRenderingToolsCreate()
{
   cAutoIPtr<IGUIRenderingTools>(new cGUIRenderingTools);
}

///////////////////////////////////////////////////////////////////////////////
