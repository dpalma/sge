///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "engine.h"
#include "guiapi.h"
#include "scriptapi.h"

#include "renderapi.h"

#include "resourceapi.h"
#include "readwriteapi.h"

#include <tinyxml.h>

#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include <CgFX/ICgFX.h>
#include <CgFX/ICgFXEffect.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

static bool ScriptExecResource(IScriptInterpreter * pInterpreter, const char * pszResource)
{
   bool bResult = false;

   char * pszCode = NULL;
   UseGlobal(ResourceManager);
   if (pResourceManager->Load(tResKey(pszResource, kRC_Text), (void**)&pszCode) == S_OK)
   {
      bResult = SUCCEEDED(pInterpreter->ExecString(pszCode));
      pResourceManager->Unload(tResKey(pszResource, kRC_Text));
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

void * TiXmlDocumentFromText(void * pData, int dataLength, void * param)
{
   char * psz = reinterpret_cast<char*>(pData);
   if (psz != NULL && strlen(psz) > 0)
   {
      TiXmlDocument * pDoc = new TiXmlDocument;
      if (pDoc != NULL)
      {
         pDoc->Parse(psz);
         bool bError = pDoc->Error();
         if (bError)
         {
            delete pDoc;
            pDoc = NULL;
         }
         return pDoc;
      }
   }

   return NULL;
}

void TiXmlDocumentUnload(void * pData)
{
   TiXmlDocument * pDoc = reinterpret_cast<TiXmlDocument*>(pData);
   delete pDoc;
}

///////////////////////////////////////////////////////////////////////////////

CGcontext g_CgContext = NULL;
ulong g_CgContextRef = 0;
CGerrorCallbackFunc g_CgOldErrorCallback = NULL;

void CgErrorCallback()
{
   CGerror lastError = cgGetError();
   if (lastError)
   {
      DebugMsg(cgGetErrorString(lastError));
      const char * pszListing = cgGetLastListing(g_CgContext);
      if (pszListing != NULL)
      {
         DebugMsg1("   %s\n", pszListing);
      }
   }
}

CGcontext CgGetContext()
{
   if (g_CgContext == NULL)
   {
      g_CgContext = cgCreateContext();
      Verify(++g_CgContextRef == 1);
      g_CgOldErrorCallback = cgGetErrorCallback();
      cgSetErrorCallback(CgErrorCallback);
   }
   else
   {
      ++g_CgContextRef;
   }

   return g_CgContext;
}

void CgReleaseContext()
{
   if (g_CgContextRef > 0)
   {
      if (--g_CgContextRef == 0 && g_CgContext != NULL)
      {
         cgSetErrorCallback(g_CgOldErrorCallback);
         g_CgOldErrorCallback = NULL;
         cgDestroyContext(g_CgContext);
         g_CgContext = NULL;
      }
   }
}

///////////////////////////////////////////////////////////////////////////////

CGprofile g_CgProfile = CG_PROFILE_UNKNOWN;

void * CgProgramFromText(void * pData, int dataLength, void * param)
{
   // Must get the Cg context first
   CGcontext cgContext = CgGetContext();
   if (cgContext == NULL)
   {
      return NULL;
   }

   if (g_CgProfile == CG_PROFILE_UNKNOWN)
   {
      g_CgProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
      if (g_CgProfile == CG_PROFILE_UNKNOWN)
      {
         return NULL;
      }
   }

   char * psz = reinterpret_cast<char*>(pData);
   if (psz != NULL && strlen(psz) > 0)
   {
      CGprogram program = cgCreateProgram(cgContext, CG_SOURCE, psz, g_CgProfile, NULL, NULL);
      if (program != NULL)
      {
         cgGLLoadProgram(program);
         return program;
      }
   }

   return NULL;
}

void CgProgramUnload(void * pData)
{
   CGprogram program = reinterpret_cast<CGprogram>(pData);
   if (program != NULL)
   {
      cgDestroyProgram(program);
   }
}

///////////////////////////////////////////////////////////////////////////////

void * CgEffectFromText(void * pData, int dataLength, void * param)
{
   char * psz = reinterpret_cast<char*>(pData);
   if (psz != NULL && strlen(psz) > 0)
   {
   }

   return NULL;
}

void CgEffectUnload(void * pData)
{
//   TiXmlDocument * pDoc = reinterpret_cast<TiXmlDocument*>(pData);
//   delete pDoc;
}

///////////////////////////////////////////////////////////////////////////////

tResult EngineRegisterResourceFormats()
{
   UseGlobal(ResourceManager);
   if (!!pResourceManager)
   {
      if (TextFormatRegister("cg") == S_OK
         && TextFormatRegister("fx") == S_OK
         && pResourceManager->RegisterFormat(kRC_CgProgram, kRC_Text, NULL, NULL, CgProgramFromText, CgProgramUnload) == S_OK
         && pResourceManager->RegisterFormat(kRC_CgEffect, kRC_Text, NULL, NULL, CgEffectFromText, CgEffectUnload) == S_OK
         && pResourceManager->RegisterFormat(kRC_TiXml, kRC_Text, NULL, NULL, TiXmlDocumentFromText, TiXmlDocumentUnload) == S_OK)
      {
         return S_OK;
      }
   }
   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////
