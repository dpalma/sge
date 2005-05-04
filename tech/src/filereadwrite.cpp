///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "filereadwrite.h"
#include "filespec.h"
#include "techstring.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFileReader
//

cFileReader::cFileReader(const cFileSpec & file)
 : m_fp(fopen(file.GetName(), "rb"))
{
}

///////////////////////////////////////

cFileReader::~cFileReader()
{
   Assert(m_fp == NULL);
}

///////////////////////////////////////

void cFileReader::OnFinalRelease()
{
   if (m_fp != NULL)
   {
      fclose(m_fp);
      m_fp = NULL;
   }
}

///////////////////////////////////////

tResult cFileReader::Tell(ulong * pPos)
{
   if (pPos == NULL)
   {
      return E_POINTER;
   }
   if (m_fp == NULL)
   {
      *pPos = 0;
      return S_FALSE;
   }
   else
   {
      *pPos = ftell(m_fp);
      return S_OK;
   }
}

///////////////////////////////////////

// ensure enumerated type values are good as array indices
AssertOnce(kSO_Set == 0);
AssertOnce(kSO_End == 1);
AssertOnce(kSO_Cur == 2);

static const int g_stdioSeekOrigin[] =
{
   SEEK_SET, SEEK_END, SEEK_CUR
};

inline int StdioSeekOrigin(eSeekOrigin origin)
{
   return g_stdioSeekOrigin[origin];
}

///////////////////////////////////////

tResult cFileReader::Seek(long pos, eSeekOrigin origin)
{
   if ((m_fp != NULL) &&
       (fseek(m_fp, pos, StdioSeekOrigin(origin)) == 0))
   {
      return S_OK;
   }
   return E_FAIL;
}

///////////////////////////////////////

tResult cFileReader::Read(cStr * pValue, char stop)
{
   if (pValue == NULL)
   {
      return E_POINTER;
   }

   if (m_fp == NULL)
   {
      return E_FAIL;
   }

   ulong pos;
   if (FAILED(Tell(&pos)))
   {
      return E_FAIL;
   }

   uint len = 0;
   for (char c = fgetc(m_fp); c != stop && c != EOF; c = fgetc(m_fp))
   {
      len++;
   }
   Seek(pos, kSO_Set);

   char * pszBuffer = (char *)alloca(len + 1);

   size_t nRead = fread(pszBuffer, 1, len + 1, m_fp);

   if (ferror(m_fp))
   {
      return E_FAIL;
   }

   pszBuffer[len] = 0; // ensure always null-terminated
   *pValue = pszBuffer;

   return feof(m_fp) ? S_FALSE : S_OK;
}

///////////////////////////////////////

tResult cFileReader::Read(void * pv, size_t nBytes, size_t * pnBytesRead)
{
   if (pv == NULL)
   {
      return E_POINTER;
   }

   if (m_fp == NULL)
   {
      return E_FAIL;
   }

   size_t nBytesRead = fread(pv, sizeof(byte), nBytes, m_fp);

   if (ferror(m_fp))
   {
     return E_FAIL;
   }

   if (pnBytesRead != NULL)
   {
      *pnBytesRead = nBytesRead;
   }

   return feof(m_fp) ? S_FALSE : S_OK;
}

///////////////////////////////////////

TECH_API IReader * FileCreateReader(const cFileSpec& file)
{
   return new cFileReader(file);
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFileWriter
//

cFileWriter::cFileWriter(const cFileSpec & file)
 : m_fp(fopen(file.GetName(), "wb"))
{
}

///////////////////////////////////////

cFileWriter::~cFileWriter()
{
   Assert(m_fp == NULL);
}

///////////////////////////////////////

void cFileWriter::OnFinalRelease()
{
   if (m_fp != NULL)
   {
      fclose(m_fp);
      m_fp = NULL;
   }
}

///////////////////////////////////////

tResult cFileWriter::Write(const char * value)
{
   if (m_fp != NULL)
   {
      size_t writeLen = strlen(value) + 1;
      if (fwrite(value, 1, writeLen, m_fp) == writeLen)
         return S_OK;
   }
   return E_FAIL;
}

///////////////////////////////////////

tResult cFileWriter::Write(void * pv, size_t nBytes, size_t * pnBytesWritten)
{
   if (m_fp != NULL)
   {
      size_t nBytesWritten = fwrite(pv, sizeof(byte), nBytes, m_fp);
      if (!ferror(m_fp) && nBytesWritten == nBytes)
      {
         if (pnBytesWritten != NULL)
            *pnBytesWritten = nBytesWritten;
         return S_OK;
      }
   }
   return E_FAIL;
}

///////////////////////////////////////

TECH_API IWriter * FileCreateWriter(const cFileSpec& file)
{
   return new cFileWriter(file);
}

///////////////////////////////////////////////////////////////////////////////
