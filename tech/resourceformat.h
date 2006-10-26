///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RESOURCEFORMAT_H
#define INCLUDED_RESOURCEFORMAT_H

#include "tech/resourceapi.h"

#include <map>
#include <set>

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceFormat
//

class cResourceFormat
{
public:
   void * Load(IReader * pReader) const;
   void * Postload(void * pData, int dataLength, void * loadParam) const;
   void Unload(void * pData) const;

   tResourceType type;
   tResourceType typeDepend; // if not NULL, this format loads 'type' by converting from 'typeDepend'
   uint extensionId;
   tResourceLoad pfnLoad;
   tResourcePostload pfnPostload;
   tResourceUnload pfnUnload;
   void * typeParam;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceFormatTable
//

class cResourceFormatTable
{
public:
   cResourceFormatTable();
   ~cResourceFormatTable();

   tResult RegisterFormat(tResourceType type, tResourceType typeDepend, const tChar * pszExtension,
                          tResourceLoad pfnLoad, tResourcePostload pfnPostload, tResourceUnload pfnUnload,
                          void * typeParam);
   inline tResult RegisterFormat(tResourceType type, tResourceType typeDepend, const tChar * pszExtension,
                          tResourceLoad pfnLoad, tResourcePostload pfnPostload, tResourceUnload pfnUnload)
   {
      return RegisterFormat(type, typeDepend, pszExtension, pfnLoad, pfnPostload, pfnUnload, NULL);
   }
   tResult RevokeFormat(tResourceType type, tResourceType typeDepend, const tChar * pszExtension);

   uint DeduceFormats(const tChar * pszName, tResourceType type, uint * pFormatIds, uint nMaxFormats);

   cResourceFormat * GetFormat(uint formatId) { return &m_formats[formatId]; }
   const cResourceFormat * GetFormat(uint formatId) const { return &m_formats[formatId]; }

   void GetExtensionsForType(tResourceType type, std::set<uint> * pExtensionIds) const;

   const tChar * GetExtension(uint extensionId) const { return m_extensions[extensionId].c_str(); }
   uint GetExtensionId(const tChar * pszExtension);
   uint GetExtensionIdForName(const tChar * pszName);

   void DumpFormats() const;

private:
   typedef std::vector<cResourceFormat> tResourceFormats;
   tResourceFormats m_formats;

   typedef std::vector<cStr> tExtensions;
   tExtensions m_extensions;

   typedef std::multimap<cStr, uint> tExtensionsForType;
   tExtensionsForType m_extsForType;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RESOURCEFORMAT_H
