///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "filereadwrite.h"
#include "filespec.h"
#include "str.h"

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

long cFileReader::Tell()
{
   if (m_fp != NULL)
   {
      return ftell(m_fp);
   }
   return -1;
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
   if (m_fp == NULL)
      return E_FAIL;

   long pos = Tell();
   uint len = 0;
   for (char c = fgetc(m_fp); c != stop && c != EOF; c = fgetc(m_fp))
      len++;
   Seek(pos, kSO_Set);

   char * pszBuffer = (char *)alloca(len + 1);

   uint nRead = fread(pszBuffer, 1, len + 1, m_fp);

   if (ferror(m_fp))
      return E_FAIL;

   pszBuffer[len] = 0; // ensure always null-terminated
   *pValue = pszBuffer;

   if (feof(m_fp))
      return S_FALSE;
   else
      return S_OK;
}

///////////////////////////////////////

tResult cFileReader::Read(void * pv, ulong nBytes, ulong * pnBytesRead)
{
   if (m_fp == NULL)
      return E_FAIL;

   uint nBytesRead = fread(pv, sizeof(byte), nBytes, m_fp);

   if (ferror(m_fp))
      return E_FAIL;

   if (pnBytesRead != NULL)
      *pnBytesRead = nBytesRead;

   if (feof(m_fp))
      return S_FALSE;
   else
      return S_OK;
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
      uint writeLen = strlen(value) + 1;
      if (fwrite(value, 1, writeLen, m_fp) == writeLen)
         return S_OK;
   }
   return E_FAIL;
}

///////////////////////////////////////

tResult cFileWriter::Write(void * pv, ulong nBytes, ulong * pnBytesWritten)
{
   if (m_fp != NULL)
   {
      uint nBytesWritten = fwrite(pv, sizeof(byte), nBytes, m_fp);
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
