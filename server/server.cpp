///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "network/netapi.h"

#include "tech/configapi.h"
#include "tech/globalobj.h"
#include "tech/filespec.h"
#include "tech/imageapi.h"
#include "tech/resourceapi.h"
#include "tech/techstring.h"
#include "tech/threadcallapi.h"

#include <ctime>

#include "tech/dbgalloc.h" // must be last header

// REFERENCES
// http://www.mindcontrol.org/~hplus/authentication.html


///////////////////////////////////////////////////////////////////////////////

static void RegisterGlobalObjects()
{
   NetworkCreate();
   ResourceManagerCreate();
   ThreadCallerCreate();
}


///////////////////////////////////////////////////////////////////////////////

static tResult InitGlobalConfig(int argc, tChar * argv[])
{
   Assert(argc > 0);

   cFileSpec cfgFile(argv[0]);
   cfgFile.SetFileExt(_T("cfg"));

   cAutoIPtr<IDictionaryStore> pStore = DictionaryStoreCreate(cfgFile);
   if (!pStore)
   {
      return E_OUTOFMEMORY;
   }
   pStore->Load(g_pConfig);

   ParseCommandLine(argc, argv, g_pConfig);

   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////

tResult ServerInit(int argc, tChar * argv[])
{
   if (InitGlobalConfig(argc, argv) != S_OK)
   {
      return E_FAIL;
   }

   cFileSpec logFile(argv[0]);
   logFile.SetFileExt(_T("log"));
   techlog.OpenLogFile(logFile);

   srand(time(NULL));

   RegisterGlobalObjects();
   if (FAILED(StartGlobalObjects()))
   {
      DebugMsg("One or more application-level services failed to start!\n");
      return E_FAIL;
   }

   TextFormatRegister(_T("lua,xml"));
   ImageRegisterResourceFormats();

   cStr temp;
   if (ConfigGet(_T("data"), &temp) == S_OK)
   {
      UseGlobal(ResourceManager);
      pResourceManager->AddDirectoryTreeFlattened(temp.c_str());
   }

   UseGlobal(ThreadCaller);
   if (FAILED(pThreadCaller->ThreadInit()))
   {
      return E_FAIL;
   }

   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////

void ServerTerm()
{
   UseGlobal(ThreadCaller);
   pThreadCaller->ThreadTerm();

   StopGlobalObjects();
}


///////////////////////////////////////////////////////////////////////////////

tResult ServerFrame()
{
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
