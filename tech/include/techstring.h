///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_TECHSTRING_H
#define INCLUDED_TECHSTRING_H

#include "techdll.h"

#include <cstdarg>
#include <string>
#include <vector>

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////

TECH_API size_t UTF8Encode(const wchar_t * pszSrc, char * pszDest, size_t maxDest);


///////////////////////////////////////////////////////////////////////////////

TECH_API int SprintfLengthEstimate(const tChar * pszFormat, va_list args);


///////////////////////////////////////////////////////////////////////////////

template <typename STRING>
const STRING & CDECL Sprintf(STRING * pString,
                             const typename STRING::value_type * pszFormat,
                             ...)
{
   Assert(pString != NULL);
   va_list args;
   va_start(args, pszFormat);
   int length = SprintfLengthEstimate(pszFormat, args) + 1; // plus one for null terminator
   size_t nBytes = length * sizeof(typename STRING::value_type);
   typename STRING::value_type * pszTemp = reinterpret_cast<typename STRING::value_type *>(alloca(nBytes));
#if _MSC_VER >= 1400
   int result = _vsntprintf_s(pszTemp, nBytes, length, pszFormat, args);
#else
   int result = _vsntprintf(pszTemp, length, pszFormat, args);
#endif
   va_end(args);
   pString->assign(pszTemp);
   return *pString;
}


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
std::string cToken<std::string, tChar>::Token(const tChar * pszToken)
{
#ifdef _UNICODE
   size_t s = wcstombs(NULL, pszToken, 0);
   if (s == 0)
   {
      return std::string();
   }
   char * pszTemp = reinterpret_cast<char*>(alloca(s * sizeof(char)));
   wcstombs(pszTemp, pszToken, s);
   return std::string(pszTemp);
#else
   return std::string(pszToken);
#endif
}

///////////////////////////////////////

template <>
double cToken<double, tChar>::Token(const tChar * pszToken)
{
#ifdef _UNICODE
   return _wtof(pszToken);
#else
   return atof(pszToken);
#endif
}

///////////////////////////////////////

template <>
float cToken<float, tChar>::Token(const tChar * pszToken)
{
#ifdef _UNICODE
   return static_cast<float>(_wtof(pszToken));
#else
   return static_cast<float>(atof(pszToken));
#endif
}

///////////////////////////////////////

template <>
int cToken<int, tChar>::Token(const tChar * pszToken)
{
#ifdef __GNUC__
   return strtol(pszToken, NULL, 10);
#else
   return _ttoi(pszToken);
#endif
}


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
//
// CLASS: cStr
//

#ifdef _UNICODE
typedef std::wstring cStr;
#else
typedef std::string cStr;
#endif


///////////////////////////////////////////////////////////////////////////////

template <typename STRING>
inline const STRING & TrimLeadingSpace(STRING * pString)
{
   typename STRING::size_type index = pString->find_first_not_of(_T(" \r\n\t"));
   if (index != STRING::npos)
   {
      pString->erase(0, index);
   }
   return *pString;
}

template <typename STRING>
inline const STRING & TrimTrailingSpace(STRING * pString)
{
   typename STRING::size_type index = pString->find_last_not_of(_T(" \r\n\t"));
   if (index != STRING::npos)
   {
      pString->erase(index + 1);
   }
   return *pString;
}


///////////////////////////////////////////////////////////////////////////////

typedef struct _GUID GUID;
typedef const GUID & REFGUID;
TECH_API const cStr & GUIDToString(REFGUID guid, cStr * pStr);


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TECHSTRING_H
