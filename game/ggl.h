///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// Serves as lightweight gl header (so don't have to include windows.h for
// example), has gl extension loading code.

#ifndef INCLUDED_GGL_H
#define INCLUDED_GGL_H

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

#ifdef _DEBUG
#define GlShowError() \
   do { \
      int error = glGetError(); \
      DebugMsgIf1(error != GL_NO_ERROR, "glGetError() = %x\n", error); \
   } while (0)
#else
#define GlShowError()
#endif

inline bool GlTestState(GLenum pname, GLint expected)
{
   int value;
   glGetIntegerv(pname, &value);
   return (glGetError() == GL_NO_ERROR) && (value == expected);
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GGL_H
