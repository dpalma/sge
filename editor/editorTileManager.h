///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_EDITORTILEMANAGER_H
#define INCLUDED_EDITORTILEMANAGER_H

#include "editorapi.h"
#include "globalobj.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorTileManager
//

class cEditorTileManager : public cGlobalObject<IMPLEMENTS(IEditorTileManager)>
{
public:
   cEditorTileManager();

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult AddTile(const tChar * pszName,
                           const tChar * pszTexture,
                           int horzImages, int vertImages);
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_EDITORTILEMANAGER_H
