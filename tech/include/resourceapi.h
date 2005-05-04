///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RESOURCEAPI_H
#define INCLUDED_RESOURCEAPI_H

#include "techdll.h"
#include "comtools.h"
#include "techstring.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IReader);

F_DECLARE_INTERFACE(IResourceManager);

///////////////////////////////////////////////////////////////////////////////

TECH_API tResult TargaFormatRegister();
TECH_API tResult BmpFormatRegister();
TECH_API tResult TextFormatRegister(const char * pszExtension);

///////////////////////////////////////////////////////////////////////////////

enum eResourceClass
{
   kRC_Unknown,
   kRC_Image,
   kRC_Texture,
   kRC_Mesh,
   kRC_Text,
   kRC_Font,
   kRC_TiXml,        // a tinyxml document
   kRC_GlTexture,    // a GL texture handle (GLuint)
   kRC_HBitmap,      // a Windows HBITMAP
   kRC_CgProgram,    // a Cg program
   kRC_CgEffect,     // a CgFX effect
   // Add new values above this line
   kNUMRESOURCECLASSES,
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceKey
//

class cResourceKey
{
public:
   cResourceKey(const char * pszName, eResourceClass rc);
   cResourceKey(const cResourceKey & other);

   const cResourceKey & operator =(const cResourceKey & other);

   bool operator ==(const cResourceKey & other);

   const char * GetName() const;
   eResourceClass GetClass() const;

private:
   cStr m_name;
   eResourceClass m_class;
};

///////////////////////////////////////

typedef cResourceKey tResKey;

///////////////////////////////////////

inline cResourceKey::cResourceKey(const char * pszName, eResourceClass rc)
 : m_name(pszName != NULL ? pszName : ""),
   m_class(rc)
{
}

///////////////////////////////////////

inline cResourceKey::cResourceKey(const cResourceKey & other)
 : m_name(other.m_name),
   m_class(other.m_class)
{
}

///////////////////////////////////////

inline const cResourceKey & cResourceKey::operator =(const cResourceKey & other)
{
   m_name = other.m_name;
   m_class = other.m_class;
   return *this;
}

///////////////////////////////////////

inline bool cResourceKey::operator ==(const cResourceKey & other)
{
   return (m_name == other.m_name) && (m_class == other.m_class);
}

///////////////////////////////////////

inline const char * cResourceKey::GetName() const
{
   return m_name.c_str();
}

///////////////////////////////////////

inline eResourceClass cResourceKey::GetClass() const
{
   return m_class;
}


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IResourceManager
//

typedef void * (* tResourceLoad)(IReader * pReader);
typedef void * (* tResourcePostload)(void * pData, int dataLength, void * param);
typedef void   (* tResourceUnload)(void * pData);

interface IResourceManager : IUnknown
{
   virtual tResult AddDirectory(const char * pszDir) = 0;
   // HACK: This AddDirectoryTreeFlattened is to support the legacy behavior
   virtual tResult AddDirectoryTreeFlattened(const char * pszDir) = 0;
   virtual tResult AddArchive(const char * pszArchive) = 0;

   // TODO: Make this type-safe. Maybe pass in a GUID sort of like QueryInterface
   virtual tResult Load(const tResKey & key, void * param, void * * ppData) = 0;
   virtual tResult Unload(const tResKey & key) = 0;

   tResult Load(const tResKey & key, void * * ppData)
   {
      return Load(key, NULL, ppData);
   }

   virtual tResult Lock(const tResKey & key) = 0;
   virtual tResult Unlock(const tResKey & key) = 0;

   virtual tResult RegisterFormat(eResourceClass rc,
                                  eResourceClass rcDepend,
                                  const char * pszExtension,
                                  tResourceLoad pfnLoad,
                                  tResourcePostload pfnPostload,
                                  tResourceUnload pfnUnload) = 0;

   tResult RegisterFormat(eResourceClass rc, const char * pszExtension,
      tResourceLoad pfnLoad, tResourcePostload pfnPostload, tResourceUnload pfnUnload)
   {
      return RegisterFormat(rc, kRC_Unknown, pszExtension, pfnLoad, pfnPostload, pfnUnload);
   }
};

////////////////////////////////////////

#define kResourceManagerName "ResourceManager"
TECH_API void ResourceManagerCreate();


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RESOURCEAPI_H
