///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RESOURCEAPI_H
#define INCLUDED_RESOURCEAPI_H

#include "techdll.h"
#include "comtools.h"
#include "str.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IReader);

F_DECLARE_INTERFACE(IResource);
F_DECLARE_INTERFACE(IResourceFormat);
F_DECLARE_INTERFACE(IResourceManager2);

///////////////////////////////////////////////////////////////////////////////

enum eResourceStorePriority
{
   kRSP_System,
   kRSP_User
};

enum eResourceClass
{
   kRC_Image,
   kRC_Mesh,
   kRC_Text,
   kRC_Font
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceKey
//

class cResourceKey
{
public:
   cResourceKey(const char * pszName, eResourceClass type);
   cResourceKey(const cResourceKey & other);

   const cResourceKey & operator=(const cResourceKey & other);

   bool operator==(const cResourceKey & other);

   const char * GetName() const;
   eResourceClass GetClass() const;

private:
   cStr m_name;
   eResourceClass m_class;
};

///////////////////////////////////////

typedef cResourceKey tResKey;

///////////////////////////////////////

inline cResourceKey::cResourceKey(const char * pszName, eResourceClass type)
 : m_name(pszName != NULL ? pszName : ""),
   m_class(type)
{
}

///////////////////////////////////////

inline cResourceKey::cResourceKey(const cResourceKey & other)
 : m_name(other.m_name),
   m_class(other.m_class)
{
}

///////////////////////////////////////

inline const cResourceKey & cResourceKey::operator=(const cResourceKey & other)
{
   m_name = other.m_name;
   m_class = other.m_class;
   return *this;
}

///////////////////////////////////////

inline bool cResourceKey::operator==(const cResourceKey & other)
{
   return (m_name.compare(other.m_name) == 0) && (m_class == other.m_class);
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

interface IResourceManager2 : IUnknown
{
   virtual tResult AddResourceStore(const char * pszName, eResourceStorePriority priority) = 0;

   virtual tResult Load(const tResKey & key, IResource * * ppResource) = 0;

   virtual tResult RegisterResourceFormat(eResourceClass resClass,
                                          const char * pszExtension,
                                          IResourceFormat * pResFormat) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IResourceFormat
//

interface IResourceFormat : IUnknown
{
   virtual tResult Load(const tResKey & key, IReader * pReader, IResource * * ppResource) = 0;
};

////////////////////////////////////////

TECH_API tResult RegisterResourceFormat(eResourceClass resClass,
                                        const char * pszExtension,
                                        IResourceFormat * pResFormat);

////////////////////////////////////////

struct sAutoRegisterResourceFormat
{
   sAutoRegisterResourceFormat(eResourceClass resClass,
                               const char * pszExtension,
                               IResourceFormat * pResFormat)
   {
      RegisterResourceFormat(resClass, pszExtension, pResFormat);
      SafeRelease(pResFormat);
   }
};

#define REFERENCE_RESOURCEFORMAT(ext) \
   extern void * ReferenceSymbol##ext(); \
   void * MAKE_UNIQUE(g_pReference##ext) = (void *)&ReferenceSymbol##ext

#define AUTOREGISTER_RESOURCEFORMAT(resClass, ext, resFormatClass) \
   void * ReferenceSymbol##ext() { return NULL; } \
   static sAutoRegisterResourceFormat MAKE_UNIQUE(g_autoRegResFormat)( \
      resClass, #ext, static_cast<IResourceFormat *>(new (resFormatClass)))

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IResource
//

interface IResource : IUnknown
{
   virtual eResourceClass GetClass() const = 0;

   virtual tResult GetData(void * * ppData) = 0;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RESOURCEAPI_H
