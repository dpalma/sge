/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "msPlugInApp.h"

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////

WTL::CAppModule _Module;

/////////////////////////////////////////////////////////////////////////////

HINSTANCE g_hInstance = NULL;

/////////////////////////////////////////////////////////////////////////////

extern "C" BOOL STDCALL DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
   if (dwReason == DLL_PROCESS_ATTACH)
   {
      if (FAILED(_Module.Init(NULL, hInstance)))
      {
         ErrorMsg("Error initializing ATL module object\n");
         return FALSE;
      }

      DisableThreadLibraryCalls(hInstance);
      g_hInstance = hInstance;
   }
   else if (dwReason == DLL_PROCESS_DETACH)
   {
      _Module.Term();
   }
   return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
