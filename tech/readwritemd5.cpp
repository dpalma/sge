///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "readwritemd5.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMD5Writer
//

////////////////////////////////////////

cMD5Writer::cMD5Writer(IWriter * pWriter)
 : m_pWriter(CTAddRef(pWriter))
 , m_bUpdateMD5(false)
{
   Assert(pWriter != NULL);
}

////////////////////////////////////////

cMD5Writer::~cMD5Writer()
{
}

////////////////////////////////////////

void cMD5Writer::InitializeMD5()
{
   m_md5.Initialize();
   m_bUpdateMD5 = true;
}

////////////////////////////////////////

tResult cMD5Writer::FinalizeMD5(byte digest[16])
{
   if (m_bUpdateMD5)
   {
      m_bUpdateMD5 = false;
      m_md5.Finalize(digest);
      return S_OK;
   }
   return S_FALSE;
}

////////////////////////////////////////

tResult cMD5Writer::Tell(ulong * pPos)
{
   return m_pWriter->Tell(pPos);
}

////////////////////////////////////////

tResult cMD5Writer::Seek(long pos, eSeekOrigin origin)
{
   WarnMsg("Attempting to seek with a MD5 IWriter\n");
   return E_FAIL;
}

////////////////////////////////////////

tResult cMD5Writer::Write(const void * pValue, size_t cbValue, size_t * pcbWritten)
{
   if (m_bUpdateMD5)
   {
      m_md5.Update(const_cast<byte *>(static_cast<const byte *>(pValue)), cbValue);
   }
   return m_pWriter->Write(pValue, cbValue, pcbWritten);
}

////////////////////////////////////////

tResult MD5WriterCreate(IWriter * pWriter, IMD5Writer * * ppWriter)
{
   if (pWriter == NULL || ppWriter == NULL)
   {
      return E_POINTER;
   }
   cAutoIPtr<IMD5Writer> pMD5Writer(static_cast<IMD5Writer*>(new cMD5Writer(pWriter)));
   if (!pMD5Writer)
   {
      return E_OUTOFMEMORY;
   }
   return pMD5Writer.GetPointer(ppWriter);
}

///////////////////////////////////////////////////////////////////////////////
