///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "engine.h"
#include "model.h"

#include "resourceapi.h"
#include "globalobj.h"

#include <tinyxml.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

void * TiXmlDocumentFromText(void * pData, int dataLength, void * param)
{
   TiXmlBase::SetCondenseWhiteSpace(false);

   char * psz = reinterpret_cast<char*>(pData);
   if (psz != NULL && strlen(psz) > 0)
   {
      TiXmlDocument * pDoc = new TiXmlDocument;
      if (pDoc != NULL)
      {
         pDoc->Parse(psz);
         bool bError = pDoc->Error();
         if (bError)
         {
            delete pDoc;
            pDoc = NULL;
         }
         return pDoc;
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

extern tResult SoundResourceRegister(); // soundwin.cpp
extern tResult RendererResourceRegister(); // renderer.cpp

tResult EngineRegisterResourceFormats()
{
   UseGlobal(ResourceManager);
   if (!!pResourceManager)
   {
      if (cModel::RegisterResourceFormat() == S_OK
         && SoundResourceRegister() == S_OK
         && RendererResourceRegister() == S_OK
         && pResourceManager->RegisterFormat(kRT_TiXml, kRT_AsciiText, _T("xml"), NULL, TiXmlDocumentFromText, TiXmlDocumentUnload) == S_OK)
      {
         return S_OK;
      }
   }
   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////
