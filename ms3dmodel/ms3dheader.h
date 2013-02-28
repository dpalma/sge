///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MS3DHEADER_H
#define INCLUDED_MS3DHEADER_H

#include "tech/readwriteapi.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dHeader
//

class cMs3dHeader
{
   friend class cReadWriteOps<cMs3dHeader>;

public:
   static const cMs3dHeader gm_ms3dHeader;

   cMs3dHeader();
   cMs3dHeader(const char id[10], int version);
   cMs3dHeader(const cMs3dHeader & other);
   ~cMs3dHeader();

   const cMs3dHeader & operator =(const cMs3dHeader & other);

   void Assign(const cMs3dHeader & other);
   bool EqualTo(const cMs3dHeader & other) const;

private:
   char m_id[10]; // always "MS3D000000"
   int m_version; // 4
};

///////////////////////////////////////

template <>
class cReadWriteOps<cMs3dHeader>
{
public:
   static tResult Read(IReader * pReader, cMs3dHeader * pHeader);
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MS3DHEADER_H
