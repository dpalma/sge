///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GROUNDTILED_H
#define INCLUDED_GROUNDTILED_H

#include "scenenode.h"

#ifdef _MSC_VER
#pragma once
#endif

class cHeightMap;

template <typename T> class cVec2;
typedef class cVec2<float> tVec2;

class cTiledGround;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainNode
//

class cTerrainNode : public cSceneNode
{
   cTerrainNode(const cTerrainNode &);
   const cTerrainNode & operator =(const cTerrainNode &);

public:
   cTerrainNode(cHeightMap * pHeightMap);
   virtual ~cTerrainNode();

   float GetElevation(float nx, float nz) const;

   tVec2 GetDimensions() const;

   virtual void Render();

private:
   cHeightMap * m_pHeightMap;
   cTiledGround * m_pGround;
};

///////////////////////////////////////

cTerrainNode * TerrainNodeCreate(const char * pszHeightData, float heightScale);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GROUNDTILED_H
