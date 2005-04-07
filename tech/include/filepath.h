///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_FILEPATH_H
#define INCLUDED_FILEPATH_H

#include "techdll.h"
#include "fileconst.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFilePath
//

class TECH_API cFilePath
{
public:
   cFilePath();
   cFilePath(const cFilePath & other);
   explicit cFilePath(const char * pszPath);
   cFilePath(const char * pszPath, size_t pathLen);

   const cFilePath & operator =(const char * pszPath);

   bool operator ==(const cFilePath & other);

   int Compare(const cFilePath & other) const;
   int CompareNoCase(const cFilePath & other) const;

   const char * GetPath() const;

   void AddRelative(const char * pszDir);

   bool IsFullPath() const;
   void MakeFullPath();

   static cFilePath GetCwd();

private:
   char m_szPath[kMaxPath];
};

///////////////////////////////////////

inline bool cFilePath::operator ==(const cFilePath & other)
{
   return Compare(other) == 0;
}

///////////////////////////////////////

inline const char * cFilePath::GetPath() const
{
   return m_szPath;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_FILEPATH_H
