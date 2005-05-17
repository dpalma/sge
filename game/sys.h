///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SYS_H
#define INCLUDED_SYS_H

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
// implemented in syswin.cpp, syslinux.cpp, or other OS-specific implementation file

void SysAppActivate(bool active);
void SysQuit();
void SysGetMousePos(int * px, int * py);
HANDLE SysGetInstanceHandle();
bool SysGetClipboardString(char * psz, int max);
bool SysSetClipboardString(const char * psz);
HANDLE SysCreateWindow(const tChar * pszTitle, int width, int height);
void SysSwapBuffers();

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SYS_H
