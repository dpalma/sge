/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorTypes.h"

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMapSettings
//

////////////////////////////////////////

cMapSettings::cMapSettings()
 : m_xDimension(0),
   m_zDimension(0),
   m_tileSet(""),
   m_heightData(kHeightData_None),
   m_heightMapFile("")
{
}

////////////////////////////////////////

cMapSettings::cMapSettings(uint xDimension,
                           uint zDimension,
                           const tChar * pszTileSet,
                           eHeightData heightData,
                           const tChar * pszHeightMapFile)
 : m_xDimension(xDimension),
   m_zDimension(zDimension),
   m_tileSet(pszTileSet != NULL ? pszTileSet : ""),
   m_heightData(heightData),
   m_heightMapFile(pszHeightMapFile != NULL ? pszHeightMapFile : "")
{
}

////////////////////////////////////////

cMapSettings::cMapSettings(const cMapSettings & mapSettings)
 : m_xDimension(mapSettings.m_xDimension),
   m_zDimension(mapSettings.m_zDimension),
   m_tileSet(mapSettings.m_tileSet),
   m_heightData(mapSettings.m_heightData),
   m_heightMapFile(mapSettings.m_heightMapFile)
{
}

////////////////////////////////////////

const cMapSettings & cMapSettings::operator=(const cMapSettings & mapSettings)
{
   m_xDimension = mapSettings.m_xDimension;
   m_zDimension = mapSettings.m_zDimension;
   m_tileSet = mapSettings.m_tileSet;
   m_heightData = mapSettings.m_heightData;
   m_heightMapFile = mapSettings.m_heightMapFile;
   return *this;
}

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorKeyEvent
//

cEditorKeyEvent::cEditorKeyEvent(WPARAM wParam, LPARAM lParam)
 : m_char(wParam), m_repeats(LOWORD(lParam)), m_flags(HIWORD(lParam))
{
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorMouseEvent
//

cEditorMouseEvent::cEditorMouseEvent(WPARAM wParam, LPARAM lParam)
 : m_flags(wParam), m_point(lParam)
{
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorMouseWheelEvent
//

cEditorMouseWheelEvent::cEditorMouseWheelEvent(WPARAM wParam, LPARAM lParam)
 : cEditorMouseEvent(LOWORD(wParam), lParam), m_zDelta(HIWORD(wParam))
{
}


/////////////////////////////////////////////////////////////////////////////
