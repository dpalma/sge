///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ms3dmodel/ms3dmodel.h"

#include "engine/modelapi.h"

#include "tech/configapi.h"
#include "tech/dictionaryapi.h"
#include "tech/filespec.h"
#include "tech/globalobj.h"
#include "tech/readwriteapi.h"
#include "tech/resourceapi.h"
#include "tech/threadcallapi.h"

#include "NvTriStrip.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

static void RegisterGlobalObjects()
{
   ResourceManagerCreate();
   ThreadCallerCreate();
}


///////////////////////////////////////////////////////////////////////////////

static tResult LoadCfgFile(const tChar * pszArgv0, IDictionary * pDict)
{
   if (pszArgv0 == NULL || pDict == NULL)
   {
      return E_POINTER;
   }

   cFileSpec cfgFile(pszArgv0);
   cfgFile.SetFileExt(_T("cfg"));

   cAutoIPtr<IDictionaryStore> pStore = DictionaryStoreCreate(cfgFile);
   if (!pStore)
   {
      return E_OUTOFMEMORY;
   }

   return pStore->Load(pDict);
}

static tResult InitGlobalConfig(int argc, tChar * argv[])
{
   Assert(argc > 0);

   LoadCfgFile(argv[0], g_pConfig);

   ParseCommandLine(argc, argv, g_pConfig);

   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////

static bool MainInit(int argc, tChar * argv[])
{
   if (InitGlobalConfig(argc, argv) != S_OK)
   {
      return false;
   }

   RegisterGlobalObjects();
   if (FAILED(StartGlobalObjects()))
   {
      ErrorMsg("One or more application-level services failed to start!\n");
      return false;
   }

   UseGlobal(ThreadCaller);
   if (FAILED(pThreadCaller->ThreadInit()))
   {
      return false;
   }

   return true;
}

///////////////////////////////////////////////////////////////////////////////

static void MainTerm()
{
   UseGlobal(ThreadCaller);
   pThreadCaller->ThreadTerm();

   StopGlobalObjects();
}

///////////////////////////////////////////////////////////////////////////////

int _tmain(int argc, tChar * argv[])
{
   int result = EXIT_FAILURE;

   if (!MainInit(argc, argv))
   {
      MainTerm();
      return result;
   }

   cStr model;
   if (ConfigGet(_T("in"), &model) == S_OK)
   {
      cAutoIPtr<IReader> pReader;
      if (FileReaderCreate(cFileSpec(model.c_str()), kFileModeBinary, &pReader) == S_OK)
      {
         cMs3dModel ms3dModel;
         if (ms3dModel.Read(pReader) == S_OK)
         {
            result = EXIT_SUCCESS;
         }
      }
   }

   MainTerm();

   return result;
}

///////////////////////////////////////////////////////////////////////////////
