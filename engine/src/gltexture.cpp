///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "textureapi.h"

#include "imagedata.h"
#include "techmath.h"
#include "globalobj.h"
#include "str.h"
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
   cTexture(float priority = kTexPriorityHighest);
   ~cTexture();

   virtual void OnFinalRelease();

   virtual tResult UploadImage(const cImageData * pImageData);

   virtual tResult HasAlphaComponent();

   virtual uint GetWidth();
   virtual uint GetHeight();

   virtual tResult SetPriority(float priority);
   virtual tResult GetPriority(float * pPriority);

   virtual tResult GetTextureHandle(HANDLE * phTexture) const;

   GLuint GetTextureId() const { return m_textureId; }

private:
   void ReleaseTextureId();

   float m_priority;
   bool m_bHasAlpha;
   uint m_width, m_height;
   uint m_textureId;
};

///////////////////////////////////////

cTexture::cTexture(float priority)
 : m_priority(priority),
   m_bHasAlpha(false),
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

tResult cTexture::UploadImage(const cImageData * pImageData)
{
   if (pImageData == NULL)
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

   // TODO: Support kPF_RGBA1555 and kPF_BGRA1555?
   if (pixelFormat == kPF_RGBA8888 || pixelFormat == kPF_BGRA8888)
   {
      m_bHasAlpha = true;
   }
   else
   {
      m_bHasAlpha = false;
   }

   m_width = pImageData->GetWidth();
   m_height = pImageData->GetHeight();

   if ((m_textureId == 0) || !glIsTexture(m_textureId))
   {
      glGenTextures(1, &m_textureId);
   }

   glBindTexture(GL_TEXTURE_2D, m_textureId);

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

///////////////////////////////////////

tResult cTexture::HasAlphaComponent()
{
   return m_bHasAlpha ? S_OK : S_FALSE;
}

///////////////////////////////////////

uint cTexture::GetWidth()
{
   return m_width;
}

///////////////////////////////////////

uint cTexture::GetHeight()
{
   return m_height;
}

///////////////////////////////////////

tResult cTexture::SetPriority(float priority)
{
   if (priority < 0.0f || priority > 1.0f)
   {
      return E_INVALIDARG;
   }
   m_priority = priority;
   return S_OK;
}

///////////////////////////////////////

tResult cTexture::GetPriority(float * pPriority)
{
   if (pPriority == NULL)
   {
      return E_POINTER;
   }
   *pPriority = m_priority;
   return S_OK;
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

   virtual tResult CreateTexture(const char * pszName, const cImageData * pImageData, ITexture * * ppTexture);
   virtual tResult GetTexture(const char * pszName, ITexture * * ppTexture);

   virtual tResult FreeAll();

   virtual tResult PrioritizeTextures();

private:
   typedef std::map<cStr, cTexture *> tTexObjMap;
   tTexObjMap m_texObjMap;
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

tResult cTextureManager::CreateTexture(const char * pszName,
                                       const cImageData * pImageData,
                                       ITexture * * ppTexture)
{
   if (pszName == NULL || pImageData == NULL || ppTexture == NULL)
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
      m_texObjMap.insert(std::make_pair(cStr(pszName), CTAddRef(pTex.operator->())));
      *ppTexture = CTAddRef(pTex);
      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cTextureManager::GetTexture(const char * pszName, ITexture * * ppTexture)
{
   if (pszName == NULL || ppTexture == NULL)
   {
      return E_POINTER;
   }

   tTexObjMap::iterator iter = m_texObjMap.find(pszName);
   if (iter != m_texObjMap.end())
   {
      *ppTexture = CTAddRef(iter->second);
      return S_OK;
   }
   else
   {
      cAutoIPtr<cTexture> pTex(new cTexture);
      if (!pTex)
      {
         return E_OUTOFMEMORY;
      }

      cImageData * pImageData = NULL;
      UseGlobal(ResourceManager);
      if (pResourceManager->Load(tResKey(pszName, kRC_Image), (void**)&pImageData) == S_OK)
      {
         tResult result = E_FAIL;

         if (pTex->UploadImage(pImageData) == S_OK)
         {
            m_texObjMap.insert(std::make_pair(cStr(pszName), CTAddRef(pTex.operator->())));
            *ppTexture = CTAddRef(pTex);
            result = S_OK;
         }

         delete pImageData;

         return result;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cTextureManager::FreeAll()
{
   tTexObjMap::iterator iter;
   for (iter = m_texObjMap.begin(); iter != m_texObjMap.end(); iter++)
   {
      iter->second->Release();
   }
   m_texObjMap.clear();

   return S_OK;
}

///////////////////////////////////////

tResult cTextureManager::PrioritizeTextures()
{
   size_t nTextures = m_texObjMap.size();

   uint * pTextureIds = new uint[nTextures];
   if (pTextureIds == NULL)
   {
      return E_OUTOFMEMORY;
   }

   float * pPriorities = new float[nTextures];
   if (pPriorities == NULL)
   {
      delete [] pTextureIds;
      return E_OUTOFMEMORY;
   }

   int index = 0;
   tTexObjMap::iterator iter;
   for (iter = m_texObjMap.begin(); iter != m_texObjMap.end(); iter++, index++)
   {
      pTextureIds[index] = iter->second->GetTextureId();
      Verify(iter->second->GetPriority(&pPriorities[index]) == S_OK);
   }

   glPrioritizeTextures(nTextures, pTextureIds, pPriorities);

   delete [] pTextureIds;
   delete [] pPriorities;

   return S_OK;
}

///////////////////////////////////////

void TextureManagerCreate()
{
   cAutoIPtr<ITextureManager>(new cTextureManager);
}

///////////////////////////////////////////////////////////////////////////////
