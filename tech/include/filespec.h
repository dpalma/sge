///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_FILESPEC_H
#define INCLUDED_FILESPEC_H

#include "techdll.h"
#include "techstring.h"

#ifdef _MSC_VER
#pragma once
#endif

class cFilePath;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFileSpec
//

class TECH_API cFileSpec : public cStr
{
public:
   cFileSpec();
   explicit cFileSpec(const char * pszFile);
   cFileSpec(const cFileSpec & other);

   const cFileSpec & operator =(const cFileSpec & other);

   bool operator ==(const cFileSpec & other);
   bool operator !=(const cFileSpec & other);

   const char * GetFileName() const;
   bool GetFileNameNoExt(cStr * pFileName) const;
   const char * GetFileExt() const;
   bool SetFileExt(const char * pszExt);

   void SetPath(const cFilePath & path);
   cFilePath GetPath() const;
};

///////////////////////////////////////

inline bool cFileSpec::operator ==(const cFileSpec & other)
{
   return filepathcmp(*this, other) == 0;
}

///////////////////////////////////////

inline bool cFileSpec::operator !=(const cFileSpec & other)
{
   return filepathcmp(*this, other) != 0;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_FILESPEC_H
