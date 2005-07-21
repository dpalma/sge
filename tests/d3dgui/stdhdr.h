///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_STDHDR_H
#define INCLUDED_STDHDR_H

#include "techtypes.h"
#include "techdebug.h"

#include "guiapi.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IGUIFontFactoryD3D);
F_DECLARE_INTERFACE(IDirect3DDevice9);

interface IGUIFontFactoryD3D : IGUIFontFactory
{
   virtual tResult SetD3DDevice(IDirect3DDevice9 * pDevice) = 0;
};

#endif // !INCLUDED_STDHDR_H
