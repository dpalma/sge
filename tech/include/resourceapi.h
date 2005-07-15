///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RESOURCEAPI_H
#define INCLUDED_RESOURCEAPI_H

/// @file resourceapi.h
/// Interface and class declarations for the caching resource loader

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
   kRC_Text,
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
   cResourceKey(const tChar * pszName, eResourceClass rc);
   cResourceKey(const cResourceKey & other);

   const cResourceKey & operator =(const cResourceKey & other);

   bool operator ==(const cResourceKey & other);

   const tChar * GetName() const;
   eResourceClass GetClass() const;

private:
   cStr m_name;
   eResourceClass m_class;
};

///////////////////////////////////////

typedef cResourceKey tResKey;

///////////////////////////////////////

inline cResourceKey::cResourceKey(const tChar * pszName, eResourceClass rc)
 : m_name(pszName != NULL ? pszName : _T("")),
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

inline const tChar * cResourceKey::GetName() const
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

typedef const char * tResourceType;

#define MAKERESOURCETYPE(i) (tResourceType)((ulong *)((ulong)(i)))

typedef void * (* tResourceLoad)(IReader * pReader);
typedef void * (* tResourcePostload)(void * pData, int dataLength, void * param);
typedef void   (* tResourceUnload)(void * pData);

interface IResourceManager : IUnknown
{
   virtual tResult AddDirectory(const tChar * pszDir) = 0;
   // HACK: This AddDirectoryTreeFlattened is to support the legacy behavior
   virtual tResult AddDirectoryTreeFlattened(const tChar * pszDir) = 0;
   virtual tResult AddArchive(const tChar * pszArchive) = 0;

   /// @brief Load the resource from disk, bypassing the cache (not updating it either)
   /// @remarks The caller is expected know what is returned in ppData and how to clean it up.
   virtual tResult LoadUncached(const tChar * pszName, tResourceType type, void * param, void * * ppData, ulong * pDataSize) = 0;

   virtual tResult Load(const tChar * pszName, tResourceType type, void * param, void * * ppData) = 0;
   virtual tResult Unload(const tChar * pszName, tResourceType type) = 0;

   tResult LoadUncached(const tResKey & key, void * param, void * * ppData, ulong * pDataSize)
   {
      return LoadUncached(key.GetName(), MAKERESOURCETYPE(key.GetClass()), param, ppData, pDataSize);
   }

   tResult LoadUncached(const tResKey & key, void * * ppData)
   {
      return LoadUncached(key, NULL, ppData, NULL);
   }

   tResult Load(const tResKey & key, void * param, void * * ppData)
   {
      return Load(key.GetName(), MAKERESOURCETYPE(key.GetClass()), param, ppData);
   }

   tResult Load(const tResKey & key, void * * ppData)
   {
      return Load(key, NULL, ppData);
   }

   tResult Unload(const tResKey & key)
   {
      return Unload(key.GetName(), MAKERESOURCETYPE(key.GetClass()));
   }

   /// @remarks Not implemented
   virtual tResult Lock(const tChar * pszName, tResourceType type) = 0;
   /// @remarks Not implemented
   virtual tResult Unlock(const tChar * pszName, tResourceType type) = 0;

   virtual tResult RegisterFormat(tResourceType type,
                                  tResourceType typeDepend,
                                  const tChar * pszExtension,
                                  tResourceLoad pfnLoad,
                                  tResourcePostload pfnPostload,
                                  tResourceUnload pfnUnload) = 0;

   tResult RegisterFormat(tResourceType type, const tChar * pszExtension,
      tResourceLoad pfnLoad, tResourcePostload pfnPostload, tResourceUnload pfnUnload)
   {
      return RegisterFormat(type, NULL, pszExtension, pfnLoad, pfnPostload, pfnUnload);
   }

   tResult RegisterFormat(eResourceClass rc,
                          eResourceClass rcDepend,
                          const tChar * pszExtension,
                          tResourceLoad pfnLoad,
                          tResourcePostload pfnPostload,
                          tResourceUnload pfnUnload)
   {
      return RegisterFormat(MAKERESOURCETYPE(rc), MAKERESOURCETYPE(rcDepend),
         pszExtension, pfnLoad, pfnPostload, pfnUnload);
   }

   tResult RegisterFormat(eResourceClass rc, const tChar * pszExtension,
      tResourceLoad pfnLoad, tResourcePostload pfnPostload, tResourceUnload pfnUnload)
   {
      return RegisterFormat(MAKERESOURCETYPE(rc), pszExtension, pfnLoad, pfnPostload, pfnUnload);
   }
};

////////////////////////////////////////

#define kResourceManagerName _T("ResourceManager")
TECH_API void ResourceManagerCreate();


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RESOURCEAPI_H
