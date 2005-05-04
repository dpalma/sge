///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_EDITORTILEMANAGER_H
#define INCLUDED_EDITORTILEMANAGER_H

#include "editorapi.h"
#include "connptimpl.h"
#include "globalobj.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorTileManager
//

class cEditorTileManager : public cGlobalObject<IMPLEMENTSCP(IEditorTileManager, IEditorTileManagerListener)>
{
public:
   cEditorTileManager();

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult CreateTileSet(const tChar * pszName, IEditorTileSet * * ppTileSet);
   virtual tResult GetTileSet(const tChar * pszName, IEditorTileSet * * ppTileSet);
   virtual tResult GetDefaultTileSet(IEditorTileSet * * ppTileSet);
   virtual tResult GetDefaultTileSet(cStr * pName) const;
   virtual tResult SetDefaultTileSet(const tChar * pszName);

   virtual tResult GetTileSetCount(uint * pTileSets);
   virtual tResult GetTileSet(uint index, IEditorTileSet * * ppTileSet);

private:
   typedef std::map<cStr, IEditorTileSet *> tTileSetMap;
   tTileSetMap m_tileSetMap;

   cStr m_defaultTileSet;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_EDITORTILEMANAGER_H
