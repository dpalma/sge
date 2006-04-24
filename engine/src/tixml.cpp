///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "engineapi.h"
#include "readwriteutils.h"

#include "globalobj.h"
#include "readwriteapi.h"
#include "resourceapi.h"

#include <tinyxml.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

void * TiXmlDocumentLoad(IReader * pReader)
{
   if (pReader == NULL)
   {
      return NULL;
   }

   ulong length = 0;
   if (pReader->Seek(0, kSO_End) == S_OK
      && pReader->Tell(&length) == S_OK
      && pReader->Seek(0, kSO_Set) == S_OK)
   {
      cAutoBuffer autoBuffer;
      char stackBuffer[256];
      char * pBuffer = NULL;

      if (length >= 32768)
      {
         WarnMsg1("Sanity check failure loading XML document %d bytes long\n", length);
         return NULL;
      }

      if (length < sizeof(stackBuffer))
      {
         pBuffer = stackBuffer;
      }
      else
      {
         if (autoBuffer.Malloc(sizeof(char) * (length + 1), (void**)&pBuffer) != S_OK)
         {
            return NULL;
         }
      }

      if (pReader->Read(pBuffer, length) == S_OK)
      {
         pBuffer[length] = 0;

         TiXmlDocument * pDoc = new TiXmlDocument;
         if (pDoc != NULL)
         {
            pDoc->Parse(pBuffer);
            if (pDoc->Error())
            {
               delete pDoc;
               return NULL;
            }
            return pDoc;
         }
      }
   }

   return NULL;
}

void TiXmlDocumentUnload(void * pData)
{
   TiXmlDocument * pDoc = reinterpret_cast<TiXmlDocument*>(pData);
   delete pDoc;
}

///////////////////////////////////////////////////////////////////////////////

tResult TiXmlRegisterResourceFormat()
{
   UseGlobal(ResourceManager);
   if (!pResourceManager)
   {
      return E_FAIL;
   }
   return pResourceManager->RegisterFormat(kRT_TiXml, _T("xml"), TiXmlDocumentLoad, NULL, TiXmlDocumentUnload);
}

///////////////////////////////////////////////////////////////////////////////
