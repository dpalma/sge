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

F_DECLARE_INTERFACE(ITexture);
F_DECLARE_INTERFACE(IMaterial);

#define UUID(uuidstr) __declspec(uuid(uuidstr))

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMapSettings
//

enum eHeightData
{
   kHeightData_None,
   kHeightData_HeightMap,
   kHeightData_Noise,
};

class cMapSettings
{
public:
   cMapSettings();
   cMapSettings(uint xDimension, uint zDimension, const tChar * pszTileSet,
      eHeightData heightData, const tChar * pszHeightMapFile);
   cMapSettings(const cMapSettings & mapSettings);
   const cMapSettings & operator=(const cMapSettings & mapSettings);

   inline uint GetXDimension() const { return m_xDimension; }
   inline uint GetZDimension() const { return m_zDimension; }
   inline const tChar * GetTileSet() const { return m_tileSet.c_str(); }
   inline eHeightData GetHeightData() const { return m_heightData; }
   inline const tChar * GetHeightMap() const { return m_heightMapFile.c_str(); }

private:
   uint m_xDimension, m_zDimension;
   cStr m_tileSet;
   eHeightData m_heightData;
   cStr m_heightMapFile;
};

/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorApp
//

interface UUID("2A04E541-6BA1-41e9-92FA-E7B3D493F1A2") IEditorApp : IUnknown
{
   virtual tResult AddLoopClient(IEditorLoopClient * pLoopClient) = 0;
   virtual tResult RemoveLoopClient(IEditorLoopClient * pLoopClient) = 0;

   virtual tResult GetMapSettings(cMapSettings * pMapSettings) = 0;
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
   virtual tResult GetTileSetCount(uint * pTileSets) = 0;
   virtual tResult GetTileSet(uint index, IEditorTileSet * * ppTileSet) = 0;
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

   virtual tResult GetMaterial(IMaterial * * ppMaterial) = 0;

   virtual tResult GetImageList(uint dimension, HIMAGELIST * phImageList) = 0;
};

/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorTile
//

interface UUID("CDEB5694-56D2-4750-BEF8-85F286364C23") IEditorTile : IUnknown
{
   virtual tResult GetName(cStr * pName) const = 0;
   virtual tResult GetTexture(cStr * pTexture) const = 0;

   virtual tResult GetTexture(ITexture * * ppTexture) = 0;

   virtual tResult GetBitmap(uint dimension, HBITMAP * phBitmap) = 0;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_EDITORAPI_H
