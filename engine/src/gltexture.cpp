///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "renderapi.h"

#include "imagedata.h"
#include "techmath.h"
#include "globalobj.h"
#include "techstring.h"
#include "resourceapi.h"

#include "stdgl.h"
#include <GL/glu.h>

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

tResult GlTextureResourceRegister()
{
   UseGlobal(ResourceManager);
   if (!!pResourceManager)
   {
      if (SUCCEEDED(pResourceManager->RegisterFormat(kRC_GlTexture, kRC_Image, NULL, NULL, GlTextureFromImageData, GlTextureUnload)))
      {
         return S_OK;
      }
   }
   return E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////
