///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ZIPARCHIVE_H
#define INCLUDED_ZIPARCHIVE_H

#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

class cFileSpec;

F_DECLARE_INTERFACE(IReader);

#if defined(STRICTUNZIP) || defined(STRICTZIPUNZIP)
typedef struct TagunzFile__ { int unused; } unzFile__;
typedef unzFile__ *unzFile;
#else
typedef void * unzFile;
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cZipArchive
//

class cZipArchive
{
public:
   cZipArchive();
   ~cZipArchive();

   tResult Open(const cFileSpec & zipFileSpec);
   tResult Close();
   tResult IsOpen() const;

   tResult OpenMember(const char * pszFile, IReader * * ppReader);

private:
   unzFile m_unzFile;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ZIPARCHIVE_H
