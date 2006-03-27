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

#define kRT_AsciiText      _T("AsciiText")
#define kRT_UnicodeText    _T("UnicodeText")
#ifdef _UNICODE
#define kRT_Text           kRT_UnicodeText
#else
#define kRT_Text           kRT_AsciiText
#endif
TECH_API tResult TextFormatRegister(const tChar * pszExtension);


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceKey
//

class cResourceKey
{
public:
   cResourceKey(const tChar * pszName, const tChar * pszType);
   cResourceKey(const cResourceKey & other);

   const cResourceKey & operator =(const cResourceKey & other);

   bool operator ==(const cResourceKey & other);

   const tChar * GetName() const;
   const tChar * GetType() const;

private:
   cStr m_name, m_type;
};

///////////////////////////////////////

typedef cResourceKey tResKey;

///////////////////////////////////////

inline cResourceKey::cResourceKey(const tChar * pszName, const tChar * pszType)
 : m_name(pszName != NULL ? pszName : _T("")),
   m_type(pszType != NULL ? pszType : _T(""))
{
}

///////////////////////////////////////

inline cResourceKey::cResourceKey(const cResourceKey & other)
 : m_name(other.m_name),
   m_type(other.m_type)
{
}

///////////////////////////////////////

inline const cResourceKey & cResourceKey::operator =(const cResourceKey & other)
{
   m_name = other.m_name;
   m_type = other.m_type;
   return *this;
}

///////////////////////////////////////

inline bool cResourceKey::operator ==(const cResourceKey & other)
{
   return (m_name == other.m_name) && (m_type == other.m_type);
}

///////////////////////////////////////

inline const tChar * cResourceKey::GetName() const
{
   return m_name.c_str();
}

///////////////////////////////////////

inline const tChar * cResourceKey::GetType() const
{
   return m_type.c_str();
}


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IResourceManager
//

typedef const tChar * tResourceType;

typedef void * (* tResourceLoad)(IReader * pReader);
typedef void * (* tResourcePostload)(void * pData, int dataLength, void * param);
typedef void   (* tResourceUnload)(void * pData);

interface IResourceManager : IUnknown
{
   virtual tResult AddDirectory(const tChar * pszDir) = 0;
   // HACK: This AddDirectoryTreeFlattened is to support the legacy behavior
   virtual tResult AddDirectoryTreeFlattened(const tChar * pszDir) = 0;
   virtual tResult AddArchive(const tChar * pszArchive) = 0;

   virtual tResult Load(const tChar * pszName, tResourceType type, void * param, void * * ppData) = 0;
   virtual tResult Unload(const tChar * pszName, tResourceType type) = 0;

   tResult Load(const tResKey & key, void * param, void * * ppData)
   {
      return Load(key.GetName(), key.GetType(), param, ppData);
   }

   tResult Load(const tResKey & key, void * * ppData)
   {
      return Load(key, NULL, ppData);
   }

   tResult Unload(const tResKey & key)
   {
      return Unload(key.GetName(), key.GetType());
   }

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

   /// @brief Returns an inexact list of resources of the given type.
   /// @remarks The list is inexact because the type of a resource is not
   /// strictly known when it is pre-loaded because the only information 
   /// available at start-up is the file extension. This method is very slow
   /// and is intended for use by the editor for populating user-interface elements.
   virtual tResult ListResources(tResourceType type, std::vector<cStr> * pNames) const = 0;
};

////////////////////////////////////////

#define kResourceManagerName _T("ResourceManager")
TECH_API tResult ResourceManagerCreate();


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RESOURCEAPI_H
