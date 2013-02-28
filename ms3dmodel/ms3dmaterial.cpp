///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ms3dmodel/ms3dmaterial.h"

#include "tech/filespec.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dMaterial
//

///////////////////////////////////////

cMs3dMaterial::cMs3dMaterial()
 : m_shininess(0)
 , m_transparency(0)
 , m_mode(0)
{
   memset(m_name, 0, sizeof(m_name));
   memset(m_ambient, 0, sizeof(m_ambient));
   memset(m_diffuse, 0, sizeof(m_diffuse));
   memset(m_specular, 0, sizeof(m_specular));
   memset(m_emissive, 0, sizeof(m_emissive));
   memset(m_texture, 0, sizeof(m_texture));
   memset(m_alphamap, 0, sizeof(m_alphamap));
}

///////////////////////////////////////

cMs3dMaterial::cMs3dMaterial(const cMs3dMaterial & other)
 : m_shininess(other.m_shininess)
 , m_transparency(other.m_transparency)
 , m_mode(other.m_mode)
{
   memcpy(m_name, other.m_name, sizeof(m_name));
   memcpy(m_ambient, other.m_ambient, sizeof(m_ambient));
   memcpy(m_diffuse, other.m_diffuse, sizeof(m_diffuse));
   memcpy(m_specular, other.m_specular, sizeof(m_specular));
   memcpy(m_emissive, other.m_emissive, sizeof(m_emissive));
   memcpy(m_texture, other.m_texture, sizeof(m_texture));
   memcpy(m_alphamap, other.m_alphamap, sizeof(m_alphamap));
}

///////////////////////////////////////

cMs3dMaterial::~cMs3dMaterial()
{
}

///////////////////////////////////////

const cMs3dMaterial & cMs3dMaterial::operator =(const cMs3dMaterial & other)
{
   m_shininess = other.m_shininess;
   m_transparency = other.m_transparency;
   m_mode = other.m_mode;
   memcpy(m_name, other.m_name, sizeof(m_name));
   memcpy(m_ambient, other.m_ambient, sizeof(m_ambient));
   memcpy(m_diffuse, other.m_diffuse, sizeof(m_diffuse));
   memcpy(m_specular, other.m_specular, sizeof(m_specular));
   memcpy(m_emissive, other.m_emissive, sizeof(m_emissive));
   memcpy(m_texture, other.m_texture, sizeof(m_texture));
   memcpy(m_alphamap, other.m_alphamap, sizeof(m_alphamap));
   return *this;
}

///////////////////////////////////////

void cMs3dMaterial::GetTexture(cStr * pTexture) const
{
   if (pTexture == NULL)
   {
      return;
   }
   // MilkShape stores texture file names as "./texture.bmp" so run
   // the texture name through cFileSpec to fix it up.
   cFileSpec(m_texture).GetFileNameNoExt(pTexture);
}

///////////////////////////////////////

tResult cReadWriteOps<cMs3dMaterial>::Read(IReader * pReader, cMs3dMaterial * pMaterial)
{
   Assert(pReader != NULL);
   Assert(pMaterial != NULL);

   tResult result = E_FAIL;

   do
   {
      if (pReader->Read(pMaterial->m_name, sizeof(pMaterial->m_name)) != S_OK
         || pReader->Read(pMaterial->m_ambient, sizeof(pMaterial->m_ambient)) != S_OK
         || pReader->Read(pMaterial->m_diffuse, sizeof(pMaterial->m_diffuse)) != S_OK
         || pReader->Read(pMaterial->m_specular, sizeof(pMaterial->m_specular)) != S_OK
         || pReader->Read(pMaterial->m_emissive, sizeof(pMaterial->m_emissive)) != S_OK
         || pReader->Read(&pMaterial->m_shininess) != S_OK
         || pReader->Read(&pMaterial->m_transparency) != S_OK
         || pReader->Read(&pMaterial->m_mode) != S_OK
         || pReader->Read(pMaterial->m_texture, sizeof(pMaterial->m_texture)) != S_OK
         || pReader->Read(pMaterial->m_alphamap, sizeof(pMaterial->m_alphamap)) != S_OK
         )
         break;

      result = S_OK;
   }
   while (0);

   return result;
}


///////////////////////////////////////////////////////////////////////////////
