/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORTYPES_H)
#define INCLUDED_EDITORTYPES_H

#include "editorapi.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef CPoint tPoint;

F_DECLARE_INTERFACE(IHeightMap);

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMapSettings
//

enum eHeightData
{
   // The order of these constants must match the order of the
   // corresponding radio buttons in the map settings dialog box
   kHeightData_None,
   kHeightData_Noise,
   kHeightData_HeightMap,
};

class cMapSettings
{
public:
   cMapSettings();
   cMapSettings(uint xDimension, uint zDimension, const tChar * pszTileSet,
      eHeightData heightData = kHeightData_None, const tChar * pszHeightMapFile = NULL);
   cMapSettings(const cMapSettings & mapSettings);
   ~cMapSettings();

   const cMapSettings & operator =(const cMapSettings & mapSettings);

   inline uint GetXDimension() const { return m_xDimension; }
   inline uint GetZDimension() const { return m_zDimension; }
   inline const tChar * GetTileSet() const { return m_tileSet.c_str(); }
   inline eHeightData GetHeightData() const { return m_heightData; }
   inline const tChar * GetHeightMap() const { return m_heightMapFile.c_str(); }

   tResult GetHeightMap(IHeightMap * * ppHeightMap) const;

private:
   uint m_xDimension, m_zDimension;
   cStr m_tileSet;
   eHeightData m_heightData;
   cStr m_heightMapFile;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorKeyEvent
//

class cEditorKeyEvent
{
public:
   cEditorKeyEvent(WPARAM wParam, LPARAM lParam);

   uint GetChar() const { return m_char; }
   uint GetRepeatCount() const { return m_repeats; }
   uint GetFlags() const { return m_flags; }

private:
   uint m_char, m_repeats, m_flags;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorMouseEvent
//

class cEditorMouseEvent
{
public:
   cEditorMouseEvent(WPARAM wParam, LPARAM lParam);

   uint GetFlags() const { return m_flags; }
   CPoint GetPoint() const { return m_point; }

private:
   uint m_flags;
   CPoint m_point;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorMouseWheelEvent
//

class cEditorMouseWheelEvent : public cEditorMouseEvent
{
public:
   cEditorMouseWheelEvent(WPARAM wParam, LPARAM lParam);

   short GetZDelta() const { return m_zDelta; }

private:
   short m_zDelta;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORTYPES_H)
