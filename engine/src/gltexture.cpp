///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "textureapi.h"

#include "imagedata.h"
#include "techmath.h"
#include "globalobj.h"
#include "str.h"
#include "resmgr.h"

#include "stdgl.h"
#include <GL/glu.h>

#include <map>
#include <vector>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

static GLenum GlTexFormat(ePixelFormat pixelFormat)
{
   switch (pixelFormat)
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

///////////////////////////////////////////////////////////////////////////////

static tResult UploadTextureImage(cImageData * pImage, GLuint * pTextureId)
{
   Assert(pImage != NULL);
   Assert(pTextureId != NULL);

   if (pImage == NULL || pTextureId == NULL)
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

   glGenTextures(1, pTextureId);
   glBindTexture(GL_TEXTURE_2D, *pTextureId);

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

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTexture
//

class cTexture : public cComObject<IMPLEMENTS(ITexture)>
{
public:
   cTexture();
   cTexture(uint textureId);

   virtual void OnFinalRelease();

   virtual tResult GetTextureId(uint * pTextureId) const
   {
      Assert(pTextureId != NULL);
      *pTextureId = m_textureId;
      return S_OK;
   }

private:
   uint m_textureId;
   bool m_bOwnTextureId;
};

///////////////////////////////////////

cTexture::cTexture()
 : m_textureId(0),
   m_bOwnTextureId(false)
{
}

///////////////////////////////////////

cTexture::cTexture(uint textureId)
 : m_textureId(textureId),
   m_bOwnTextureId(false)
{
}

///////////////////////////////////////

void cTexture::OnFinalRelease()
{
   if (m_bOwnTextureId)
   {
      if (glIsTexture(m_textureId))
         glDeleteTextures(1, &m_textureId);
   }
   m_textureId = 0;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTextureManager
//

class cTextureManager : public cGlobalObject<IMPLEMENTS(ITextureManager)>
{
public:
   cTextureManager();
   ~cTextureManager();

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult GetTexture(const char * pszName, ITexture * * ppTexture);

   virtual tResult FreeTexture(ITexture * pTexture);
   virtual tResult FreeAll();

private:
   struct sTextureInfo
   {
      GLuint textureId;
      ulong refCount;
   };
   typedef std::map<cStr, sTextureInfo> tTextureObjectMap;
   tTextureObjectMap m_textureObjectMap;
};

///////////////////////////////////////

cTextureManager::cTextureManager()
{
}

///////////////////////////////////////

cTextureManager::~cTextureManager()
{
}

///////////////////////////////////////

tResult cTextureManager::Init()
{
   return S_OK;
}

///////////////////////////////////////

tResult cTextureManager::Term()
{
   FreeAll();
   return S_OK;
}

///////////////////////////////////////

tResult cTextureManager::GetTexture(const char * pszName, ITexture * * ppTexture)
{
   Assert(pszName != NULL);
   Assert(ppTexture != NULL);

   tTextureObjectMap::iterator iter = m_textureObjectMap.find(pszName);
   if (iter == m_textureObjectMap.end())
   {
      UseGlobal(ResourceManager);
      cImageData * pImage = ImageLoad(pResourceManager, pszName);
      if (pImage == NULL)
      {
         *ppTexture = NULL;
         return E_FAIL;
      }

      sTextureInfo textureInfo;
      if (UploadTextureImage(pImage, &textureInfo.textureId) != S_OK)
      {
         delete pImage;
         *ppTexture = NULL;
         return E_FAIL;
      }

      delete pImage;

      textureInfo.refCount = 1;
      m_textureObjectMap.insert(std::make_pair(cStr(pszName), textureInfo));

      *ppTexture = new cTexture(textureInfo.textureId);
      return ((*ppTexture) != NULL) ? S_OK : E_FAIL;
   }
   else
   {
      iter->second.refCount++;
      *ppTexture = new cTexture(iter->second.textureId);
      return ((*ppTexture) != NULL) ? S_OK : E_FAIL;
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cTextureManager::FreeTexture(ITexture * pTexture)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cTextureManager::FreeAll()
{
   std::vector<GLuint> textureIds(m_textureObjectMap.size());

   uint i;
   tTextureObjectMap::iterator iter;
   for (i = 0, iter = m_textureObjectMap.begin(); iter != m_textureObjectMap.end(); iter++, i++)
   {
      textureIds[i] = iter->second.textureId;
   }

   glDeleteTextures(textureIds.size(), &textureIds[0]);

   DebugMsgIf(glGetError() != GL_NO_ERROR, "WARNING: glDeleteTextures call caused an error\n");

   return S_OK;
}

///////////////////////////////////////

void TextureManagerCreate()
{
   cAutoIPtr<ITextureManager>(new cTextureManager);
}

///////////////////////////////////////////////////////////////////////////////
