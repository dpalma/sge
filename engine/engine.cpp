///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "engine.h"
#include "model.h"
#include "tech/readwriteutils.h"
#include "engine/saveloadapi.h"

#include "render/renderapi.h"

#include "tech/globalobj.h"
#include "tech/resourceapi.h"
#include "tech/techmath.h"

#include <ctime>

#include "tech/dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

cRand g_engineRand;


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

void * MapPropertiesLoad(IReader * pReader)
{
   UseGlobal(SaveLoadManager);

   cAutoIPtr<IReader> pEntryReader;
   if (pSaveLoadManager->OpenSingleEntry(pReader, SAVELOADID_MapProperties, &pEntryReader) == S_OK)
   {
      cMapProperties * pMapProps = new cMapProperties;
      if (pMapProps != NULL
         && pEntryReader->Read(pMapProps) == S_OK)
      {
         return pMapProps;
      }
   }

   return NULL;
}

void MapPropertiesUnload(void * pData)
{
   delete (cMapProperties *)pData;
}


///////////////////////////////////////////////////////////////////////////////

extern tResult ModelSgemResourceRegister(); // modelsgem.cpp
extern tResult TiXmlRegisterResourceFormat(); // tixml.cpp

tResult EngineRegisterResourceFormats()
{
   g_engineRand.Seed(time(NULL));

   UseGlobal(ResourceManager);
   if (!!pResourceManager)
   {
      if (ModelSgemResourceRegister() == S_OK
         && TiXmlRegisterResourceFormat() == S_OK
         && pResourceManager->RegisterFormat(kRT_MapProperties, kMapExt, MapPropertiesLoad, NULL, MapPropertiesUnload) == S_OK)
      {
         return S_OK;
      }
   }
   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////
