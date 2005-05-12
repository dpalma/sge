///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_TECHTYPES_H
#define INCLUDED_TECHTYPES_H

#ifdef _MSC_VER
#include <tchar.h>
#endif

#include <cstddef>

#ifdef _MSC_VER
#pragma once
#endif

#ifndef STRICT
#define STRICT
#endif

// "identifier was truncated to '255' characters in the browser information"
#pragma warning(disable:4786)

///////////////////////////////////////////////////////////////////////////////

#ifndef CDECL
#ifdef _MSC_VER
#define CDECL __cdecl
#else
#define CDECL
#endif
#endif

#ifndef STDCALL
#ifdef _MSC_VER
#define STDCALL __stdcall
#else
#define STDCALL
#endif
#endif

#ifndef CALLBACK
#ifdef _MSC_VER
#define CALLBACK __stdcall
#else
#define CALLBACK
#endif
#endif

///////////////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
typedef wchar_t   tChar;
#else
typedef char      tChar;
#endif

///////////////////////////////////////////////////////////////////////////////

#define Stringize2(x) #x
#define Stringize(x) Stringize2(x)
#define PragmaMsg(msg) __FILE__ "(" Stringize(__LINE__) ") : " msg

///////////////////////////////////////////////////////////////////////////////

#define TOKEN_PASTE_2(x,y)       x##y
#define TOKEN_PASTE_1(x,y)       TOKEN_PASTE_2(x,y)
#define MAKE_UNIQUE(sym)         TOKEN_PASTE_1(sym,__LINE__)

///////////////////////////////////////////////////////////////////////////////

#ifndef _countof
#define _countof(array) (sizeof(array) / sizeof((array)[0]))
#endif // _countof

///////////////////////////////////////////////////////////////////////////////

#define Max(a,b) ((a)>(b)?(a):(b))
#define Min(a,b) ((a)<(b)?(a):(b))

///////////////////////////////////////////////////////////////////////////////

#ifndef NULL
#define NULL 0
#endif

#ifndef interface
#define interface struct
#endif

typedef unsigned char   byte;

typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;
typedef unsigned long   ulong;

typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;
typedef char            int8;
typedef short           int16;
typedef int             int32;

#if defined(_MSC_VER)
typedef __int64         int64;
#elif defined(__GNUC__)
typedef long long       int64;
#else
#error ("Need platform definition for 64-bit integer")
#endif

typedef float           real32;
typedef double          real64;

#ifndef NO_BOOL
typedef int    BOOL;
#define TRUE   1
#define FALSE  0
#endif // !NO_BOOL

///////////////////////////////////////

#define F_DECLARE_WIN32_STRUCT(type) \
   struct tag##type; \
   typedef struct tag##type type, * LP##type

#ifndef EXTERN_C
#define EXTERN_C extern "C"
#endif

#if !defined(DECLSPEC_DLLIMPORT) && defined(_MSC_VER)
#define DECLSPEC_DLLIMPORT __declspec(dllimport)
#else
#define DECLSPEC_DLLIMPORT
#endif

#if !defined(DECLSPEC_DLLEXPORT) && defined(_MSC_VER)
#define DECLSPEC_DLLEXPORT __declspec(dllexport)
#else
#define DECLSPEC_DLLEXPORT
#endif

#ifdef _DLL
#define CRT_IMPORT DECLSPEC_DLLIMPORT
#else
#define CRT_IMPORT
#endif

typedef void * HANDLE;

#ifdef STRICT
#define F_DECLARE_HANDLE(name) struct name##__; typedef struct name##__ * name
#ifndef DECLARE_HANDLE
#define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
#endif
#else
#define F_DECLARE_HANDLE(name) typedef HANDLE name
#ifndef DECLARE_HANDLE
#define DECLARE_HANDLE(name) typedef HANDLE name
#endif
#endif

///////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <malloc.h>
#define alloca _alloca
#elif defined(__GNUC__)
#include <alloca.h>
#else
#error ("Need compiler-specific function prototype for alloca()")
#endif

#ifndef _MSC_VER
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

#ifdef _MSC_VER
#define snprintf _snprintf
EXTERN_C CRT_IMPORT int CDECL _snprintf(char *, size_t, const char * , ...);
#endif

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TECHTYPES_H
