////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "readwritemem.h"

#include "tech/readwriteutils.h"
#include "tech/techstring.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

static const char szLineDelimiters[] = "\r\n";
static const wchar_t wszLineDelimiters[] = L"\r\n";


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMemReader
//

////////////////////////////////////////

cMemReader::cMemReader(const byte * pMem, size_t memSize, bool bOwn)
 : m_pMem(pMem)
 , m_memSize(memSize)
 , m_bOwn(bOwn)
 , m_readPos(0)
{
}

////////////////////////////////////////

cMemReader::~cMemReader()
{
   Assert(m_pMem == NULL);
}

////////////////////////////////////////

void cMemReader::OnFinalRelease()
{
   if (m_pMem != NULL)
   {
      if (m_bOwn)
      {
         delete [] m_pMem;
      }
      m_pMem = NULL;
      m_memSize = 0;
   }
}

////////////////////////////////////////

tResult cMemReader::Tell(ulong * pPos)
{
   if (pPos == NULL)
   {
      return E_POINTER;
   }
   if (m_pMem == NULL)
   {
      *pPos = 0;
      return S_FALSE;
   }
   else
   {
      *pPos = m_readPos;
      return S_OK;
   }
}

////////////////////////////////////////

tResult cMemReader::Seek(long pos, eSeekOrigin origin)
{
   switch (origin)
   {
   case kSO_Set: m_readPos = pos; break;
   case kSO_End: m_readPos = m_memSize + pos; break;
   case kSO_Cur: m_readPos += pos; break;
   }
   return S_OK;
}

////////////////////////////////////////

tResult cMemReader::ReadLine(std::string * pLine)
{
   if (pLine == NULL)
   {
      return E_POINTER;
   }

   if (m_pMem == NULL)
   {
      return E_FAIL;
   }

   if (m_readPos >= m_memSize)
   {
      return S_FALSE;
   }

   for (; m_readPos < m_memSize; ++m_readPos)
   {
      char c = static_cast<char>(m_pMem[m_readPos]);

      if (strchr(szLineDelimiters, c) != NULL)
      {
         break;
      }

      pLine->push_back(c);
   }

   if (m_readPos <= m_memSize)
   {
      return S_OK;
   }
   else
   {
      return S_FALSE;
   }
}

////////////////////////////////////////

tResult cMemReader::ReadLine(std::wstring * pLine)
{
   if (pLine == NULL)
   {
      return E_POINTER;
   }

   if (m_pMem == NULL)
   {
      return E_FAIL;
   }

   if (m_readPos >= m_memSize)
   {
      return S_FALSE;
   }

   for (; m_readPos <= (m_memSize - sizeof(wchar_t)); m_readPos += sizeof(wchar_t))
   {
      wchar_t c = 0;
      memcpy(&c, &m_pMem[m_readPos], sizeof(c));

      if (wcschr(wszLineDelimiters, c) != NULL)
      {
         break;
      }

      pLine->push_back(c);
   }

   if (m_readPos <= m_memSize)
   {
      return S_OK;
   }
   else
   {
      return S_FALSE;
   }
}

////////////////////////////////////////

tResult cMemReader::Read(void * pv, size_t nBytes, size_t * pnBytesRead)
{
   if (pv == NULL)
   {
      return E_POINTER;
   }

   if (m_pMem == NULL)
   {
      return E_FAIL;
   }

   size_t nBytesRead = Min(m_memSize - m_readPos, nBytes);

   memcpy(pv, m_pMem + m_readPos, nBytesRead);

   m_readPos += nBytesRead;

   if (pnBytesRead != NULL)
   {
      *pnBytesRead = nBytesRead;
   }

   if (m_readPos <= m_memSize)
   {
      return S_OK;
   }
   else
   {
      return S_FALSE;
   }
}

////////////////////////////////////////

tResult MemReaderCreate(const byte * pMem, size_t memSize, bool bOwn, IReader * * ppReader)
{
   if (pMem == NULL || ppReader == NULL)
   {
      return E_POINTER;
   }

   *ppReader = static_cast<IReader *>(new cMemReader(pMem, memSize, bOwn));

   if (*ppReader == NULL)
   {
      return E_OUTOFMEMORY;
   }

   return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMemWriter
//

////////////////////////////////////////

cMemWriter::cMemWriter(byte * pMem, size_t memSize)
 : m_pMem(pMem)
 , m_memSize(memSize)
 , m_writePos(0)
{
}

////////////////////////////////////////

cMemWriter::~cMemWriter()
{
   m_pMem = NULL;
   m_memSize = 0;
}

////////////////////////////////////////

tResult cMemWriter::Tell(ulong * pPos)
{
   if (pPos == NULL)
   {
      return E_POINTER;
   }
   if (m_pMem == NULL)
   {
      *pPos = 0;
      return S_FALSE;
   }
   else
   {
      *pPos = m_writePos;
      return S_OK;
   }
}

////////////////////////////////////////

tResult cMemWriter::Seek(long pos, eSeekOrigin origin)
{
   switch (origin)
   {
   case kSO_Set: m_writePos = pos; break;
   case kSO_End: m_writePos = m_memSize + pos; break;
   case kSO_Cur: m_writePos += pos; break;
   }
   return S_OK;
}

////////////////////////////////////////

tResult cMemWriter::Write(const void * pValue, size_t cbValue, size_t * pcbWritten /*=NULL*/)
{
   if (pValue == NULL)
   {
      return E_POINTER;
   }

   if (m_pMem == NULL)
   {
      return E_FAIL;
   }

   size_t nBytes = Min(cbValue, m_memSize - m_writePos);
   memcpy(&m_pMem[m_writePos], pValue, nBytes);
   m_writePos += nBytes;

   if (pcbWritten != NULL)
   {
      *pcbWritten = nBytes;
   }

   return (nBytes == cbValue) ? S_OK : S_FALSE;
}

////////////////////////////////////////

tResult MemWriterCreate(byte * pMem, size_t memSize, IWriter * * ppWriter)
{
   if (pMem == NULL || ppWriter == NULL)
   {
      return E_POINTER;
   }

   *ppWriter = static_cast<IWriter *>(new cMemWriter(pMem, memSize));

   if (*ppWriter == NULL)
   {
      return E_OUTOFMEMORY;
   }

   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

SUITE(ReadWriteMem)
{
   TEST(MemWriterBasics)
   {
      byte mem[100];

      const int writeInt = 1000;
      const std::string writeString("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
      const float writeFloat = 3.14159f;

      {
         cAutoIPtr<IWriter> pWriter;
         CHECK_EQUAL(S_OK, MemWriterCreate(&mem[0], sizeof(mem), &pWriter));
         pWriter->Write(writeInt);
         pWriter->Write(writeFloat);
      }

      {
         cAutoIPtr<IReader> pReader;
         CHECK_EQUAL(S_OK, MemReaderCreate(&mem[0], sizeof(mem), false, &pReader));

         int readInt = -1;
         CHECK_EQUAL(S_OK, pReader->Read(&readInt));
         CHECK_EQUAL(writeInt, readInt);

         std::string readString;
         CHECK_EQUAL(S_OK, pReader->Read(&readString));
         CHECK_EQUAL(writeString, readString);

         float readFloat = 0;
         CHECK_EQUAL(S_OK, pReader->Read(&readFloat));
         CHECK(writeFloat == readFloat);
      }
   }

   TEST(MemWriterExceedMemCapacity)
   {
      byte mem[10];

      cAutoIPtr<IWriter> pWriter;
      CHECK_EQUAL(S_OK, MemWriterCreate(&mem[0], sizeof(mem), &pWriter));

      static const char testString[] = "01234567890123456789";

      size_t nWritten = 0;
      CHECK_EQUAL(S_FALSE, pWriter->Write(testString, sizeof(testString), &nWritten));
      CHECK_EQUAL(10, nWritten);
      CHECK(memcmp(testString, mem, nWritten) == 0);
   }
}

#endif // HAVE_UNITTESTPP


////////////////////////////////////////////////////////////////////////////////
