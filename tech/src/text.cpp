///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "readwriteapi.h"
#include "resourceapi.h"
#include "globalobj.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

static char * GetContents(IReader * pReader)
{
   Assert(pReader != NULL);

   pReader->Seek(0, kSO_End);
   ulong length;
   if (FAILED(pReader->Tell(&length)))
   {
      return NULL;
   }
   pReader->Seek(0, kSO_Set);

   char * pszContents = new char[length + 1];

   if (pReader->Read(pszContents, length) != S_OK)
   {
      delete [] pszContents;
      return NULL;
   }

   pszContents[length] = 0;

   return pszContents;
}

///////////////////////////////////////////////////////////////////////////////

void * TextLoad(IReader * pReader)
{
   if (pReader != NULL)
   {
      return GetContents(pReader);
   }

   return NULL;
}

void TextUnload(void * pData)
{
   delete [] reinterpret_cast<char *>(pData);
}

TECH_API tResult TextFormatRegister(const tChar * pszExtension)
{
   UseGlobal(ResourceManager);
   if (!!pResourceManager)
   {
      return pResourceManager->RegisterFormat(kRC_Text, pszExtension, TextLoad, NULL, TextUnload);
   }
   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////
