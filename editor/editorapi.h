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

struct sTileDefinition
{
   cStr name;
   cStr texture;
   int horzImages;
   int vertImages;
};

interface UUID("CA3DFC7D-CF34-43cd-AE46-FA1AF6A34F27") IEditorTileManager : IUnknown
{
   virtual tResult AddTile(const tChar * pszName,
                           const tChar * pszTexture,
                           int horzImages, int vertImages) = 0;
};

void EditorTileManagerCreate();

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_EDITORAPI_H
