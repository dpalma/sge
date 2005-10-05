///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_READWRITEAPI_H
#define INCLUDED_READWRITEAPI_H

#include "techdll.h"
#include "comtools.h"
#include "techstring.h"

#ifdef _MSC_VER
#pragma once
#endif

class cStr;
class cFileSpec;
F_DECLARE_INTERFACE(IReader);
F_DECLARE_INTERFACE(IWriter);

enum eSeekOrigin
{
   kSO_Set,
   kSO_End,
   kSO_Cur,
};

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cReadWriteOps
//

template <typename T>
class cReadWriteOps
{
public:
   ////////////////////////////////////

   static tResult Read(IReader * pReader, T * pValue)
   {
      Assert(!"Cannot use default cReadWriteOps<>::Read()!");
      return E_NOTIMPL;
   }

   ////////////////////////////////////

   static tResult Write(IWriter * pWriter, T value)
   {
      Assert(!"Cannot use default cReadWriteOps<>::Write()!");
      return E_NOTIMPL;
   }
};

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IReader
//

interface IReader : IUnknown
{
   virtual tResult Tell(ulong * pPos) = 0;
   virtual tResult Seek(long pos, eSeekOrigin origin) = 0;

   template <typename T>
   tResult Read(T * pValue)
   {
      return cReadWriteOps<T>::Read(this, pValue);
   }

   tResult Read(int * pValue);
   tResult Read(uint * pValue);
   tResult Read(long * pValue);
   tResult Read(ulong * pValue);
   tResult Read(short * pValue);
   tResult Read(ushort * pValue);
   tResult Read(byte * pValue);

   virtual tResult Read(cStr * pValue, tChar stop) = 0;
   virtual tResult Read(void * pValue, size_t cbValue,
                        size_t * pcbRead = NULL) = 0;
};

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IWriter
//

interface IWriter : IUnknown
{
   virtual tResult Tell(ulong * pPos) = 0;
   virtual tResult Seek(long pos, eSeekOrigin origin) = 0;

   template <typename T>
   tResult Write(T value)
   {
      return cReadWriteOps<T>::Write(this, value);
   }

   tResult Write(int value);
   tResult Write(uint value);
   tResult Write(long value);
   tResult Write(ulong value);
   tResult Write(short value);
   tResult Write(ushort value);
   tResult Write(byte value);

   virtual tResult Write(const char * value) = 0;
   virtual tResult Write(void * pValue, size_t cbValue,
                         size_t * pcbWritten = NULL) = 0;
};

///////////////////////////////////////////////////////////////////////////////
// Explicit template instantiations for basic types

///////////////////////////////////////

template <>
inline tResult cReadWriteOps<int>::Read(IReader * pReader, int * pValue)
{
   return pReader->Read(pValue, sizeof(*pValue));
}

template <>
inline tResult cReadWriteOps<int>::Write(IWriter * pWriter, int value)
{
   return pWriter->Write(&value, sizeof(value));
}

///////////////////////////////////////

template <>
inline tResult cReadWriteOps<uint>::Read(IReader * pReader, uint * pValue)
{
   return pReader->Read(pValue, sizeof(*pValue));
}

template <>
inline tResult cReadWriteOps<uint>::Write(IWriter * pWriter, uint value)
{
   return pWriter->Write(&value, sizeof(value));
}

///////////////////////////////////////

template <>
inline tResult cReadWriteOps<long>::Read(IReader * pReader, long * pValue)
{
   return pReader->Read(pValue, sizeof(*pValue));
}

template <>
inline tResult cReadWriteOps<long>::Write(IWriter * pWriter, long value)
{
   return pWriter->Write(&value, sizeof(value));
}

///////////////////////////////////////

template <>
inline tResult cReadWriteOps<ulong>::Read(IReader * pReader, ulong * pValue)
{
   return pReader->Read(pValue, sizeof(*pValue));
}

template <>
inline tResult cReadWriteOps<ulong>::Write(IWriter * pWriter, ulong value)
{
   return pWriter->Write(&value, sizeof(value));
}

///////////////////////////////////////

template <>
inline tResult cReadWriteOps<short>::Read(IReader * pReader, short * pValue)
{
   return pReader->Read(pValue, sizeof(*pValue));
}

template <>
inline tResult cReadWriteOps<short>::Write(IWriter * pWriter, short value)
{
   return pWriter->Write(&value, sizeof(value));
}

///////////////////////////////////////

template <>
inline tResult cReadWriteOps<ushort>::Read(IReader * pReader, ushort * pValue)
{
   return pReader->Read(pValue, sizeof(*pValue));
}

template <>
inline tResult cReadWriteOps<ushort>::Write(IWriter * pWriter, ushort value)
{
   return pWriter->Write(&value, sizeof(value));
}

///////////////////////////////////////

template <>
inline tResult cReadWriteOps<byte>::Read(IReader * pReader, byte * pValue)
{
   return pReader->Read(pValue, sizeof(*pValue));
}

template <>
inline tResult cReadWriteOps<byte>::Write(IWriter * pWriter, byte value)
{
   return pWriter->Write(&value, sizeof(value));
}

///////////////////////////////////////////////////////////////////////////////

template <>
class cReadWriteOps<cStr>
{
public:
   static tResult Read(IReader * pReader, cStr * pValue);
   static tResult Write(IWriter * pWriter, const cStr & value);
};

///////////////////////////////////////

inline tResult cReadWriteOps<cStr>::Read(IReader * pReader, cStr * pValue)
{
   cStr::size_type length = 0;
   if (pReader->Read(&length) == S_OK)
   {
      pValue->resize(length);
      return pReader->Read(&(*pValue)[0], length * sizeof(cStr::value_type));
   }
   return E_FAIL;
}

///////////////////////////////////////

inline tResult cReadWriteOps<cStr>::Write(IWriter * pWriter, const cStr & value)
{
   if (pWriter->Write(value.length()) == S_OK
      && pWriter->Write((void*)value.c_str(), value.length() * sizeof(cStr::value_type)) == S_OK)
   {
      return S_OK;
   }
   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////
// IReader inline functions

inline tResult IReader::Read(int * pValue)
{
   return cReadWriteOps<int>::Read(this, pValue);
}

inline tResult IReader::Read(uint * pValue)
{
   return cReadWriteOps<uint>::Read(this, pValue);
}

inline tResult IReader::Read(long * pValue)
{
   return cReadWriteOps<long>::Read(this, pValue);
}

inline tResult IReader::Read(ulong * pValue)
{
   return cReadWriteOps<ulong>::Read(this, pValue);
}

inline tResult IReader::Read(short * pValue)
{
   return cReadWriteOps<short>::Read(this, pValue);
}

inline tResult IReader::Read(ushort * pValue)
{
   return cReadWriteOps<ushort>::Read(this, pValue);
}

inline tResult IReader::Read(byte * pValue)
{
   return cReadWriteOps<byte>::Read(this, pValue);
}

///////////////////////////////////////////////////////////////////////////////
// IWriter inline functions

inline tResult IWriter::Write(int value)
{
   return cReadWriteOps<int>::Write(this, value);
}

inline tResult IWriter::Write(uint value)
{
   return cReadWriteOps<uint>::Write(this, value);
}

inline tResult IWriter::Write(long value)
{
   return cReadWriteOps<long>::Write(this, value);
}

inline tResult IWriter::Write(ulong value)
{
   return cReadWriteOps<ulong>::Write(this, value);
}

inline tResult IWriter::Write(short value)
{
   return cReadWriteOps<short>::Write(this, value);
}

inline tResult IWriter::Write(ushort value)
{
   return cReadWriteOps<ushort>::Write(this, value);
}

inline tResult IWriter::Write(byte value)
{
   return cReadWriteOps<byte>::Write(this, value);
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cReadWriteOps<float>
//

template <>
class cReadWriteOps<float>
{
public:
   static tResult Read(IReader * pReader, float * pValue)
   {
      return pReader->Read(pValue, sizeof(float));
   }

   static tResult Write(IWriter * pWriter, float value)
   {
      return pWriter->Write(&value, sizeof(float));
   }
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cReadWriteOps<double>
//

template <>
class cReadWriteOps<double>
{
public:
   static tResult Read(IReader * pReader, double * pValue)
   {
      return pReader->Read(pValue, sizeof(double));
   }

   static tResult Write(IWriter * pWriter, double value)
   {
      return pWriter->Write(&value, sizeof(double));
   }
};

///////////////////////////////////////////////////////////////////////////////

TECH_API IReader * FileCreateReader(const cFileSpec & file);
TECH_API IWriter * FileCreateWriter(const cFileSpec & file);

TECH_API tResult ReaderCreateMem(byte * pMem, size_t memSize, bool bOwn, IReader * * ppReader);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_READWRITEAPI_H
