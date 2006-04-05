///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_READWRITEUTILS_H
#define INCLUDED_READWRITEUTILS_H

#include "readwriteapi.h"

#include "vec3.h"

#ifdef _MSC_VER
#pragma once
#endif

////////////////////////////////////////////////////////////////////////////////

template <>
class cReadWriteOps<tVec3>
{
public:
   static tResult Read(IReader * pReader, tVec3 * pV);
   static tResult Write(IWriter * pWriter, const tVec3 & v);
};

////////////////////////////////////////////////////////////////////////////////

template <>
class cReadWriteOps<GUID>
{
public:
   static tResult Read(IReader * pReader, GUID * pGUID);
   static tResult Write(IWriter * pWriter, const GUID & guid);
};

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAutoBuffer
//

class cAutoBuffer
{
   cAutoBuffer(const cAutoBuffer &);
   void operator =(const cAutoBuffer &);

public:
   cAutoBuffer();
   ~cAutoBuffer();

   tResult Malloc(size_t bufferSize, void * * ppBuffer);
   tResult Free();

private:
   byte * m_pBuffer;
   size_t m_bufferSize;
};

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_READWRITEUTILS_H
