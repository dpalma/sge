///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GROUNDTILED_H
#define INCLUDED_GROUNDTILED_H

#ifdef _MSC_VER
#pragma once
#endif

class cHeightMap;
class cSceneNode;

template <typename T> class cVec2;
typedef class cVec2<float> tVec2;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainData
//

class cTerrainData
{
public:
   cTerrainData(ulong sizeX, ulong sizeZ, float heightScale);
   ~cTerrainData();

   bool LoadHeightMap(const tChar * pszHeightMapFile);

   float GetElevation(float nx, float nz) const;

   cVec2<float> GetDimensions() const;

private:
   ulong m_sizeX, m_sizeZ;
   float m_heightScale;
   cHeightMap * m_pHeightMap;
};

extern cTerrainData g_terrainData;

///////////////////////////////////////////////////////////////////////////////

cSceneNode * TerrainRootNodeCreate();

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GROUNDTILED_H
