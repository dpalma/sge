///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorapi.h"

#include "scriptapi.h"
#include "scriptvar.h"

#include "resourceapi.h"
#include "dictionaryapi.h"
#include "globalobj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////

int CreateTileSet(int argc, const cScriptVar * argv, 
                  int nMaxResults, cScriptVar * pResults)
{
   if (argc > 1)
   {
      if (argv[0].type != kString)
      {
         WarnMsg("Expected string value for tile set name\n");
         return 0;
      }
      std::vector<cStr> textures;
      UseGlobal(ResourceManager);
      for (int i = 1; i < argc; i++)
      {
         if (argv[i].type != kString)
         {
            WarnMsg("Expected string value for terrain texture\n");
            continue;
         }
         void * pTemp;
         if (pResourceManager->Load(tResKey(argv[i], kRC_Image), (void**)&pTemp) != S_OK)
         {
            WarnMsg1("Unable to load terrain texture \"%s\"\n", argv[i].psz);
            continue;
         }
         textures.push_back(argv[i].psz);
      }
      if (!textures.empty())
      {
         cAutoIPtr<IEditorTileSet> pTileSet;
         if (EditorTileSetCreate(argv[0], textures, &pTileSet) == S_OK)
         {
            UseGlobal(EditorTileManager);
            pEditorTileManager->AddTileSet(pTileSet);
         }
      }
   }
   return 0;
}

///////////////////////////////////////////////////////////////////////////////

int SetDefaultTileSet(int argc, const cScriptVar * argv, 
                                int nMaxResults, cScriptVar * pResults)
{
   if (argc == 1 && argv[0].type == kString)
   {
      UseGlobal(EditorTileManager);
      pEditorTileManager->SetDefaultTileSet(argv[0]);
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
   { "CreateTileSet", CreateTileSet },
   { "SetDefaultTileSet", SetDefaultTileSet },
#ifdef _DEBUG
   { "EmitDebugMessages", EmitDebugMessages },
#endif
};

uint g_nEditorCmds = _countof(g_editorCmds);

///////////////////////////////////////////////////////////////////////////////
