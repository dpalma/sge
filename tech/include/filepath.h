///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_FILEPATH_H
#define INCLUDED_FILEPATH_H

#include "techdll.h"
#include "fileconst.h"
#include <vector>
#include <string>

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
   cFilePath(const char * pszPath, int pathLen);

   const cFilePath & operator =(const char * pszPath);

   int Compare(const cFilePath & other) const;

   const char * GetPath() const;

   void AddRelative(const char * pszDir);

   bool IsFullPath() const;
   void MakeFullPath();

   static cFilePath GetCwd();

   int ListDirs(std::vector<std::string> * pDirs) const;

private:
   char m_szPath[kMaxPath];
};

///////////////////////////////////////

inline const char * cFilePath::GetPath() const
{
   return m_szPath;
}

///////////////////////////////////////

inline bool operator ==(const cFilePath & path1, const cFilePath & path2)
{
   return (path1.Compare(path2) == 0);
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_FILEPATH_H
