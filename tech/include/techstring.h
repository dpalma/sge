///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_TECHSTRING_H
#define INCLUDED_TECHSTRING_H

#include "techdll.h"

#include <string>
#include <vector>

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

class TECH_API cStr : public cStrBase
{
public:
   cStr();
   cStr(const tChar * psz);
   cStr(const tChar * psz, uint length);

   const cStr & operator =(const cStr & other);
   bool operator ==(const cStr & other) const;
   bool operator !=(const cStr & other) const;
   const cStr & operator =(const tChar * psz);
   bool operator ==(const tChar * psz) const;
   bool operator !=(const tChar * psz) const;

   const tChar * Get() const;
   uint GetLength() const;
   void Empty();
   bool IsEmpty() const;

   void Append(const tChar * psz);
   void TrimLeadingSpace();
   void TrimTrailingSpace();

   int ToInt() const;
   float ToFloat() const;

   int ParseTuple(std::vector<cStr> * pStrings, const tChar * pszDelims = NULL) const;
   int ParseTuple(double * pDoubles, int nMaxDoubles) const;
   int ParseTuple(float * pFloats, int nMaxFloats) const;

   int CDECL Format(const tChar * pszFormat, ...);

   static const cStr gm_whitespace;
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

inline cStr::cStr(const tChar * psz, uint length)
 : cStrBase(psz, length)
{
}

///////////////////////////////////////

inline const cStr & cStr::operator =(const cStr & other)
{
   assign(other);
   return *this;
}

///////////////////////////////////////

inline bool cStr::operator ==(const cStr & other) const
{
   return compare(other) == 0;
}

///////////////////////////////////////

inline bool cStr::operator !=(const cStr & other) const
{
   return compare(other) != 0;
}

///////////////////////////////////////

inline const cStr & cStr::operator =(const tChar * psz)
{
   assign(psz);
   return *this;
}

///////////////////////////////////////

inline bool cStr::operator ==(const tChar * psz) const
{
   return compare(psz) == 0;
}

///////////////////////////////////////

inline bool cStr::operator !=(const tChar * psz) const
{
   return compare(psz) != 0;
}

///////////////////////////////////////

inline const tChar * cStr::Get() const
{
   return c_str();
}

///////////////////////////////////////

inline uint cStr::GetLength() const
{
   return length();
}

///////////////////////////////////////

inline void cStr::Empty()
{
   erase();
}

///////////////////////////////////////

inline bool cStr::IsEmpty() const
{
   return empty();
}

///////////////////////////////////////

inline void cStr::Append(const tChar * psz)
{
   *this += psz;
}

///////////////////////////////////////

inline void cStr::TrimLeadingSpace()
{
   cStrBase::size_type index = find_first_not_of(gm_whitespace);
   if (index != cStrBase::npos)
   {
      erase(0, index);
   }
}

///////////////////////////////////////

inline void cStr::TrimTrailingSpace()
{
   cStrBase::size_type index = find_last_not_of(gm_whitespace);
   if (index != cStrBase::npos)
   {
      erase(index + 1);
   }
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cStrLessNoCase
//

class cStrLessNoCase
{
public:
   bool operator()(const cStr & lhs, const cStr & rhs) const;
};

///////////////////////////////////////

inline bool cStrLessNoCase::operator()(const cStr & lhs, const cStr & rhs) const
{
   return (_tcsicmp(lhs.c_str(), rhs.c_str()) < 0) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////

int filepathcmp(const cStr & f1, const cStr & f2);
int filepathicmp(const cStr & f1, const cStr & f2);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TECHSTRING_H
