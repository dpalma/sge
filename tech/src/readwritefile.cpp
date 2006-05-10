///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "readwritefile.h"
#include "filespec.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFileReader
//

///////////////////////////////////////
// The cFileReader object will own the FILE* pointer

cFileReader::cFileReader(FILE * fp)
 : m_fp(fp)
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

tResult cFileReader::Read(cStr * pValue, tChar stop)
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
   for (char c = _fgettc(m_fp); c != stop && c != EOF; c = _fgettc(m_fp))
   {
      len++;
   }
   Seek(pos, kSO_Set);

   tChar * pszBuffer = reinterpret_cast<tChar*>(alloca((len + 1) * sizeof(tChar)));

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

tResult FileReaderCreate(const cFileSpec & file, IReader * * ppReader)
{
   if (ppReader == NULL)
   {
      return E_POINTER;
   }
   
   FILE * fp = _tfopen(file.CStr(), _T("rb"));
   if (fp == NULL)
   {
      return E_FAIL;
   }

   cAutoIPtr<IReader> pReader(static_cast<IReader*>(new cFileReader(fp)));
   if (!pReader)
   {
      fclose(fp);
      return E_OUTOFMEMORY;
   }
   return pReader.GetPointer(ppReader);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFileWriter
//

///////////////////////////////////////
// The cFileWriter object will own the FILE* pointer

cFileWriter::cFileWriter(FILE * fp)
 : m_fp(fp)
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

tResult cFileWriter::Tell(ulong * pPos)
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

tResult cFileWriter::Seek(long pos, eSeekOrigin origin)
{
   if ((m_fp != NULL) &&
       (fseek(m_fp, pos, StdioSeekOrigin(origin)) == 0))
   {
      return S_OK;
   }
   return E_FAIL;
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

tResult FileWriterCreate(const cFileSpec & file, IWriter * * ppWriter)
{
   if (ppWriter == NULL)
   {
      return E_POINTER;
   }

   FILE * fp = _tfopen(file.CStr(), _T("wb"));
   if (fp == NULL)
   {
      return E_FAIL;
   }

   cAutoIPtr<IWriter> pWriter(static_cast<IWriter*>(new cFileWriter(fp)));
   if (!pWriter)
   {
      fclose(fp);
      return E_OUTOFMEMORY;
   }
   return pWriter.GetPointer(ppWriter);
}

///////////////////////////////////////////////////////////////////////////////
