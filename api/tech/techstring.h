///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_TECHSTRING_H
#define INCLUDED_TECHSTRING_H

#include "techdll.h"

#include <cstdarg>
#include <string>

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////

TECH_API size_t UTF8Encode(const wchar_t * pszSrc, char * pszDest, size_t maxDest);


///////////////////////////////////////////////////////////////////////////////

TECH_API int SprintfLengthEstimate(const tChar * pszFormat, va_list args);


///////////////////////////////////////////////////////////////////////////////

bool WildCardMatch(const tChar * pszPattern, const tChar * pszString);


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
