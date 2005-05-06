///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_FILESPEC_H
#define INCLUDED_FILESPEC_H

#include "techdll.h"
#include "techstring.h"
#include "fileconst.h"

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

   int Compare(const cFileSpec & other) const;
   int CompareNoCase(const cFileSpec & other) const;

   const char * GetFileName() const;
   bool GetFileNameNoExt(cStr * pFileName) const;
   const char * GetFileExt() const;
   bool SetFileExt(const char * pszExt);

   void SetPath(const cFilePath & path);
   cFilePath GetPath() const;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_FILESPEC_H
