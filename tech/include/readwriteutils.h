///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_READWRITEUTILS_H
#define INCLUDED_READWRITEUTILS_H

#include "techdll.h"
#include "quat.h"
#include "readwriteapi.h"
#include "vec3.h"

#ifdef _MSC_VER
#pragma once
#endif

////////////////////////////////////////////////////////////////////////////////

template <>
class TECH_API cReadWriteOps<tQuat>
{
public:
   static tResult Read(IReader * pReader, tQuat * pQ);
   static tResult Write(IWriter * pWriter, const tQuat & q);
};

////////////////////////////////////////////////////////////////////////////////

template <>
class TECH_API cReadWriteOps<tVec3>
{
public:
   static tResult Read(IReader * pReader, tVec3 * pV);
   static tResult Write(IWriter * pWriter, const tVec3 & v);
};

////////////////////////////////////////////////////////////////////////////////

template <>
class TECH_API cReadWriteOps<GUID>
{
public:
   static tResult Read(IReader * pReader, GUID * pGUID);
   static tResult Write(IWriter * pWriter, const GUID & guid);
};

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAutoBuffer
//

class TECH_API cAutoBuffer
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
