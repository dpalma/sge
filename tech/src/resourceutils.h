///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RESOURCEUTILS_H
#define INCLUDED_RESOURCEUTILS_H

#include "resourceapi.h"

#ifdef _MSC_VER
#pragma once
#endif

class cFilePath;
class cResourceStore;

F_DECLARE_INTERFACE(IResourceManagerDiagnostics);

const uint kNoIndex = ~0u;

///////////////////////////////////////////////////////////////////////////////

inline const tChar * ResourceTypeName(tResourceType resourceType)
{
   return resourceType;
}

////////////////////////////////////////////////////////////////////////////////

inline bool SameType(tResourceType lhs, tResourceType rhs)
{
   if (lhs != NULL && rhs != NULL)
   {
      return _tcscmp(lhs, rhs) == 0;
   }
   else if (lhs == NULL && rhs == NULL)
   {
      return true;
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////

typedef std::vector<cStr> tStrings;
tStrings::size_type ListDirs(const cFilePath & path, bool bSkipHidden, tStrings * pDirs);


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceCacheKey
//

class cResourceCacheKey
{
public:
   cResourceCacheKey(const tChar * pszName, tResourceType type);
   cResourceCacheKey(const cResourceCacheKey &);
   virtual ~cResourceCacheKey();

   const cResourceCacheKey & operator =(const cResourceCacheKey & other);

   bool operator ==(const cResourceCacheKey & other) const;
   bool operator <(const cResourceCacheKey & other) const;

   inline const tChar * GetName() const { return m_name.c_str(); }
   inline tResourceType GetType() const { return m_type; }

private:
   cStr m_name;
   tResourceType m_type;
};


///////////////////////////////////////////////////////////////////////////////
//
// STRUCT: sResource
//

struct sResource
{
   sResource();
   sResource(const sResource &);
   ~sResource();

   const sResource & operator =(const sResource &);

   cStr name;
   uint extensionId;
   uint formatId;
   cResourceStore * pStore;
   void * pData;
   ulong dataSize;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IResourceManagerDiagnostics
//

interface IResourceManagerDiagnostics : IUnknown
{
   virtual void DumpFormats() const = 0;
   virtual void DumpCache() const = 0;
   virtual size_t GetCacheSize() const = 0;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RESOURCEUTILS_H
