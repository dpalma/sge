///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENGINEAPI_H
#define INCLUDED_ENGINEAPI_H

/// @file engineapi.h
/// All-encompassing front-end for engine features 

#include "enginedll.h"
#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IEngine);
F_DECLARE_INTERFACE(IEngineConfiguration);

class cStr;
struct sRenderDeviceParameters;
F_DECLARE_INTERFACE(IRenderDevice);

///////////////////////////////////////////////////////////////////////////////

enum eRenderDevice
{
   kRD_OpenGL,
   kRD_Direct3D
};

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEngine
//

interface IEngine : IUnknown
{
   virtual tResult Startup(IEngineConfiguration * pConfiguration) = 0;
   virtual tResult Shutdown() = 0;

   virtual tResult GetRenderDevice(IRenderDevice * * ppRenderDevice) = 0;
};

///////////////////////////////////////

ENGINE_API void EngineCreate();


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEngineConfiguration
//

interface IEngineConfiguration : IUnknown
{
   virtual tResult GetStartupScript(cStr * pScript) = 0;

   virtual tResult GetPreferredRenderDevice(uint * pPreferDevice) = 0;
   virtual tResult GetRenderDeviceParameters(sRenderDeviceParameters * pParams) = 0;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENGINEAPI_H
