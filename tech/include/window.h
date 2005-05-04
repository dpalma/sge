///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_WINDOW_H
#define INCLUDED_WINDOW_H

#include "techdll.h"
#include "comtools.h"
#include "connpt.h"
#include "techstring.h"

#ifdef _MSC_VER
#pragma once
#endif

// forward declarations
F_DECLARE_INTERFACE(IWindow);
F_DECLARE_INTERFACE(IWindowFullScreen);
F_DECLARE_INTERFACE(IWindowSink);
#ifdef _WIN32
F_DECLARE_HANDLE(HWND);
F_DECLARE_INTERFACE(IWindowW32);
#else
F_DECLARE_INTERFACE(IWindowX11);
typedef unsigned long XID;
typedef XID Window;
typedef struct _XDisplay Display;
typedef union _XEvent XEvent;
#endif

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IWindow
//

enum eWindowCreateFlags
{
   kWCF_None = 0,
   kWCF_CreateGlContext = 1 << 0
};

struct sWindowCreateParams
{
   int width;
   int height;
   int bpp;
   uint flags;
   cStr title;
};

interface IWindow : IUnknown
{
   DECLARE_CONNECTION_POINT(IWindowSink);

   virtual tResult Create(const sWindowCreateParams * pParams) = 0;

   virtual tResult SwapBuffers() = 0;
};

TECH_API IWindow * WindowCreate();
TECH_API IWindow * WindowCreate(const sWindowCreateParams * pParams);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IWindowFullScreen
//

interface IWindowFullScreen : IUnknown
{
   virtual tResult BeginFullScreen() = 0;
   virtual tResult EndFullScreen() = 0;
};

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IWindowSink
//

interface IWindowSink : IUnknown
{
   virtual void OnKeyEvent(long key, bool down, double time) = 0;
   virtual void OnMouseEvent(int x, int y, uint mouseState, double time) = 0;
   virtual void OnDestroy(double time) = 0;
   virtual void OnResize(int width, int height, double time) = 0;
   virtual void OnActivateApp(bool bActive, double time) = 0;
};

class cDefaultWindowSink : public IWindowSink
{
   virtual void OnKeyEvent(long key, bool down, double time) {}
   virtual void OnMouseEvent(int x, int y, uint mouseState, double time) {}
   virtual void OnDestroy(double time) {}
   virtual void OnResize(int width, int height, double time) {}
   virtual void OnActivateApp(bool bActive, double time) {}
};

#ifdef _WIN32

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IWindowW32
//

interface IWindowW32 : IUnknown
{
   virtual tResult GetHwnd(HWND * phWnd) = 0;
};

#else

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IWindowX11
//

interface IWindowX11 : IWindow
{
   virtual void DispatchEvent(const XEvent * pXEvent) = 0;
};

TECH_API IWindowX11 * FindWindow(Window window);
TECH_API tResult GetDisplay(Display * * ppDisplay);

#endif

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_WINDOW_H
