///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_READWRITEMD5_H
#define INCLUDED_READWRITEMD5_H

#include "tech/readwriteapi.h"
#include "tech/techhash.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMD5Writer
//

class cMD5Writer : public cComObject<IMPLEMENTS(IMD5Writer)>
{
public:
   cMD5Writer(IWriter * pWriter);
   ~cMD5Writer();

   virtual void InitializeMD5();
   virtual tResult FinalizeMD5(byte digest[16]);

   virtual tResult Tell(ulong * pPos);
   virtual tResult Seek(long pos, eSeekOrigin origin);
   virtual tResult Write(const void * pValue, size_t cbValue, size_t * pcbWritten);

private:
   cAutoIPtr<IWriter> m_pWriter;
   bool m_bUpdateMD5;
   cMD5 m_md5;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_READWRITEMD5_H
