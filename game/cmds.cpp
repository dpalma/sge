///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "sys.h"
#include "script.h"

#include "scriptvar.h"
#include "guiapi.h"

#include "renderapi.h"

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

extern IRenderDevice * AccessRenderDevice();

int ToggleFrameStats(int argc, const cScriptVar * argv, 
                     int nMaxResults, cScriptVar * pResults)
{
   float x = 25, y = 25;
   tGUIColor color(tGUIColor::White);

   if (argc == 2 
      && ScriptArgIsNumber(0) 
      && ScriptArgIsNumber(1))
   {
      x = ScriptArgAsNumber(0);
      y = ScriptArgAsNumber(1);
   }
   else if (argc == 3 
      && ScriptArgIsNumber(0) 
      && ScriptArgIsNumber(1) 
      && ScriptArgIsString(2))
   {
      x = ScriptArgAsNumber(0);
      y = ScriptArgAsNumber(1);
      GUIStyleParseColor(ScriptArgAsString(2), &color);
   }
   else if (argc == 5 
      && ScriptArgIsNumber(0) 
      && ScriptArgIsNumber(1) 
      && ScriptArgIsNumber(2) 
      && ScriptArgIsNumber(3) 
      && ScriptArgIsNumber(4))
   {
      x = ScriptArgAsNumber(0);
      y = ScriptArgAsNumber(1);
      color = tGUIColor(ScriptArgAsNumber(2), ScriptArgAsNumber(3), ScriptArgAsNumber(4));
   }

   if (AccessRenderDevice() != NULL)
   {
      if (AccessRenderDevice()->ShowStatistics(x, y, color) == S_FALSE)
      {
         AccessRenderDevice()->HideStatistics();
      }
   }

   return 0;
}

AUTOADD_SCRIPTFUNCTION(ToggleFrameStats, ToggleFrameStats);

///////////////////////////////////////////////////////////////////////////////
