/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_TERRAINAPI_H)
#define INCLUDED_TERRAINAPI_H

#include "comtools.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define TERRAIN_API // temporary

F_DECLARE_INTERFACE(IHeightMap);

/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IHeightMap
//

interface IHeightMap : IUnknown
{
   virtual float GetNormalizedHeight(float nx, float nz) const = 0;
};

////////////////////////////////////////

TERRAIN_API tResult HeightMapCreateSimple(float heightValue, IHeightMap * * ppHeightMap);
TERRAIN_API tResult HeightMapLoad(const tChar * pszHeightData, IHeightMap * * ppHeightMap);

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TERRAINAPI_H
