///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SYS_H
#define INCLUDED_SYS_H

#include "enginedll.h"

#include "comtools.h"
#include "techstring.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IDirect3DDevice9);

///////////////////////////////////////////////////////////////////////////////
// implemented in syswin.cpp, syslinux.cpp, or other OS-specific implementation file

enum eSys3DAPI
{
   kOpenGL,
   kDirect3D9,
};

ENGINE_API tResult SysRunUnitTests();
ENGINE_API void SysAppActivate(bool active);
ENGINE_API void SysQuit();
ENGINE_API tResult SysGetClipboardString(cStr * pStr, ulong max = 8192); // 8K max for sanity
ENGINE_API tResult SysSetClipboardString(const tChar * psz);
ENGINE_API HANDLE SysCreateWindow(const tChar * pszTitle, int width, int height, eSys3DAPI api = kOpenGL);
ENGINE_API tResult SysGetDirect3DDevice9(IDirect3DDevice9 * * ppDevice);
ENGINE_API void SysSwapBuffers();
ENGINE_API int SysEventLoop(bool (* pfnFrameHandler)(), void (* pfnResizeHack)(int, int));
ENGINE_API void SysReportFrameStats(tChar * psz, ulong max);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SYS_H
