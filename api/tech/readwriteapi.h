///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_READWRITEAPI_H
#define INCLUDED_READWRITEAPI_H

#include "techdll.h"
#include "comtools.h"

#include <string>

#ifdef _MSC_VER
#pragma once
#endif

class cFileSpec;
F_DECLARE_INTERFACE(IReader);
F_DECLARE_INTERFACE(IWriter);
F_DECLARE_INTERFACE(IMD5Writer);

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
   static tResult Read(IReader * pReader, T * pValue)
   {
      Assert(!"Cannot use default cReadWriteOps<>::Read()!");
      return E_NOTIMPL;
   }

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

   virtual tResult ReadLine(std::string * pLine) = 0;
   virtual tResult ReadLine(std::wstring * pLine) = 0;

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
   tResult Write(const T & value)
   {
      return cReadWriteOps<T>::Write(this, value);
   }

   virtual tResult Write(const void * pValue, size_t cbValue,
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

///////////////////////////////////////

template <>
inline tResult cReadWriteOps<char>::Read(IReader * pReader, char * pValue)
{
   return pReader->Read(pValue, sizeof(*pValue));
}

template <>
inline tResult cReadWriteOps<char>::Write(IWriter * pWriter, char value)
{
   return pWriter->Write(&value, sizeof(value));
}

///////////////////////////////////////

// For, Visual C++ or lower, the wchar_t type is covered by the ushort overload of Read/Write
#if _MSC_VER > 1310
template <>
inline tResult cReadWriteOps<wchar_t>::Read(IReader * pReader, wchar_t * pValue)
{
   return pReader->Read(pValue, sizeof(*pValue));
}

template <>
inline tResult cReadWriteOps<wchar_t>::Write(IWriter * pWriter, wchar_t value)
{
   return pWriter->Write(&value, sizeof(value));
}
#endif

///////////////////////////////////////

template <>
inline tResult cReadWriteOps<float>::Read(IReader * pReader, float * pValue)
{
   return pReader->Read(pValue, sizeof(*pValue));
}

template <>
inline tResult cReadWriteOps<float>::Write(IWriter * pWriter, float value)
{
   return pWriter->Write(&value, sizeof(value));
}

///////////////////////////////////////

template <>
inline tResult cReadWriteOps<double>::Read(IReader * pReader, double * pValue)
{
   return pReader->Read(pValue, sizeof(*pValue));
}

template <>
inline tResult cReadWriteOps<double>::Write(IWriter * pWriter, double value)
{
   return pWriter->Write(&value, sizeof(value));
}


///////////////////////////////////////////////////////////////////////////////

enum eFileMode
{
   kFileModeText, kFileModeBinary,
};

TECH_API tResult FileReaderCreate(const cFileSpec & file, eFileMode mode, IReader * * ppReader);
TECH_API tResult FileWriterCreate(const cFileSpec & file, eFileMode mode, IWriter * * ppWriter);

TECH_API tResult MemReaderCreate(const byte * pMem, size_t memSize, bool bOwn, IReader * * ppReader);
TECH_API tResult MemWriterCreate(byte * pMem, size_t memSize, IWriter * * ppWriter);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IMD5Writer
//

interface IMD5Writer : IWriter
{
   virtual void InitializeMD5() = 0;
   virtual tResult FinalizeMD5(byte digest[16]) = 0;
};

TECH_API tResult MD5WriterCreate(IWriter * pWriter, IMD5Writer * * ppWriter);


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_READWRITEAPI_H
