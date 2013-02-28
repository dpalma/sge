///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_READWRITEFILE_H
#define INCLUDED_READWRITEFILE_H

#include "tech/readwriteapi.h"

#include <cstdio>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFileReader
//

class cFileReader : public cComObject<IMPLEMENTS(IReader)>
{
public:
   cFileReader(FILE * fp);
   virtual ~cFileReader();

   virtual void OnFinalRelease();

   virtual tResult Tell(ulong * pPos);
   virtual tResult Seek(long pos, eSeekOrigin origin);

   virtual tResult ReadLine(std::string * pLine);
   virtual tResult ReadLine(std::wstring * pLine);

   virtual tResult Read(void * pv, size_t cb, size_t * pcbRead = NULL);

private:
   FILE * m_fp;
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFileWriter
//

class cFileWriter : public cComObject<IMPLEMENTS(IWriter)>
{
public:
   cFileWriter(FILE * fp);
   virtual ~cFileWriter();

   virtual void OnFinalRelease();

   virtual tResult Tell(ulong * pPos);
   virtual tResult Seek(long pos, eSeekOrigin origin);

   virtual tResult Write(const void * pv, size_t cb, size_t * pcbWritten = NULL);

private:
   FILE * m_fp;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_READWRITEFILE_H
