///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_STR_H
#define INCLUDED_STR_H

#include "techdll.h"

#include <string>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cStr
//

#ifdef _UNICODE
typedef std::wstring cStrBase;
#else
typedef std::string cStrBase;
#endif

class cStr : public cStrBase
{
public:
   cStr();
   cStr(const tChar * psz);

   operator const tChar *() const;

   size_t GetLength() const;

   void Empty();
};

///////////////////////////////////////

inline cStr::cStr()
{
}

///////////////////////////////////////

inline cStr::cStr(const tChar * psz)
 : cStrBase(psz)
{
}

///////////////////////////////////////

inline cStr::operator const tChar *() const
{
   return cStrBase::c_str();
}

///////////////////////////////////////

inline size_t cStr::GetLength() const
{
   return cStrBase::length();
}

///////////////////////////////////////

inline void cStr::Empty()
{
   cStrBase::erase();
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_STR_H
