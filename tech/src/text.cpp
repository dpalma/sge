///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "readwriteapi.h"
#include "resourceapi.h"
#include "globalobj.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

template <typename T>
void * TextLoad(IReader * pReader)
{
   if (pReader != NULL)
   {
      ulong length;
      if (pReader->Seek(0, kSO_End) == S_OK
         && pReader->Tell(&length) == S_OK
         && pReader->Seek(0, kSO_Set) == S_OK)
      {
         T * pszContents = new T[length + 1];
         if (pszContents != NULL)
         {
            if (pReader->Read(pszContents, length * sizeof(T)) == S_OK)
            {
               pszContents[length] = 0;
               return pszContents;
            }
            delete [] pszContents;
            pszContents = NULL;
         }
      }
   }
   return NULL;
}

template <typename T>
void TextUnload(void * pData)
{
   delete [] reinterpret_cast<T*>(pData);
}

TECH_API tResult TextFormatRegister(const tChar * pszExtension)
{
   UseGlobal(ResourceManager);
   if (!!pResourceManager)
   {
      if (pResourceManager->RegisterFormat(kRT_AsciiText, pszExtension, TextLoad<char>, NULL, TextUnload<char>) == S_OK
         && pResourceManager->RegisterFormat(kRT_UnicodeText, pszExtension, TextLoad<wchar_t>, NULL, TextUnload<wchar_t>) == S_OK)
      {
         return S_OK;
      }
   }
   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////
