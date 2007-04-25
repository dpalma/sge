///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_TOKEN_H
#define INCLUDED_TOKEN_H

#include "techdll.h"

#include "techstring.h"

#include <string>
#include <vector>

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////

typedef void (* tParseTupleCallbackFn)(const tChar * pszToken, uint_ptr userData);

TECH_API int ParseTuple(const tChar * pszIn, const tChar * pszDelims,
                        tParseTupleCallbackFn pfnCallback, uint_ptr userData);


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cToken
//
// An accessory class to cTokenizer used to create typed tokens from
// a string representation

template <typename TOKEN, typename CHAR = tChar>
class cToken
{
public:
   static TOKEN Token(const CHAR * pszToken)
   {
      Assert(!"Cannot use default token convert function!");
      return TOKEN();
   }
};

///////////////////////////////////////

template <>
class TECH_API cToken<std::string, tChar>
{
public:
   static std::string Token(const tChar * pszToken);
};

///////////////////////////////////////

template <>
class TECH_API cToken<double, tChar>
{
public:
   static double Token(const tChar * pszToken);
};

///////////////////////////////////////

template <>
class TECH_API cToken<float, tChar>
{
public:
   static float Token(const tChar * pszToken);
};

///////////////////////////////////////

template <>
class TECH_API cToken<int, tChar>
{
public:
   static int Token(const tChar * pszToken);
};


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cTokenizer
//
// Really just a helper class that wraps the ParseTuple function

template <typename TOKEN, typename CONTAINER = std::vector<TOKEN>, typename CHAR = tChar>
class cTokenizer
{
public:
   int Tokenize(const CHAR * pszIn, const CHAR * pszDelims = NULL)
   {
      tParseTupleCallbackFn pfn = &cTokenizer<TOKEN, CONTAINER, CHAR>::FillContainer;
      return ::ParseTuple(pszIn, pszDelims, pfn, (uint_ptr)&m_tokens);
   }

   static void FillContainer(const CHAR * pszToken, uint_ptr userData)
   {
      CONTAINER * pContainer = (CONTAINER *)userData;
      if (pszToken && pContainer)
      {
         pContainer->push_back(cToken<TOKEN>::Token(pszToken));
      }
   }

   CONTAINER m_tokens;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TOKEN_H
