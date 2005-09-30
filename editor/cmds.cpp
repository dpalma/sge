///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorapi.h"

#include "scriptapi.h"
#include "scriptvar.h"

#include "dictionaryapi.h"
#include "globalobj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


///////////////////////////////////////////////////////////////////////////////

int SetDefaultTileSet(int argc, const cScriptVar * argv, 
                                int nMaxResults, cScriptVar * pResults)
{
   if (argc == 1 && argv[0].type == kString)
   {
      UseGlobal(EditorApp);
      pEditorApp->SetDefaultTileSet(argv[0]);
   }
   return 0;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
// Used for testing the log window
int EmitDebugMessages(int argc, const cScriptVar * argv, 
                      int nMaxResults, cScriptVar * pResults)
{
   static ulong base = 0;
   if (argc > 0 && argv[0].type == kNumber)
   {
      int n = (int)argv[0];
      for (int i = 0; i < n; i++)
      {
         DebugMsg1("Sample debug message %d\n", base + i);
      }
      base += n;
   }
   return 0;
}
#endif

///////////////////////////////////////////////////////////////////////////////

sScriptReg g_editorCmds[] =
{
   { "SetDefaultTileSet", SetDefaultTileSet },
#ifdef _DEBUG
   { "EmitDebugMessages", EmitDebugMessages },
#endif
};

uint g_nEditorCmds = _countof(g_editorCmds);

///////////////////////////////////////////////////////////////////////////////
