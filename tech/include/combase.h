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
// Macros for defining COM error codes. Taken from winerror.h.
//

#ifdef RC_INVOKED
#define _HRESULT_TYPEDEF_(_sc) _sc
#else // RC_INVOKED
#define _HRESULT_TYPEDEF_(_sc) ((HRESULT)_sc)
#endif // RC_INVOKED

// Windows SDK version of <winerror.h> defines _WINERROR_
// Cygwin version of <winerror.h> defines _WINERROR_H
#if !defined(_WINERROR_) && !defined(_WINERROR_H)
#define S_OK                              ((HRESULT)0x00000000L)
#define S_FALSE                           ((HRESULT)0x00000001L)
#define E_UNEXPECTED                      _HRESULT_TYPEDEF_(0x8000FFFFL)
#if defined(_WIN32) && !defined(_MAC)
#define E_NOTIMPL                         _HRESULT_TYPEDEF_(0x80004001L)
#define E_OUTOFMEMORY                     _HRESULT_TYPEDEF_(0x8007000EL)
#define E_INVALIDARG                      _HRESULT_TYPEDEF_(0x80070057L)
#define E_NOINTERFACE                     _HRESULT_TYPEDEF_(0x80004002L)
#define E_POINTER                         _HRESULT_TYPEDEF_(0x80004003L)
#define E_HANDLE                          _HRESULT_TYPEDEF_(0x80070006L)
#define E_ABORT                           _HRESULT_TYPEDEF_(0x80004004L)
#define E_FAIL                            _HRESULT_TYPEDEF_(0x80004005L)
#define E_ACCESSDENIED                    _HRESULT_TYPEDEF_(0x80070005L)
#else
#define E_NOTIMPL                         _HRESULT_TYPEDEF_(0x80000001L)
#define E_OUTOFMEMORY                     _HRESULT_TYPEDEF_(0x80000002L)
#define E_INVALIDARG                      _HRESULT_TYPEDEF_(0x80000003L)
#define E_NOINTERFACE                     _HRESULT_TYPEDEF_(0x80000004L)
#define E_POINTER                         _HRESULT_TYPEDEF_(0x80000005L)
#define E_HANDLE                          _HRESULT_TYPEDEF_(0x80000006L)
#define E_ABORT                           _HRESULT_TYPEDEF_(0x80000007L)
#define E_FAIL                            _HRESULT_TYPEDEF_(0x80000008L)
#define E_ACCESSDENIED                    _HRESULT_TYPEDEF_(0x80000009L)
#endif
#endif // !_WINERROR_

typedef long HRESULT;

typedef HRESULT tResult;

// Generic test for success on any status value
// (non-negative numbers indicate success).
#define SUCCEEDED(Status) ((HRESULT)(Status) >= 0)

// and the inverse
#define FAILED(Status) ((HRESULT)(Status)<0)

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

#define STDMETHOD(method)       virtual HRESULT STDMETHODCALLTYPE method
#define STDMETHOD_(type,method) virtual type STDMETHODCALLTYPE method

#ifdef DEFINE_IID_IUNKNOWN
DEFINE_GUID(IID_IUnknown, 0x00000000, 0x0000, 0x0000, 0xC, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x46);
#else
F_DECLARE_GUID(IID_IUnknown);
#endif

#ifndef NO_ULONG
typedef unsigned long ULONG;
#endif

#ifndef __IUnknown_INTERFACE_DEFINED__
#define __IUnknown_INTERFACE_DEFINED__

interface IUnknown
{
   STDMETHOD(QueryInterface)(REFIID riid, void * * ppvObject) PURE;

   STDMETHOD_(ULONG, AddRef)() PURE;

   STDMETHOD_(ULONG, Release)() PURE;
};

typedef IUnknown * LPUNKNOWN;

#endif // !__IUnknown_INTERFACE_DEFINED__

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_COMBASE_H
