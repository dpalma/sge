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
AssertAtCompileTime(kSO_Set == 0);
AssertAtCompileTime(kSO_End == 1);
AssertAtCompileTime(kSO_Cur == 2);

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

AssertAtCompileTime(kFileModeText == 0);
AssertAtCompileTime(kFileModeBinary == 1);

tResult FileReaderCreate(const cFileSpec & file, eFileMode mode, IReader * * ppReader)
{
   if (ppReader == NULL)
   {
      return E_POINTER;
   }

   static const tChar * fopenModeStrings[] =
   {
      _T("rt"),
      _T("rb"),
   };

#if _MSC_VER >= 1300
   FILE * fp = NULL;
   if (_tfopen_s(&fp, file.CStr(), fopenModeStrings[mode]) != 0)
   {
      return E_FAIL;
   }
#else
   FILE * fp = _tfopen(file.CStr(), fopenModeStrings[mode]);
   if (fp == NULL)
   {
      return E_FAIL;
   }
#endif

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

tResult cFileWriter::Write(const tChar * value)
{
   if (m_fp != NULL)
   {
      size_t nBytes = _tcslen(value) * sizeof(tChar);
      if (fwrite(value, nBytes, 1, m_fp) == 1)
      {
         return S_OK;
      }
   }
   return E_FAIL;
}

///////////////////////////////////////

tResult cFileWriter::Write(const void * pv, size_t nBytes, size_t * pnBytesWritten)
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

AssertAtCompileTime(kFileModeText == 0);
AssertAtCompileTime(kFileModeBinary == 1);

tResult FileWriterCreate(const cFileSpec & file, eFileMode mode, IWriter * * ppWriter)
{
   if (ppWriter == NULL)
   {
      return E_POINTER;
   }

   static const tChar * fopenModeStrings[] =
   {
      _T("wt"),
      _T("wb"),
   };

#if _MSC_VER >= 1300
   FILE * fp = NULL;
   if (_tfopen_s(&fp, file.CStr(), fopenModeStrings[mode]) != 0)
   {
      return E_FAIL;
   }
#else
   FILE * fp = _tfopen(file.CStr(), fopenModeStrings[mode]);
   if (fp == NULL)
   {
      return E_FAIL;
   }
#endif

   cAutoIPtr<IWriter> pWriter(static_cast<IWriter*>(new cFileWriter(fp)));
   if (!pWriter)
   {
      fclose(fp);
      return E_OUTOFMEMORY;
   }
   return pWriter.GetPointer(ppWriter);
}

///////////////////////////////////////////////////////////////////////////////
