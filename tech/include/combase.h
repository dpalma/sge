///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_COMBASE_H
#define INCLUDED_COMBASE_H

#include "techdll.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
    #define EXTERN_C    extern "C"
#else
    #define EXTERN_C    extern
#endif

///////////////////////////////////////////////////////////////////////////////
//
// COM error codes. Values should match those in winerror.h.
//

#undef S_OK
#undef S_FALSE
#undef E_UNEXPECTED
#undef E_NOTIMPL
#undef E_OUTOFMEMORY
#undef E_INVALIDARG
#undef E_NOINTERFACE
#undef E_POINTER
#undef E_HANDLE
#undef E_ABORT
#undef E_FAIL
#undef E_ACCESSDENIED

enum eCOMErrorCode
{
   S_OK                              = 0x00000000L,
   S_FALSE                           = 0x00000001L,
   E_UNEXPECTED                      = 0x8000FFFFL,
#if defined(_WIN32) && !defined(_MAC)
   E_NOTIMPL                         = 0x80004001L,
   E_OUTOFMEMORY                     = 0x8007000EL,
   E_INVALIDARG                      = 0x80070057L,
   E_NOINTERFACE                     = 0x80004002L,
   E_POINTER                         = 0x80004003L,
   E_HANDLE                          = 0x80070006L,
   E_ABORT                           = 0x80004004L,
   E_FAIL                            = 0x80004005L,
   E_ACCESSDENIED                    = 0x80070005L,
#else
   E_NOTIMPL                         = 0x80000001L,
   E_OUTOFMEMORY                     = 0x80000002L,
   E_INVALIDARG                      = 0x80000003L,
   E_NOINTERFACE                     = 0x80000004L,
   E_POINTER                         = 0x80000005L,
   E_HANDLE                          = 0x80000006L,
   E_ABORT                           = 0x80000007L,
   E_FAIL                            = 0x80000008L,
   E_ACCESSDENIED                    = 0x80000009L,
#endif
};

typedef long tResult;

// Generic test for success on any status value
// (non-negative numbers indicate success).
inline bool SUCCEEDED(tResult result) { return result >= 0; }

// and the inverse
inline bool FAILED(tResult result) { return result < 0; }

///////////////////////////////////////////////////////////////////////////////

const int kGuidStringLength = 40;

#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID
{
    unsigned long Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
} GUID;
#endif // !GUID_DEFINED

#ifndef _REFGUID_DEFINED
   #define _REFGUID_DEFINED
   #if defined(__cplusplus)
      typedef const GUID & REFGUID;
   #else
      typedef const GUID * REFGUID;
   #endif
#endif // !_REFGUID_DEFINED

#ifndef _REFIID_DEFINED
   typedef GUID IID;
   #define _REFIID_DEFINED
   #if defined(__cplusplus)
      typedef const IID & REFIID;
   #else
      typedef const IID * REFIID;
   #endif
#endif // !_REFIID_DEFINED

///////////////////////////////////////////////////////////////////////////////

#define F_DECLARE_GUID(guid) \
   EXTERN_C const GUID guid

#ifndef DEFINE_GUID
#ifndef INITGUID
// Keep the FAR to be compatible with the <objbase.h> definition and not
// cause a C4005: 'DEFINE_GUID' : macro redefinition warning.
#define FAR 
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
   EXTERN_C const GUID FAR name
#else
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
   EXTERN_C const GUID name \
      = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#endif // INITGUID
#endif // DEFINE_GUID

// Valid for at least n = 0 .. 256 (generated on 6/18/2001)
#define DEFINE_TECH_GUID(name, n) \
   DEFINE_GUID(name, 0x729290c0 + n, 0x623e, 0x11d5, 0x9e, 0x0, 0x0, 0x20, 0x78, 0x1f, 0xa5, 0x2)

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IUnknown
//

#ifndef interface
#define interface struct
#endif

#define F_DECLARE_INTERFACE(iface) \
   F_DECLARE_GUID(IID_##iface); \
   interface iface

#if defined(_MSC_VER)
   #define STDMETHODCALLTYPE       __stdcall
   #define PURE                    = 0
#else
   #define STDMETHODCALLTYPE
   #define PURE                    __attribute__((stdcall)) = 0
#endif

#ifdef DEFINE_IID_IUNKNOWN
DEFINE_GUID(IID_IUnknown, 0x00000000, 0x0000, 0x0000, 0xC, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x46);
#else
F_DECLARE_GUID(IID_IUnknown);
#endif

typedef unsigned long ulong;

#ifndef __IUnknown_INTERFACE_DEFINED__
#define __IUnknown_INTERFACE_DEFINED__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
interface __declspec(uuid("{00000000-0000-0000-0C00-000000000046}")) IUnknown
#else
interface IUnknown
#endif
{
   virtual tResult STDMETHODCALLTYPE QueryInterface(REFIID riid, void * * ppvObject) PURE;

   virtual ulong STDMETHODCALLTYPE AddRef() PURE;

   virtual ulong STDMETHODCALLTYPE Release() PURE;

#ifdef QI_TEMPLATE_METHOD_FOR_ATL
   template<class Q>
   tResult STDMETHODCALLTYPE QueryInterface(Q** pp)
   {
      return QueryInterface(__uuidof(Q), (void **)pp);
   }
#endif
};

typedef IUnknown * LPUNKNOWN;

#endif // !__IUnknown_INTERFACE_DEFINED__

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_COMBASE_H
