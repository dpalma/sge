///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENGINE_H
#define INCLUDED_ENGINE_H

#include "engineapi.h"

#include "globalobj.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEngine
//

class cEngine : public cGlobalObject<IMPLEMENTS(IEngine)>
{
public:
   cEngine();
   ~cEngine();

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult Startup(IEngineConfiguration * pConfiguration);
   virtual tResult Shutdown();

   virtual tResult GetRenderDevice(IRenderDevice * * ppRenderDevice);

private:
   cAutoIPtr<IRenderDevice> m_pRenderDevice;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENGINE_H
