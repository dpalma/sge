///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_READWRITEUTILS_H
#define INCLUDED_READWRITEUTILS_H

#include "techdll.h"
#include "quat.h"
#include "readwriteapi.h"
#include "vec3.h"

#include <string>
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
// Read and write STL strings

template <typename T>
class cReadWriteOps< std::basic_string<T> >
{
public:
   static tResult Read(IReader * pReader, std::basic_string<T> * pS);
   static tResult Write(IWriter * pWriter, const std::basic_string<T> & s);
};

template <typename T>
tResult cReadWriteOps< std::basic_string<T> >::Read(IReader * pReader, std::basic_string<T> * pS)
{
   if (pReader == NULL || pS == NULL)
   {
      return E_POINTER;
   }

   pS->clear();

   std::basic_string<T>::size_type len = 0;

   tResult result = pReader->Read(&len);
   if ((result == S_OK) && (len > 0))
   {
      pS->resize(len);
      std::basic_string<T>::iterator iter = pS->begin(), end = pS->end();
      for (; iter != end; ++iter)
      {
         T & t = *iter;
         result = pReader->Read(&t);
         if (result != S_OK)
         {
            break;
         }
      }
   }

   return result;
}

template <typename T>
tResult cReadWriteOps< std::basic_string<T> >::Write(IWriter * pWriter, const std::basic_string<T> & s)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }
   tResult result = pWriter->Write(s.length());
   if (result == S_OK)
   {
      std::basic_string<T>::const_iterator iter = s.begin(), end = s.end();
      for (; iter != end; ++iter)
      {
         result = pWriter->Write(*iter);
         if (result != S_OK)
         {
            break;
         }
      }
   }
   return result;
}


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
   if (result == S_OK)
   {
      if (nValues > 0)
      {
         pValues->resize(nValues);
         std::vector<T>::iterator iter = pValues->begin(), end = pValues->end();
         for (; iter != end; ++iter)
         {
            T & t = *iter;
            result = pReader->Read(&t);
            if (result != S_OK)
            {
               break;
            }
         }
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
      std::vector<T>::const_iterator iter = values.begin(), end = values.end();
      for (; iter != end; ++iter)
      {
         result = pWriter->Write(*iter);
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
