///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_STRIPIFY_H
#define INCLUDED_STRIPIFY_H

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

typedef std::vector<int> tIntArray;

struct sStripInfo
{
   tIntArray strip;
   tIntArray faces;
};

///////////////////////////////////////////////////////////////////////////////

void ComputeStrips(const int * pFaces, int nFaces,
                   std::vector<sStripInfo> * pAllStrips);

void CombineStrips(const std::vector<sStripInfo> & allStrips,
                   tIntArray * pStrip);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_STRIPIFY_H
