///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "textureapi.h"

#include "imagedata.h"
#include "techmath.h"
#include "globalobj.h"
#include "techstring.h"
#include "resourceapi.h"

#include "stdgl.h"
#include <GL/glu.h>

#include <map>
#include <vector>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

// Convert a pixel format to a number of color components.
// This array must match the enum ePixelFormat type in imagedata.h,
// excluding kPF_ERROR which is -1 and not a valid array index.
static const GLint g_glTexComponents[] =
{
   0, // kPF_Grayscale
   0, // kPF_ColorMapped
   3, // kPF_RGB555
   3, // kPF_BGR555
   3, // kPF_RGB565
   3, // kPF_BGR565
   4, // kPF_RGBA1555
   4, // kPF_BGRA1555
   3, // kPF_RGB888
   3, // kPF_BGR888
   4, // kPF_RGBA8888
   4, // kPF_BGRA8888
};

///////////////////////////////////////////////////////////////////////////////

// Convert a pixel format to a GL format constant.
// This array must match the enum ePixelFormat type in imagedata.h,
// excluding kPF_ERROR which is -1 and not a valid array index.
static const GLenum g_glTexFormats[] =
{
   0, // kPF_Grayscale
   0, // kPF_ColorMapped
   0, // kPF_RGB555
   0, // kPF_BGR555
   0, // kPF_RGB565
   0, // kPF_BGR565
   0, // kPF_RGBA1555
   0, // kPF_BGRA1555
   GL_RGB,        // kPF_RGB888
   GL_BGR_EXT,    // kPF_BGR888
   GL_RGBA,       // kPF_RGBA8888
   GL_BGRA_EXT,   // kPF_BGRA8888
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTexture
//

class cTexture : public cComObject<IMPLEMENTS(ITexture)>
{
public:
   cTexture();
   ~cTexture();

   virtual void OnFinalRelease();

   tResult UploadImage(const cImageData * pImageData);

   virtual tResult GetTextureHandle(HANDLE * phTexture) const;

   GLuint GetTextureId() const { return m_textureId; }

private:
   void ReleaseTextureId();

   bool m_bHasAlpha;
   uint m_width, m_height;
   uint m_textureId;
};

///////////////////////////////////////

cTexture::cTexture()
 : m_bHasAlpha(false),
   m_width(0),
   m_height(0),
   m_textureId(0)
{
}

///////////////////////////////////////

cTexture::~cTexture()
{
}

///////////////////////////////////////

void cTexture::OnFinalRelease()
{
   ReleaseTextureId();
}

///////////////////////////////////////

tResult GlTextureCreate(const cImageData * pImageData, uint * pTexId)
{
   if (pImageData == NULL || pTexId == NULL)
   {
      return E_POINTER;
   }

   ePixelFormat pixelFormat = pImageData->GetPixelFormat();
   if (pixelFormat == kPF_ERROR)
   {
      WarnMsg("Invalid image format while creating texture\n");
      return E_FAIL;
   }

   GLenum texelFormat = g_glTexFormats[pixelFormat];
   if (texelFormat == 0)
   {
      WarnMsg1("Unsupported texture pixel format %d\n", pixelFormat);
      return E_FAIL;
   }

   GLint nComponents = g_glTexComponents[pixelFormat];
   if (nComponents == 0)
   {
      WarnMsg1("Unsupported texture pixel format %d\n", pixelFormat);
      return E_FAIL;
   }

   glGenTextures(1, pTexId);
   glBindTexture(GL_TEXTURE_2D, *pTexId);

   // gluBuild2DMipmaps will scale the image if its dimensions are not powers of two
   int result = gluBuild2DMipmaps(
      GL_TEXTURE_2D,
      nComponents,
      pImageData->GetWidth(),
      pImageData->GetHeight(),
      texelFormat,
      GL_UNSIGNED_BYTE,
      pImageData->GetData());

   DebugMsgIf1(result != 0, "gluBuild2DMipmaps returned error: \n",
      (const char *)gluErrorString(glGetError()));

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   return (result == 0) ? S_OK : E_FAIL;
}

tResult cTexture::UploadImage(const cImageData * pImageData)
{
   return GlTextureCreate(pImageData, &m_textureId);
}

///////////////////////////////////////

tResult cTexture::GetTextureHandle(HANDLE * phTexture) const
{
   if (phTexture == NULL)
   {
      return E_POINTER;
   }

   *phTexture = (HANDLE)m_textureId;
   return (m_textureId != 0) ? S_OK : S_FALSE;
}

///////////////////////////////////////

void cTexture::ReleaseTextureId()
{
   if (glIsTexture(m_textureId))
   {
      glDeleteTextures(1, &m_textureId);
   }
   m_textureId = 0;
}

///////////////////////////////////////

tResult TextureCreate(const cImageData * pImageData, ITexture * * ppTexture)
{
   if (pImageData == NULL || ppTexture == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<cTexture> pTex(new cTexture);
   if (!pTex)
   {
      return E_OUTOFMEMORY;
   }

   if (pTex->UploadImage(pImageData) == S_OK)
   {
      *ppTexture = CTAddRef(pTex);
      return S_OK;
   }

   return E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////

void * GlTextureFromImageData(void * pData, int dataLength, void * param)
{
   cImageData * pImageData = reinterpret_cast<cImageData*>(pData);

   uint texId;
   if (GlTextureCreate(pImageData, &texId) == S_OK)
   {
      return reinterpret_cast<void*>(texId);
   }

   return NULL;
}

void GlTextureUnload(void * pData)
{
   uint tex = reinterpret_cast<uint>(pData);
   if (glIsTexture(tex))
   {
      glDeleteTextures(1, &tex);
   }
}

///////////////////////////////////////////////////////////////////////////////

void * ITextureFromImageData(void * pData, int dataLength, void * param)
{
   cImageData * pImageData = reinterpret_cast<cImageData*>(pData);

   cAutoIPtr<cTexture> pTex(new cTexture);
   if (!pTex || pTex->UploadImage(pImageData) != S_OK)
   {
      return NULL;
   }

   return CTAddRef(static_cast<ITexture*>(pTex));
}

void ITextureUnload(void * pData)
{
   ITexture * pTexture = reinterpret_cast<ITexture*>(pData);
   if (pTexture != NULL)
   {
      pTexture->Release();
   }
}

///////////////////////////////////////////////////////////////////////////////

tResult GlTextureResourceRegister()
{
   UseGlobal(ResourceManager);
   if (!!pResourceManager)
   {
      if (SUCCEEDED(pResourceManager->RegisterFormat(kRC_Texture, kRC_Image, NULL, NULL, ITextureFromImageData, ITextureUnload))
         && SUCCEEDED(pResourceManager->RegisterFormat(kRC_GlTexture, kRC_Image, NULL, NULL, GlTextureFromImageData, GlTextureUnload)))
      {
         return S_OK;
      }
   }
   return E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////
