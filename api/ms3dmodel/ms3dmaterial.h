///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MS3DMATERIAL_H
#define INCLUDED_MS3DMATERIAL_H

#include "ms3dmodeldll.h"

#include "tech/readwriteapi.h"
#include "tech/techstring.h"

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dMaterial
//

class MS3DMODEL_API cMs3dMaterial
{
   friend class cReadWriteOps<cMs3dMaterial>;

public:
   cMs3dMaterial();
   cMs3dMaterial(const cMs3dMaterial & other);
   ~cMs3dMaterial();

   const cMs3dMaterial & operator =(const cMs3dMaterial & other);

   const char * GetName() const;
   const float * GetDiffuse() const;
   void GetTexture(cStr * pTexture) const;

private:
   char m_name[32];
   float m_ambient[4];
   float m_diffuse[4];
   float m_specular[4];
   float m_emissive[4];
   float m_shininess;
   float m_transparency;
   char m_mode;
   char m_texture[128];
   char m_alphamap[128];
};

///////////////////////////////////////

inline const char * cMs3dMaterial::GetName() const
{
   return m_name;
}

///////////////////////////////////////

inline const float * cMs3dMaterial::GetDiffuse() const
{
   return m_diffuse;
}

///////////////////////////////////////

template <>
class MS3DMODEL_API cReadWriteOps<cMs3dMaterial>
{
public:
   static tResult Read(IReader * pReader, cMs3dMaterial * pMaterial);
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MS3DMATERIAL_H
