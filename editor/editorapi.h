///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_EDITORAPI_H
#define INCLUDED_EDITORAPI_H

#include "comtools.h"
#include "str.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

F_DECLARE_INTERFACE(IEditorApp);
F_DECLARE_INTERFACE(IEditorLoopClient);
F_DECLARE_INTERFACE(IEditorTileManager);
F_DECLARE_INTERFACE(IEditorTileSet);
F_DECLARE_INTERFACE(IEditorTile);

#define UUID(uuidstr) __declspec(uuid(uuidstr))

/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorApp
//

interface UUID("2A04E541-6BA1-41e9-92FA-E7B3D493F1A2") IEditorApp : IUnknown
{
   virtual tResult AddLoopClient(IEditorLoopClient * pLoopClient) = 0;
   virtual tResult RemoveLoopClient(IEditorLoopClient * pLoopClient) = 0;
};

IEditorApp * AccessEditorApp();

/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorLoopClient
//

interface UUID("ED1B3A1A-E2D8-4eec-AABD-648A548729E8") IEditorLoopClient : IUnknown
{
   virtual void OnFrame(double time, double elapsed) = 0;
};

/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorTileManager
//

interface UUID("CA3DFC7D-CF34-43cd-AE46-FA1AF6A34F27") IEditorTileManager : IUnknown
{
   virtual tResult CreateTileSet(const tChar * pszName, IEditorTileSet * * ppTileSet) = 0;
   virtual tResult GetTileSet(const tChar * pszName, IEditorTileSet * * ppTileSet) = 0;
   virtual tResult GetDefaultTileSet(IEditorTileSet * * ppTileSet) = 0;
   virtual tResult SetDefaultTileSet(const tChar * pszName) = 0;
};

void EditorTileManagerCreate();

/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorTileSet
//

interface UUID("61B488AA-AB50-41c5-AA42-45F07C982F6A") IEditorTileSet : IUnknown
{
   virtual tResult GetName(cStr * pName) const = 0;

   virtual tResult AddTile(const tChar * pszName,
                           const tChar * pszTexture,
                           int horzImages,
                           int vertImages) = 0;

   virtual tResult GetTileCount(uint * pTileCount) const = 0;
   virtual tResult GetTile(uint index, IEditorTile * * ppTile) = 0;
};

/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorTile
//

interface UUID("CDEB5694-56D2-4750-BEF8-85F286364C23") IEditorTile : IUnknown
{
   virtual tResult GetName(cStr * pName) const = 0;
   virtual tResult GetTexture(cStr * pTexture) const = 0;

   virtual tResult GetButtonImage(HBITMAP * phBitmap) = 0;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_EDITORAPI_H
