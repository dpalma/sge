///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "engine.h"
#include "model.h"
#include "saveloadapi.h"

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

void * MapLoad(IReader * pReader)
{
   // Pass the reader along to the post-load function
   return pReader;
}

void * MapPostload(void * pData, int dataLength, void * param)
{
   IReader * pReader = reinterpret_cast<IReader *>(pData);

   UseGlobal(SaveLoadManager);

   if (param != NULL)
   {
      cAutoIPtr<IReader> pEntryReader;
      if (pSaveLoadManager->OpenSingleEntry(pReader, SAVELOADID_MapProperties, &pEntryReader) == S_OK)
      {
         pEntryReader->Read((cMapProperties*)param);
         // Return NULL to prevent the resource manager from caching
         // the properties struct in place of the actual map
         return NULL;
      }
   }
   else
   {
      if (pSaveLoadManager->Load(pReader) == S_OK)
      {
         return reinterpret_cast<void*>(NULL + 1); // TODO: WTF? This return value is a hack
      }
   }

   return NULL;
}

void MapUnload(void * pData)
{
}


///////////////////////////////////////////////////////////////////////////////

extern tResult SoundResourceRegister(); // soundwin.cpp
extern tResult RendererResourceRegister(); // renderer.cpp
extern tResult ModelMs3dResourceRegister(); // modelms3d.cpp

tResult EngineRegisterResourceFormats()
{
   UseGlobal(ResourceManager);
   if (!!pResourceManager)
   {
      if (ModelMs3dResourceRegister() == S_OK
         && SoundResourceRegister() == S_OK
         && RendererResourceRegister() == S_OK
         && pResourceManager->RegisterFormat(kRT_TiXml, kRT_AsciiText, _T("xml"), NULL, TiXmlDocumentFromText, TiXmlDocumentUnload) == S_OK
         && pResourceManager->RegisterFormat(kRT_Map, kMapExt, MapLoad, MapPostload, MapUnload) == S_OK)
      {
         return S_OK;
      }
   }
   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMapProperties
//

////////////////////////////////////////

cMapProperties::cMapProperties()
 : m_nPlayers(0)
{
}

////////////////////////////////////////

cMapProperties::cMapProperties(const tChar * pszTitle, const tChar * pszAuthor,
                               const tChar * pszDescription, int nPlayers)
 : m_title((pszTitle != NULL) ? pszTitle : _T(""))
 , m_author((pszAuthor != NULL) ? pszAuthor : _T(""))
 , m_description((pszDescription != NULL) ? pszDescription : _T(""))
 , m_nPlayers(nPlayers)
{
}

////////////////////////////////////////

cMapProperties::cMapProperties(const cMapProperties & other)
 : m_title(other.m_title)
 , m_author(other.m_author)
 , m_description(other.m_description)
 , m_nPlayers(other.m_nPlayers)
{
}

////////////////////////////////////////

cMapProperties::~cMapProperties()
{
}

////////////////////////////////////////

const tChar * cMapProperties::GetTitle() const
{
   return m_title.c_str();
}

////////////////////////////////////////

void cMapProperties::SetTitle(const tChar * pszTitle)
{
   if (pszTitle != NULL)
   {
      m_title = pszTitle;
   }
   else
   {
      m_title.erase();
   }
}

////////////////////////////////////////

const tChar * cMapProperties::GetAuthor() const
{
   return m_author.c_str();
}

////////////////////////////////////////

void cMapProperties::SetAuthor(const tChar * pszAuthor)
{
   if (pszAuthor != NULL)
   {
      m_author = pszAuthor;
   }
   else
   {
      m_author.erase();
   }
}

////////////////////////////////////////

const tChar * cMapProperties::GetDescription() const
{
   return m_description.c_str();
}

////////////////////////////////////////

void cMapProperties::SetDescription(const tChar * pszDescription)
{
   if (pszDescription != NULL)
   {
      m_description = pszDescription;
   }
   else
   {
      m_description.erase();
   }
}

////////////////////////////////////////

int cMapProperties::GetNumPlayers() const
{
   return m_nPlayers;
}

////////////////////////////////////////

void cMapProperties::SetNumPlayers(int numPlayers)
{
   m_nPlayers = numPlayers;
}

////////////////////////////////////////

tResult cReadWriteOps<cMapProperties>::Read(IReader * pReader, cMapProperties * pMapProperties)
{
   if (pReader == NULL || pMapProperties == NULL)
   {
      return E_POINTER;
   }

   int version;
   if (pReader->Read(&version) != S_OK || version != 1)
   {
      return E_FAIL;
   }

   if (pReader->Read(&pMapProperties->m_title) == S_OK
      && pReader->Read(&pMapProperties->m_author) == S_OK
      && pReader->Read(&pMapProperties->m_description) == S_OK
      && pReader->Read(&pMapProperties->m_nPlayers) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cReadWriteOps<cMapProperties>::Write(IWriter * pWriter, const cMapProperties & mapProperties)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (pWriter->Write(1) == S_OK // version
      && pWriter->Write(mapProperties.m_title) == S_OK
      && pWriter->Write(mapProperties.m_author) == S_OK
      && pWriter->Write(mapProperties.m_description) == S_OK
      && pWriter->Write(mapProperties.m_nPlayers) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////
