///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "engine.h"
#include "guiapi.h"
#include "scriptapi.h"

#include "renderapi.h"

#include "resourceapi.h"
#include "readwriteapi.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

static char * GetEntireContents(IReader * pReader)
{
   Assert(pReader != NULL);

   pReader->Seek(0, kSO_End);
   ulong length;
   if (FAILED(pReader->Tell(&length)))
   {
      return NULL;
   }
   pReader->Seek(0, kSO_Set);

   char * pszContents = new char[length + 1];

   if (pReader->Read(pszContents, length) != S_OK)
   {
      delete [] pszContents;
      return NULL;
   }

   pszContents[length] = 0;

   return pszContents;
}

///////////////////////////////////////////////////////////////////////////////

static bool ScriptExecResource(IScriptInterpreter * pInterpreter, const char * pszResource)
{
   bool bResult = false;

   char * pszCode = NULL;
   UseGlobal(ResourceManager);
   if (pResourceManager->Load(tResKey(pszResource, kRC_Text), (void**)&pszCode) == S_OK)
   {
      bResult = SUCCEEDED(pInterpreter->ExecString(pszCode));
      delete [] pszCode;
   }

   return bResult;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEngine
//

///////////////////////////////////////

BEGIN_CONSTRAINTS()
   AFTER_GUID(IID_IGUIRenderingTools) // TODO: probably not necessary
END_CONSTRAINTS()

///////////////////////////////////////

cEngine::cEngine()
 : cGlobalObject<IMPLEMENTS(IEngine)>("Engine", CONSTRAINTS())
{
}

///////////////////////////////////////

cEngine::~cEngine()
{
}

///////////////////////////////////////

tResult cEngine::Init()
{
   return S_OK;
}

///////////////////////////////////////

tResult cEngine::Term()
{
   return S_OK;
}

///////////////////////////////////////

tResult cEngine::Startup(IEngineConfiguration * pConfiguration)
{
   if (pConfiguration == NULL)
   {
      return E_POINTER;
   }

   if (!!m_pRenderDevice)
   {
      // Already initialized
      return S_FALSE;
   }

   cStr script;
   if (pConfiguration->GetStartupScript(&script) == S_OK && !script.empty())
   {
      UseGlobal(ScriptInterpreter);
      if (pScriptInterpreter->ExecFile(script.c_str()) != S_OK)
      {
         if (!ScriptExecResource(pScriptInterpreter, script.c_str()))
         {
            return E_FAIL;
         }
      }
   }

   uint preferDevice;
   if (pConfiguration->GetPreferredRenderDevice(&preferDevice) == S_OK)
   {
      if (preferDevice == kRD_Direct3D)
      {
         // TODO
      }
      else if (preferDevice == kRD_OpenGL)
      {
         // TODO
      }
   }

   sRenderDeviceParameters params = {0};
   if (pConfiguration->GetRenderDeviceParameters(&params) == S_OK)
   {
      if (RenderDeviceCreate(&params, &m_pRenderDevice) != S_OK)
      {
         return E_FAIL;
      }
   }


   UseGlobal(GUIRenderingTools);
   pGUIRenderingTools->SetRenderDevice(m_pRenderDevice);

   return S_OK;
}

///////////////////////////////////////

tResult cEngine::Shutdown()
{
   SafeRelease(m_pRenderDevice);

   return S_OK;
}

///////////////////////////////////////

tResult cEngine::GetRenderDevice(IRenderDevice * * ppRenderDevice)
{
   return m_pRenderDevice.GetPointer(ppRenderDevice);
}

///////////////////////////////////////

void EngineCreate()
{
   cAutoIPtr<IEngine> p(new cEngine);
}

///////////////////////////////////////////////////////////////////////////////
