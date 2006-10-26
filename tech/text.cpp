///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "tech/readwriteapi.h"
#include "tech/resourceapi.h"
#include "tech/globalobj.h"

#include "tech/dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1200

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

///////////////////////////////////////////////////////////////////////////////

#else // _MSC_VER <= 1200

void * AnsiTextLoad(IReader * pReader)
{
   if (pReader != NULL)
   {
      ulong length;
      if (pReader->Seek(0, kSO_End) == S_OK
         && pReader->Tell(&length) == S_OK
         && pReader->Seek(0, kSO_Set) == S_OK)
      {
         char * pszContents = new char[length + 1];
         if (pszContents != NULL)
         {
            if (pReader->Read(pszContents, length * sizeof(char)) == S_OK)
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

void * UnicodeTextLoad(IReader * pReader)
{
   if (pReader != NULL)
   {
      ulong length;
      if (pReader->Seek(0, kSO_End) == S_OK
         && pReader->Tell(&length) == S_OK
         && pReader->Seek(0, kSO_Set) == S_OK)
      {
         wchar_t * pszContents = new wchar_t[length + 1];
         if (pszContents != NULL)
         {
            if (pReader->Read(pszContents, length * sizeof(wchar_t)) == S_OK)
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

void AnsiTextUnload(void * pData)
{
   delete [] reinterpret_cast<char*>(pData);
}

void UnicodeTextUnload(void * pData)
{
   delete [] reinterpret_cast<wchar_t*>(pData);
}

#endif


///////////////////////////////////////////////////////////////////////////////

TECH_API tResult TextFormatRegister(const tChar * pszExtensions)
{
   if (pszExtensions == NULL)
   {
      return E_POINTER;
   }

   UseGlobal(ResourceManager);
   if (!pResourceManager)
   {
      return E_FAIL;
   }

   int nRegistered = 0;

   cTokenizer<cStr> extensions;
   int nExtensions = extensions.Tokenize(pszExtensions, _T(","));
   for (int i = 0; i < nExtensions; i++)
   {
      const tChar * pszExtension = extensions.m_tokens[i].c_str();

#if _MSC_VER > 1200
      if (pResourceManager->RegisterFormat(kRT_AsciiText, pszExtension, TextLoad<char>, NULL, TextUnload<char>) == S_OK
         && pResourceManager->RegisterFormat(kRT_UnicodeText, pszExtension, TextLoad<wchar_t>, NULL, TextUnload<wchar_t>) == S_OK)
#else
      if (pResourceManager->RegisterFormat(kRT_AsciiText, pszExtension, AnsiTextLoad, NULL, AnsiTextUnload) == S_OK
         && pResourceManager->RegisterFormat(kRT_UnicodeText, pszExtension, UnicodeTextLoad, NULL, UnicodeTextUnload) == S_OK)
#endif
      {
         nRegistered++;
      }
   }

   return (nRegistered == nExtensions) ? S_OK : S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////
