///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_COMENUMUTIL_H
#define INCLUDED_COMENUMUTIL_H

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

template <typename IENUM, typename INTRFC, typename F>
F ForEach(IENUM * pEnum, F f)
{
   INTRFC * pIntrfcs[32];
   ulong count = 0;
   while (SUCCEEDED((pEnum->Next(_countof(pIntrfcs), &pIntrfcs[0], &count))) && (count > 0))
   {
      for (ulong i = 0; i < count; ++i)
      {
         f(pIntrfcs[i]);
         SafeRelease(pIntrfcs[i]);
      }
      count = 0;
   }
   return f;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_COMENUMUTIL_H
