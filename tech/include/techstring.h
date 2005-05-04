///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_TECHSTRING_H
#define INCLUDED_TECHSTRING_H

#include "techdll.h"

#include <string>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cStr
//

class cStr : private std::string
{
   typedef std::string tBase;

public:
   cStr();
   cStr(const tChar * psz);
   cStr(const tChar * psz, uint length);

   operator const tChar *() const;

   bool operator ==(const cStr & other) const;
   bool operator !=(const cStr & other) const;
   bool operator ==(const tChar * psz) const;
   bool operator !=(const tChar * psz) const;

   const tChar * Get() const;
   int GetLength() const;
   void Empty();
   bool IsEmpty() const;

   void Append(const tChar * psz);
   void TrimLeadingSpace();
   void TrimTrailingSpace();

   int ToInt() const;
   float ToFloat() const;

   static const cStr gm_whitespace;
};

__declspec(selectany) const cStr cStr::gm_whitespace(" \t\r\n");

///////////////////////////////////////

inline cStr::cStr()
{
}

///////////////////////////////////////

inline cStr::cStr(const tChar * psz)
 : tBase(psz)
{
}

///////////////////////////////////////

inline cStr::cStr(const tChar * psz, uint length)
 : tBase(psz, length)
{
}

///////////////////////////////////////

inline cStr::operator const tChar *() const
{
   return Get();
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

inline int cStr::GetLength() const
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
   tBase::size_type index = find_first_not_of(gm_whitespace);
   if (index != tBase::npos)
   {
      erase(0, index);
   }
}

///////////////////////////////////////

inline void cStr::TrimTrailingSpace()
{
   tBase::size_type index = find_last_not_of(gm_whitespace);
   if (index != tBase::npos)
   {
      erase(index + 1);
   }
}

///////////////////////////////////////

inline int cStr::ToInt() const
{
   return atoi(Get());
}

///////////////////////////////////////

inline float cStr::ToFloat() const
{
   return static_cast<float>(atof(Get()));
}

///////////////////////////////////////////////////////////////////////////////

class cStrLessNoCase
{
public:
   bool operator()(const cStr & lhs, const cStr & rhs) const;
};

///////////////////////////////////////

inline bool cStrLessNoCase::operator()(const cStr & lhs, const cStr & rhs) const
{
   return (stricmp(lhs, rhs) < 0) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TECHSTRING_H
