///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SYSCOMMON_H
#define INCLUDED_SYSCOMMON_H

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

extern tSysDestroyFn    g_pfnDestroyCallback;
extern tSysCharEventFn  g_pfnCharCallback;
extern tSysKeyEventFn   g_pfnKeyCallback;
extern tSysMouseEventFn g_pfnMouseMoveCallback;
extern tSysMouseEventFn g_pfnMouseCallback;
extern tSysFrameFn      g_pfnFrameCallback;
extern tSysResizeFn     g_pfnResizeCallback;

extern uint_ptr         g_charCallbackUserData;
extern uint_ptr         g_keyCallbackUserData;
extern uint_ptr         g_mouseMoveCallbackUserData;
extern uint_ptr         g_mouseCallbackUserData;

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SYSCOMMON_H
