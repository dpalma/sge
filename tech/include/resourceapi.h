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
// INTERFACE: IResourceManager
//

typedef const tChar * tResourceType;

typedef void * (* tResourceLoadNoParam)(IReader * pReader);

typedef void * (* tResourceLoad)(IReader * pReader, void * typeParam);
typedef void * (* tResourcePostload)(void * pData, int dataLength, void * loadParam);
typedef void   (* tResourceUnload)(void * pData);

TECH_API void * ThunkResourceLoadNoParam(IReader * pReader, void * typeParam);

interface IResourceManager : IUnknown
{
   virtual tResult AddDirectory(const tChar * pszDir) = 0;
   // HACK: This AddDirectoryTreeFlattened is to support the legacy behavior
   virtual tResult AddDirectoryTreeFlattened(const tChar * pszDir) = 0;
   virtual tResult AddArchive(const tChar * pszArchive) = 0;

   virtual tResult Load(const tChar * pszName, tResourceType type, void * loadParam, void * * ppData) = 0;
   virtual tResult Unload(const tChar * pszName, tResourceType type) = 0;

   virtual tResult RegisterFormat(tResourceType type,
                                  tResourceType typeDepend,
                                  const tChar * pszExtension,
                                  tResourceLoad pfnLoad,
                                  tResourcePostload pfnPostload,
                                  tResourceUnload pfnUnload,
                                  void * typeParam) = 0;

   inline tResult RegisterFormat(tResourceType type, const tChar * pszExtension,
      tResourceLoad pfnLoad, tResourcePostload pfnPostload, tResourceUnload pfnUnload, void * typeParam)
   {
      return RegisterFormat(type, NULL, pszExtension, pfnLoad, pfnPostload, pfnUnload, typeParam);
   }

   inline tResult RegisterFormat(tResourceType type, tResourceType typeDepend, const tChar * pszExtension,
      tResourceLoadNoParam pfnLoad, tResourcePostload pfnPostload, tResourceUnload pfnUnload)
   {
      return RegisterFormat(type, typeDepend, pszExtension, ThunkResourceLoadNoParam, pfnPostload, pfnUnload, pfnLoad);
   }

   inline tResult RegisterFormat(tResourceType type, const tChar * pszExtension,
      tResourceLoadNoParam pfnLoad, tResourcePostload pfnPostload, tResourceUnload pfnUnload)
   {
      return RegisterFormat(type, NULL, pszExtension, ThunkResourceLoadNoParam, pfnPostload, pfnUnload, pfnLoad);
   }

   virtual tResult ListResources(const tChar * pszMatch, std::vector<cStr> * pNames) const = 0;
};

////////////////////////////////////////

#define kResourceManagerName _T("ResourceManager")
TECH_API tResult ResourceManagerCreate();


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RESOURCEAPI_H
