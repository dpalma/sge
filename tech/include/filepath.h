///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_FILEPATH_H
#define INCLUDED_FILEPATH_H

#include "techdll.h"
#include "techstring.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFilePath
//

class TECH_API cFilePath : public cStr
{
public:
   cFilePath();
   cFilePath(const cFilePath & other);
   explicit cFilePath(const tChar * pszPath);
   cFilePath(const tChar * pszPath, size_t pathLen);

   const cFilePath & operator =(const cFilePath & other);

   bool operator ==(const cFilePath & other);
   bool operator !=(const cFilePath & other);

   void AddRelative(const tChar * pszDir);

   bool IsFullPath() const;
   void MakeFullPath();

   cFilePath CollapseDots();

   static cFilePath GetCwd();
};

///////////////////////////////////////

inline bool cFilePath::operator ==(const cFilePath & other)
{
   return filepathcmp(*this, other) == 0;
}

///////////////////////////////////////

inline bool cFilePath::operator !=(const cFilePath & other)
{
   return filepathcmp(*this, other) != 0;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_FILEPATH_H
