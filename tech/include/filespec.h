///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_FILESPEC_H
#define INCLUDED_FILESPEC_H

#include "techdll.h"
#include "fileconst.h"

#ifdef _MSC_VER
#pragma once
#endif

class cFilePath;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFileSpec
//

class TECH_API cFileSpec
{
public:
   cFileSpec();
   explicit cFileSpec(const char * pszFile);
   cFileSpec(const cFileSpec & other);

   const cFileSpec & operator =(const char * pszFile);

   int Compare(const cFileSpec & other) const;
   int CompareNoCase(const cFileSpec & other) const;

   const char * GetName() const;
   const char * GetFileName() const;
   const char * GetFileExt() const;
   bool SetFileExt(const char * pszExt);

   void SetPath(const cFilePath & path);
   cFilePath GetPath() const;

   bool Exists() const;

private:
   char m_szFullName[kMaxPath];
};

///////////////////////////////////////

inline const char * cFileSpec::GetName() const
{
   return m_szFullName;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_FILESPEC_H
