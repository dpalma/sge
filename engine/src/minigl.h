///////////////////////////////////////////////////////////////////////////////
// TEMPORARY

#ifndef INCLUDED_MINIGL_H
#define INCLUDED_MINIGL_H

#if defined(_WIN32)
   #define DECLSPEC_IMPORT __declspec(dllimport)
   #define WINGDIAPI DECLSPEC_IMPORT
   #define WINAPI __stdcall
   #define APIENTRY WINAPI
   typedef long LONG;
   typedef short SHORT;
   #ifdef VOID
   #undef VOID
   #endif
   typedef void VOID;
#endif

#ifdef _WINDOWS_
#define WINDOWS_H_ALREADY_INCLUDED
#endif

#include <GL/gl.h>

#ifndef _WIN32
#include <GL/glx.h>
#endif

#if defined(_WINDOWS_) && !defined(WINDOWS_H_ALREADY_INCLUDED)
#error Unauthorized inclusion of windows.h!
#endif

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MINIGL_H
