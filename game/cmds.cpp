///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "sys.h"
#include "script.h"

#include "scriptvar.h"
#include "guiapi.h"

#include "globalobj.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

SCRIPT_DEFINE_FUNCTION(quit)
{
   SysQuit();
   return 0;
}

///////////////////////////////////////////////////////////////////////////////

SCRIPT_DEFINE_FUNCTION(ConfirmedQuit)
{
   if (ScriptArgc() == 1 && ScriptArgIsString(0))
   {
      UseGlobal(GUIContext);
      if (pGUIContext->LoadFromString(ScriptArgAsString(0)) == 0)
      {
         if (pGUIContext->LoadFromResource(ScriptArgAsString(0)) == 0)
         {
            DebugMsg1("Error showing quit dialog %s\n", ScriptArgAsString(0));
         }
      }
   }

   return 0;
}

///////////////////////////////////////////////////////////////////////////////
