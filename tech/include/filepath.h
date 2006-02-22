///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_FILEPATH_H
#define INCLUDED_FILEPATH_H

#include "techdll.h"

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
   enum { kMaxPath = 260 };

   cFilePath();
   cFilePath(const cFilePath & other);
   explicit cFilePath(const tChar * pszPath);
   cFilePath(const tChar * pszPath, size_t pathLen);

   const cFilePath & operator =(const cFilePath & other);

   bool operator ==(const cFilePath & other);
   bool operator !=(const cFilePath & other);

   const tChar * CStr() const;

   size_t GetLength() const;
   bool IsEmpty() const;

   bool AddRelative(const tChar * pszDir);

   bool IsFullPath() const;
   void MakeFullPath();

   cFilePath CollapseDots();

   static cFilePath GetCwd();

private:
   tChar m_szPath[kMaxPath];
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_FILEPATH_H
