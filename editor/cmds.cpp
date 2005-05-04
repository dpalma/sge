///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorapi.h"

#include "scriptapi.h"
#include "scriptvar.h"

#include "resourceapi.h"
#include "dictionaryapi.h"
#include "readwriteapi.h"
#include "globalobj.h"

#include <tinyxml.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define STRING_TILESET           "tileset"
#define STRING_TILE              "tile"
#define STRING_TEXTURE           "texture"
#define STRING_NAME              "name"
#define STRING_HORIZONTALIMAGES  "horizontalImages"
#define STRING_VERTICALIMAGES    "verticalImages"

///////////////////////////////////////////////////////////////////////////////

static uint LoadTileSets(const TiXmlDocument & xmlDoc)
{
   Assert(!xmlDoc.Error());

   uint nTileSets = 0;

   UseGlobal(EditorTileManager);

   TiXmlElement * pXmlElement;
   for (pXmlElement = xmlDoc.FirstChildElement();
        pXmlElement != NULL;
        pXmlElement = pXmlElement->NextSiblingElement())
   {
      if (pXmlElement->Type() == TiXmlNode::ELEMENT
         && strcmp(pXmlElement->Value(), STRING_TILESET) == 0)
      {
         const char * pszName = NULL;
         if ((pszName = pXmlElement->Attribute(STRING_NAME)) != NULL)
         {
            cAutoIPtr<IEditorTileSet> pTileSet;
            if (SUCCEEDED(pEditorTileManager->CreateTileSet(pszName, &pTileSet)))
            {
               nTileSets++;

               TiXmlElement * pXmlChild;
               for (pXmlChild = pXmlElement->FirstChildElement();
                    pXmlChild != NULL;
                    pXmlChild = pXmlChild->NextSiblingElement())
               {
                  if (pXmlChild->Type() == TiXmlNode::ELEMENT
                     && strcmp(pXmlChild->Value(), STRING_TILE) == 0)
                  {
                     const char * pszValue;

                     if ((pszValue = pXmlChild->Attribute(STRING_TEXTURE)) != NULL)
                     {
                        cStr texture, name;
                        int horzImages = 1, vertImages = 1;

                        texture = pszValue;

                        if ((pszValue = pXmlChild->Attribute(STRING_NAME)) != NULL)
                        {
                           name = pszValue;
                        }
                        else
                        {
                           name = texture;
                        }

                        if (pXmlChild->QueryIntAttribute(STRING_HORIZONTALIMAGES, &horzImages) != TIXML_SUCCESS)
                        {
                           horzImages = 1;
                        }

                        if (pXmlChild->QueryIntAttribute(STRING_VERTICALIMAGES, &vertImages) != TIXML_SUCCESS)
                        {
                           vertImages = 1;
                        }

                        Verify(pTileSet->AddTile(name.c_str(), texture.c_str(), horzImages, vertImages) == S_OK);
                     }
                  }
               }

               pEditorTileManager->SetDefaultTileSet(pszName);
            }
         }
      }
   }

   return nTileSets;
}

int LoadTiles(int argc, const cScriptVar * argv, 
              int nMaxResults, cScriptVar * pResults)
{
   if (argc == 1 && argv[0].type == kString)
   {
      tResKey rk(argv[0].psz, kRC_TiXml);
      TiXmlDocument * pTiXmlDoc = NULL;
      UseGlobal(ResourceManager);
      if (pResourceManager->Load(rk, (void**)&pTiXmlDoc) == S_OK)
      {
         LoadTileSets(*pTiXmlDoc);
         pResourceManager->Unload(rk);
      }
   }

   return 0;
}

///////////////////////////////////////////////////////////////////////////////

sScriptReg g_editorCmds[] =
{
   { "LoadTiles", LoadTiles },
};

uint g_nEditorCmds = _countof(g_editorCmds);

///////////////////////////////////////////////////////////////////////////////
