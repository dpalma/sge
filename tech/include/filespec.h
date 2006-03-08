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

class TECH_API cFileSpec
{
public:
   enum { kMaxPath = 260 };

   cFileSpec();
   explicit cFileSpec(const tChar * pszFile);
   cFileSpec(const cFileSpec & other);

   const cFileSpec & operator =(const cFileSpec & other);

   bool operator ==(const cFileSpec & other);
   bool operator !=(const cFileSpec & other);

   const tChar * CStr() const;

   size_t GetLength() const;
   bool IsEmpty() const;

   const tChar * GetFileName() const;
   bool GetFileNameNoExt(cStr * pFileName) const;
   const tChar * GetFileExt() const;
   bool SetFileExt(const tChar * pszExt);

   void SetPath(const cFilePath & path);
   bool GetPath(cFilePath * pPath) const;

private:
   tChar m_szFile[kMaxPath];
};

////////////////////////////////////////

TECH_API int FileSpecCompare(const cFileSpec & f1, const cFileSpec & f2);

////////////////////////////////////////

TECH_API int FileSpecCompareNoCase(const cFileSpec & f1, const cFileSpec & f2);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_FILESPEC_H
