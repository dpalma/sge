///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ggl.h"
#include "uirender.h"

#include "font.h"
#include "render.h"
#include "textureapi.h"

#include "globalobj.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

cUISize UIGetRootContainerSize()
{
   int viewport[4];
   glGetIntegerv(GL_VIEWPORT, viewport);

   return cUISize(viewport[2], viewport[3]);
}

///////////////////////////////////////////////////////////////////////////////

cUISize UIMeasureText(const char * pszText, int textLen, IRenderFont * pFont /*=NULL*/)
{
   Assert(pszText != NULL);

   static cColor nocolor;

   tRect rect(0,0,0,0);
   if (pFont == NULL)
   {
      cAutoIPtr<IRenderFont> pFont2 = FontCreateDefault();
      pFont2->DrawText(pszText, textLen, kDT_CalcRect, &rect, nocolor);
   }
   else
   {
      pFont->DrawText(pszText, textLen, kDT_CalcRect, &rect, nocolor);
   }

   return cUISize(rect.GetWidth(), rect.GetHeight());
}

///////////////////////////////////////////////////////////////////////////////

void UIDrawText(const char * pszText, int textLen,
                const cUIRect * pRect, uint flags /*=kTextDefault*/,
                IRenderFont * pFont /*=NULL*/,
                const cUIColor & color /*=cUIColor(1,1,1,1)*/)
{
   tRect rect(Round(pRect->left),Round(pRect->top),Round(pRect->right),Round(pRect->bottom));
   pFont->DrawText(pszText, textLen, flags, &rect, color);
}

///////////////////////////////////////////////////////////////////////////////

void UIDraw3dRect(const cUIRect & rect,
                  int bevel,
                  const cUIColor & topLeft,
                  const cUIColor & bottomRight,
                  const cUIColor & face)
{
   float x0 = rect.left;
   float x1 = rect.left + bevel;
   float x2 = rect.right - bevel;
   float x3 = rect.right;

   float y0 = rect.top;
   float y1 = rect.top + bevel;
   float y2 = rect.bottom - bevel;
   float y3 = rect.bottom;

   glBegin(GL_TRIANGLES);

      glColor4fv(topLeft.GetPointer());

      glVertex2f(x0, y0);
      glVertex2f(x0, y3);
      glVertex2f(x1, y2);

      glVertex2f(x0, y0);
      glVertex2f(x1, y2);
      glVertex2f(x1, y1);

      glVertex2f(x0, y0);
      glVertex2f(x2, y1);
      glVertex2f(x3, y0);

      glVertex2f(x0, y0);
      glVertex2f(x1, y1);
      glVertex2f(x2, y1);

      glColor4fv(bottomRight.GetPointer());

      glVertex2f(x0, y3);
      glVertex2f(x3, y3);
      glVertex2f(x1, y2);

      glVertex2f(x1, y2);
      glVertex2f(x3, y3);
      glVertex2f(x2, y2);

      glVertex2f(x3, y0);
      glVertex2f(x2, y1);
      glVertex2f(x3, y3);

      glVertex2f(x2, y1);
      glVertex2f(x2, y2);
      glVertex2f(x3, y3);

      glColor4fv(face.GetPointer());

      glVertex2f(x1, y1);
      glVertex2f(x2, y2);
      glVertex2f(x2, y1);

      glVertex2f(x2, y2);
      glVertex2f(x1, y1);
      glVertex2f(x1, y2);

   glEnd();
}

///////////////////////////////////////////////////////////////////////////////

void UIDrawGradientRect(const cUIRect & rect,
                        const cUIColor & topLeft,
                        const cUIColor & topRight,
                        const cUIColor & bottomRight,
                        const cUIColor & bottomLeft)
{
   glPushAttrib(GL_CURRENT_BIT);
   glBegin(GL_QUADS);
      glColor4fv(topLeft.GetPointer());
      glVertex2f(rect.left, rect.top);
      glColor4fv(bottomLeft.GetPointer());
      glVertex2f(rect.left, rect.bottom);
      glColor4fv(bottomRight.GetPointer());
      glVertex2f(rect.right, rect.bottom);
      glColor4fv(topRight.GetPointer());
      glVertex2f(rect.right, rect.top);
   glEnd();
   glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////////

void UIDrawSolidRect(const cUIRect & rect,
                     const cUIColor & color)
{
   UIDrawGradientRect(rect, color, color, color, color);
}

///////////////////////////////////////////////////////////////////////////////

void UIDrawTextureRect(const cUIRect & rect,
                       ITexture * pTexture)
{
   uint texId;
   if (pTexture != NULL && pTexture->GetTextureId(&texId) == S_OK)
   {
      glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT);
      glEnable(GL_TEXTURE_2D);
      glEnable(GL_ALPHA_TEST);
      glBindTexture(GL_TEXTURE_2D, texId);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glColor4f(1,1,1,1); // set color and alpha multipliers to one
      glBegin(GL_QUADS);
         glTexCoord2i(0, 1);
         glVertex2f(rect.left, rect.top);
         glTexCoord2i(0, 0);
         glVertex2f(rect.left, rect.bottom);
         glTexCoord2i(1, 0);
         glVertex2f(rect.right, rect.bottom);
         glTexCoord2i(1, 1);
         glVertex2f(rect.right, rect.top);
      glEnd();
      glPopAttrib();
   }
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIRenderingTools
//

class cUIRenderingTools : public cGlobalObject<IMPLEMENTS(IUIRenderingTools)>
{
public:
   cUIRenderingTools();
   ~cUIRenderingTools();

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult SetRenderDevice(IRenderDevice * pRenderDevice);
   virtual tResult GetRenderDevice(IRenderDevice * * ppRenderDevice);
   virtual tResult GetUIVertexDeclaration(IVertexDeclaration * * ppVertexDecl);
   virtual tResult GetBitmapButtonIndexBuffer(IIndexBuffer * * ppIndexBuffer);
   virtual tResult GetBitmapButtonVertexBuffer(const cUIRect & rect, IVertexBuffer * * ppVertexBuffer);

private:
   cAutoIPtr<IRenderDevice> m_pRenderDevice;
   static sVertexElement gm_UIVertexDecl[];
   cAutoIPtr<IVertexDeclaration> m_pUIVertexDecl;
   cAutoIPtr<IVertexBuffer> m_pBitmapButtonVB;
   cAutoIPtr<IIndexBuffer> m_pBitmapButtonIB;
   cAutoIPtr<IRenderFont> m_pFont;
};

///////////////////////////////////////

VERTEXDECL_BEGIN(cUIRenderingTools::gm_UIVertexDecl)
   VERTEXDECL_ELEMENT(kVDU_TexCoord, kVDT_Float2)
   VERTEXDECL_ELEMENT(kVDU_Color, kVDT_UnsignedByte4)
   VERTEXDECL_ELEMENT(kVDU_Position, kVDT_Float2)
VERTEXDECL_END()

///////////////////////////////////////

cUIRenderingTools::cUIRenderingTools()
{
}

///////////////////////////////////////

cUIRenderingTools::~cUIRenderingTools()
{
}

///////////////////////////////////////

tResult cUIRenderingTools::Init()
{
   m_pFont = FontCreateDefault();
   return S_OK;
}

///////////////////////////////////////

tResult cUIRenderingTools::Term()
{
   SafeRelease(m_pFont);
   SafeRelease(m_pBitmapButtonVB);
   SafeRelease(m_pBitmapButtonIB);
   SafeRelease(m_pUIVertexDecl);
   SafeRelease(m_pRenderDevice);
   return S_OK;
}

///////////////////////////////////////

tResult cUIRenderingTools::SetRenderDevice(IRenderDevice * pRenderDevice)
{
   SafeRelease(m_pRenderDevice);
   m_pRenderDevice = pRenderDevice;
   if (pRenderDevice)
      pRenderDevice->AddRef();
   return S_OK;
}

///////////////////////////////////////

tResult cUIRenderingTools::GetRenderDevice(IRenderDevice * * ppRenderDevice)
{
   if (ppRenderDevice != NULL)
   {
      if (!m_pRenderDevice)
      {
         *ppRenderDevice = NULL;
         return S_FALSE;
      }
      else
      {
         *ppRenderDevice = m_pRenderDevice;
         m_pRenderDevice->AddRef();
         return S_OK;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cUIRenderingTools::GetUIVertexDeclaration(IVertexDeclaration * * ppVertexDecl)
{
   if (ppVertexDecl != NULL)
   {
      if (!m_pUIVertexDecl)
      {
         if (m_pRenderDevice != NULL)
         {
            m_pRenderDevice->CreateVertexDeclaration(gm_UIVertexDecl, _countof(gm_UIVertexDecl), &m_pUIVertexDecl);
         }
      }

      if (m_pUIVertexDecl != NULL)
      {
         *ppVertexDecl = m_pUIVertexDecl;
         m_pUIVertexDecl->AddRef();
         return S_OK;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cUIRenderingTools::GetBitmapButtonIndexBuffer(IIndexBuffer * * ppIndexBuffer)
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

tResult cUIRenderingTools::GetBitmapButtonVertexBuffer(const cUIRect & rect, IVertexBuffer * * ppVertexBuffer)
{
   if (ppVertexBuffer != NULL)
   {
      if (!m_pBitmapButtonVB)
      {
         if (m_pRenderDevice != NULL)
         {
            cAutoIPtr<IVertexDeclaration> pVertexDecl;
            if (GetUIVertexDeclaration(&pVertexDecl) == S_OK)
            {
               if (m_pRenderDevice->CreateVertexBuffer(kNumBitmapButtonVertices, 
                                                       kBU_Default, 
                                                       pVertexDecl, 
                                                       kBP_Auto, 
                                                       &m_pBitmapButtonVB) != S_OK)
               {
                  return E_FAIL;
               }
            }
         }
      }

      if (m_pBitmapButtonVB != NULL)
      {
         float w = rect.GetWidth();
         float h = rect.GetHeight();

         sUIVertex verts[kNumBitmapButtonVertices];

         for (int i = 0; i < _countof(verts); i++)
         {
            verts[i].r = verts[i].g = verts[i].b = verts[i].a = 255;
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

         *ppVertexBuffer = m_pBitmapButtonVB;
         m_pBitmapButtonVB->AddRef();
         return S_OK;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

void UIRenderingToolsCreate()
{
   cAutoIPtr<IUIRenderingTools>(new cUIRenderingTools);
}

///////////////////////////////////////////////////////////////////////////////
