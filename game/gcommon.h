///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GCOMMON_H
#define INCLUDED_GCOMMON_H

#include "combase.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IRenderDevice);
F_DECLARE_INTERFACE(IResourceManager);

///////////////////////////////////////////////////////////////////////////////
// main.cpp

IRenderDevice * AccessRenderDevice();
IResourceManager * AccessResourceManager();

bool MainInit(int argc, char * argv[]);
void MainTerm();
void MainFrame();

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GCOMMON_H
