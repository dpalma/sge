///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_CPUFEATURES_H
#define INCLUDED_CPUFEATURES_H

#include "techdll.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

const int kMaxVendorName      = 13;
const int kMaxModelName       = 30;
const int kMaxBrandString     = 48;

const int kCpuHasRdtsc              = (1<<4);
const int kCpuHasMmx                = (1<<23);
const int kCpuHasSse                = (1<<25);
const int kCpuHasSse2               = (1<<26);
const int kCpuHasHyperThreading     = (1<<28);

struct sCpuFeatures
{
   char szVendor[kMaxVendorName];
   char szModel[kMaxModelName];
   int features;
   char szBrand[kMaxBrandString];
};

TECH_API bool GetCpuFeatures(sCpuFeatures * pCpuFeatures);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_CPUFEATURES_H
