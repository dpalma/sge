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

#ifdef _UNICODE
typedef std::wstring cStrBase;
#else
typedef std::string cStrBase;
#endif

class TECH_API cStr : public cStrBase
{
public:
   cStr() {}
   cStr(const tChar * psz) : cStrBase(psz) {}

   operator const tChar *() const { return cStrBase::c_str(); }

   int GetLength() const { return cStrBase::length(); }

   void Empty() { cStrBase::erase(); }

   tChar GetAt(int index) const { return cStrBase::operator[](index); }

   void Delete(int index) { cStrBase::erase(index); }
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_STR_H
