///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_READWRITEUTILS_H
#define INCLUDED_READWRITEUTILS_H

#include "techdll.h"
#include "quat.h"
#include "readwriteapi.h"
#include "vec3.h"

#include <vector>

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
// Read and write STL vectors

template <typename T>
class cReadWriteOps< std::vector<T> >
{
public:
   static tResult Read(IReader * pReader, std::vector<T> * pValues);
   static tResult Write(IWriter * pWriter, const std::vector<T> & values);
};

template <typename T>
tResult cReadWriteOps< std::vector<T> >::Read(IReader * pReader, std::vector<T> * pValues)
{
   if (pReader == NULL || pValues == NULL)
   {
      return E_POINTER;
   }
   uint nValues = 0;
   tResult result = pReader->Read(&nValues);
   if ((nValues > 0) && (result == S_OK))
   {
      pValues->resize(nValues);
      for (uint i = 0; (i < nValues) && (result == S_OK); ++i)
      {
         result = pReader->Read(&((*pValues)[i]));
      }
   }
   return result;
}

template <typename T>
tResult cReadWriteOps< std::vector<T> >::Write(IWriter * pWriter, const std::vector<T> & values)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }
   tResult result = pWriter->Write(static_cast<uint>(values.size()));
   if (result == S_OK)
   {
      for (uint i = 0; i < values.size(); ++i)
      {
         result = pWriter->Write(values[i]);
         if (result != S_OK)
         {
            break;
         }
      }
   }
   return result;
}


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAutoBuffer
//

class TECH_API cAutoBuffer
{
   cAutoBuffer(const cAutoBuffer &);
   const cAutoBuffer & operator =(const cAutoBuffer &);

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
