///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "render.h"

#include "techmath.h"
#include "pixelformat.h"
#include "image.h"

#include "stdgl.h"
#include <GL/glu.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTexture
//

class cTexture : public cComObject<IMPLEMENTS(ITexture)>
{
public:
   cTexture();

   virtual void OnFinalRelease();

   tResult UploadImage(cImage * pImageData);

   virtual tResult GetTextureId(uint * pTextureId) const
   {
      Assert(pTextureId != NULL);
      *pTextureId = m_textureId;
      return S_OK;
   }

private:
   uint m_textureId;
};

///////////////////////////////////////

cTexture::cTexture()
 : m_textureId(0)
{
}

///////////////////////////////////////

void cTexture::OnFinalRelease()
{
   if (glIsTexture(m_textureId))
      glDeleteTextures(1, &m_textureId);
   m_textureId = 0;
}

///////////////////////////////////////

static GLint GlTexNumComponents(GLenum format)
{
   switch (format)
   {
      case GL_RGB:
      case GL_BGR_EXT:
         return 3;

      case GL_RGBA:
      case GL_BGRA_EXT:
         return 4;

      default:
         return 0;
   }
}

///////////////////////////////////////

static GLenum GlTexFormat(ePixelFormat pf)
{
   switch (pf)
   {
      case kPF_RGB888:
         return GL_RGB;

      case kPF_BGR888:
         return GL_BGR_EXT;

      case kPF_RGBA8888:
         return GL_RGBA;

      case kPF_BGRA8888:
         return GL_BGRA_EXT;

      default:
         return 0;
   }
}

///////////////////////////////////////

tResult cTexture::UploadImage(cImage * pImage)
{
   if (pImage == NULL)
      return E_FAIL;

   bool bNoMipMaps = false;

   if (bNoMipMaps &&
       (!IsPowerOfTwo(pImage->GetWidth()) || !IsPowerOfTwo(pImage->GetHeight())))
   {
      DebugMsg2("WARNING: Improper texture dimensions %dx%d\n",
         pImage->GetWidth(), pImage->GetHeight());
      return E_FAIL;
   }

   GLenum format = GlTexFormat(pImage->GetPixelFormat());

   if (format == 0)
   {
      DebugMsg("WARNING: unsupported texture pixel format\n");
      return E_FAIL;
   }

   GLint nComponents = GlTexNumComponents(format);

   if (nComponents == 0)
   {
      DebugMsg1("Unsupported texture pixel format %x in InstallTexture\n", format);
      return E_FAIL;
   }

   glGenTextures(1, &m_textureId);
   glBindTexture(GL_TEXTURE_2D, m_textureId);

   glPixelStorei(GL_UNPACK_ALIGNMENT, pImage->GetAlignment());

   if (bNoMipMaps)
   {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

      glTexImage2D(GL_TEXTURE_2D, 0, nComponents, pImage->GetWidth(), pImage->GetHeight(),
         0, format, GL_UNSIGNED_BYTE, pImage->GetData());
   }
   else
   {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);

      // gluBuild2DMipmaps will scale the image if its dimensions are not powers of two
      gluBuild2DMipmaps(GL_TEXTURE_2D, nComponents, pImage->GetWidth(), pImage->GetHeight(),
         format, GL_UNSIGNED_BYTE, pImage->GetData());
   }

   return S_OK;
}

///////////////////////////////////////

ITexture * TextureCreate(cImage * pImageData)
{
   cTexture * pTexture = new cTexture;
   if (pTexture != NULL)
   {
      if (pTexture->UploadImage(pImageData) == S_OK)
      {
         return static_cast<ITexture *>(pTexture);
      }
   }
   return NULL;
}

///////////////////////////////////////////////////////////////////////////////
