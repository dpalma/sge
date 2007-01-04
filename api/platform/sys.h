///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SYS_H
#define INCLUDED_SYS_H

#include "platform/platformdll.h"

#include "tech/comtools.h"
#include "tech/techstring.h"

#ifdef _MSC_VER
#pragma once
#endif

class cFilePath;

F_DECLARE_INTERFACE(IDirect3DDevice9);

#ifdef _WIN32
F_DECLARE_HANDLE(HWND);
#else
struct _XDisplay;
typedef struct _XDisplay Display;
typedef unsigned long XID;
typedef XID Window;
#endif

///////////////////////////////////////////////////////////////////////////////
// implemented in syswin.cpp, syslinux.cpp, or other OS-specific implementation file

enum eSys3DAPI
{
   kOpenGL,
   kDirect3D9,
};

typedef void (* tSysKeyEventFn)(long key, bool down, double time, uint_ptr userData);
typedef void (* tSysMouseEventFn)(int x, int y, uint mouseState, double time, uint_ptr userData);
typedef tResult (* tSysFrameFn)();
typedef void (* tSysResizeFn)(int w, int h, double time);

PLATFORM_API void SysSetKeyEventCallback(tSysKeyEventFn pfn, uint_ptr userData);
PLATFORM_API void SysSetMouseEventCallback(tSysMouseEventFn pfn, uint_ptr userData);
PLATFORM_API tSysFrameFn SysSetFrameCallback(tSysFrameFn pfn);
PLATFORM_API tSysFrameFn SysGetFrameCallback();
PLATFORM_API tSysResizeFn SysSetResizeCallback(tSysResizeFn pfn);

PLATFORM_API tResult SysRunUnitTests();
PLATFORM_API void SysAppActivate(bool active);
PLATFORM_API void SysQuit();
PLATFORM_API tResult SysGetClipboardString(cStr * pStr, ulong max = 8192); // 8K max for sanity
PLATFORM_API tResult SysSetClipboardString(const tChar * psz);
PLATFORM_API tResult SysGetFontPath(cFilePath * pFontPath);
PLATFORM_API tResult SysCreateWindow(const tChar * pszTitle, int width, int height, eSys3DAPI api = kOpenGL);
#ifdef _WIN32
PLATFORM_API HWND SysGetWindowHandle();
#else
// TODO
#endif
PLATFORM_API tResult SysGetDirect3DDevice9(IDirect3DDevice9 * * ppDevice);
PLATFORM_API tResult SysGetWindowSize(int * pWidth, int * pHeight);
PLATFORM_API void SysSwapBuffers();

enum eSysEventLoopFlags
{
   kSELF_None                       = 0,
   kSELF_RunScheduler               = (1 << 0),
   kSELF_ReceiveThreadCalls         = (1 << 1),
};

PLATFORM_API int SysEventLoop(tSysFrameFn pfnFrameHandler, uint flags);
PLATFORM_API void SysReportFrameStats(tChar * psz, ulong max);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SYS_H
