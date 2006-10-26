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

#ifndef FASTCALL
#ifdef _MSC_VER
#define FASTCALL __fastcall
#else
#define FASTCALL
#endif
#endif

///////////////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
typedef wchar_t   tChar;
#else
typedef char      tChar;
#endif

#ifndef _T
#ifdef _UNICODE
#define _T(s) L##s
#else
#define _T(s) s
#endif
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

// pointer-sized numeric types
#ifdef _WIN64
typedef __int64            int_ptr;
typedef unsigned __int64   uint_ptr;
typedef __int64            long_ptr;
typedef unsigned __int64   ulong_ptr;
#else
typedef int                int_ptr;
typedef unsigned int       uint_ptr;
typedef long               long_ptr;
typedef unsigned long      ulong_ptr;
#endif

///////////////////////////////////////

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

#ifdef __GNUC__
#define _stricmp strcasecmp
#define _snprintf snprintf
#ifdef _UNICODE
#define _fgettc      fgetwc
#define _fgetts      fgetws
#define _ftprintf    fwprintf
#define _istalpha    iswalpha
#define _istspace    iswspace
#define _sntprintf   snwprintf
#define _stscanf     swscanf
#define _tcscat      wcscat
#define _tcschr      wcschr
#define _tcscmp      wcscmp
#define _tcscpy      wcscpy
#define _tcscspn     wcscspn
#define _tcsdec(s,p) (((p)>(s))?((p)-1):(s))
#define _tcsicmp     wcscasecmp
#define _tcsinc(p)   ((p)+1)
#define _tcslen      wcslen
#define _tcsncat     wcsncat
#define _tcsncmp     wcsncmp
#define _tcsncpy     wcsncpy
#define _tcsnicmp    wcsncasecmp
#define _tcspbrk     wcspbrk
#define _tcsrchr     wcsrchr
#define _tcsstr      wcsstr
#define _tfopen      wfopen
#define _vsntprintf  vsnwprintf
#else
#define _fgettc      fgetc
#define _fgetts      fgets
#define _ftprintf    fprintf
#define _istalpha    isalpha
#define _istspace    isspace
#define _sntprintf   snprintf
#define _stscanf     sscanf
#define _tcscat      strcat
#define _tcschr      strchr
#define _tcscmp      strcmp
#define _tcscpy      strcpy
#define _tcscspn     strcspn
#define _tcsdec(s,p) (((p)>(s))?((p)-1):(s))
#define _tcsicmp     strcasecmp
#define _tcsinc(p)   ((p)+1)
#define _tcslen      strlen
#define _tcsncat     strncat
#define _tcsncmp     strncmp
#define _tcsncpy     strncpy
#define _tcsnicmp    strncasecmp
#define _tcspbrk     strpbrk
#define _tcsrchr     strrchr
#define _tcsstr      strstr
#define _tfopen      fopen
#define _vsntprintf  vsnprintf
#endif
#endif

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TECHTYPES_H
