///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorapi.h"

#include "scriptapi.h"
#include "scriptvar.h"

#include "dictionaryapi.h"
#include "globalobj.h"
#include "str.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////

int RegisterTileTexture(int argc, const cScriptVar * argv, 
                        int nMaxResults, cScriptVar * pResults)
{
   if (argc == 1 && argv[0].type == kInterface)
   {
      cAutoIPtr<IDictionary> pDict;
      if (argv[0].pUnk->QueryInterface(IID_IDictionary, (void**)&pDict) == S_OK)
      {
         cStr name, texture;
         int horzImages = -1, vertImages = -1;

         if (pDict->Get("texture", &texture) == S_OK)
         {
            if (pDict->Get("name", &name) != S_OK)
            {
               name = texture;
            }

            pDict->Get("horizontalImages", &horzImages);
            pDict->Get("verticalImages", &vertImages);

            UseGlobal(EditorTileManager);
            pEditorTileManager->AddTile(name.c_str(), texture.c_str(), horzImages, vertImages);
         }
         else
         {
            DebugMsg("No tile texture provided\n");
         }

         // TODO
      }
   }
   return 0;
}

AUTOADD_SCRIPTFUNCTION(RegisterTileTexture, RegisterTileTexture);

///////////////////////////////////////////////////////////////////////////////
