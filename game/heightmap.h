///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_HEIGHTMAP_H
#define INCLUDED_HEIGHTMAP_H

#ifdef _MSC_VER
#pragma once
#endif

class cImageData;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cHeightMap
//

class cHeightMap
{
public:
   cHeightMap(float scale = 1);
   ~cHeightMap();

   bool Load(const tChar * pszFilename);
   uint GetSize() const;
   uint8 HeightSample(uint x, uint z) const;
   float Height(uint x, uint z) const;

private:
   void Destroy();

   cImageData * m_pImage;
   float m_scale;
   uint m_size; // all height maps are square, for now
   uint8 * m_pData; // byte pointer to height data
};

///////////////////////////////////////

inline uint cHeightMap::GetSize() const
{
   return m_size;
}

///////////////////////////////////////

inline uint8 cHeightMap::HeightSample(uint x, uint z) const
{
   if (m_pData == NULL)
      return 0;
   return m_pData[(z * GetSize()) + x];
}

///////////////////////////////////////

inline float cHeightMap::Height(uint x, uint z) const
{
   return (float)HeightSample(x,z) * m_scale;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_HEIGHTMAP_H
