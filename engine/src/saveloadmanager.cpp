////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "saveloadmanager.h"

#include <vector>

#include "dbgalloc.h" // must be last header


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cVersionedParticipant
//

///////////////////////////////////////

static const uint g_saveLoadFileId     = 0x5A5E70AD;

// {8542CD88-7986-450e-B3C3-5ED5BCC23F04}
static const GUID SAVELOADID_SaveLoadFile = {
   0x8542cd88, 0x7986, 0x450e, { 0xb3, 0xc3, 0x5e, 0xd5, 0xbc, 0xc2, 0x3f, 0x4 } };
//DEFINE_GUID(<<name>>, 
//0x8542cd88, 0x7986, 0x450e, 0xb3, 0xc3, 0x5e, 0xd5, 0xbc, 0xc2, 0x3f, 0x4);

static const int g_saveLoadFileVer     = 1;

///////////////////////////////////////

cVersionedParticipant::cVersionedParticipant()
{
}

///////////////////////////////////////

cVersionedParticipant::~cVersionedParticipant()
{
   tVersionMap::iterator iter = m_versionMap.begin();
   tVersionMap::iterator end = m_versionMap.end();
   for (; iter != end; iter++)
   {
      iter->second->Release();
   }
}

///////////////////////////////////////

tResult cVersionedParticipant::AddVersion(int version, ISaveLoadParticipant * pSLP)
{
   if (version < 0)
   {
      return E_INVALIDARG;
   }

   if (pSLP == NULL)
   {
      return E_POINTER;
   }

   if (m_versionMap.find(version) != m_versionMap.end())
   {
      m_versionMap[version]->Release(); 
   }
   m_versionMap[version] = CTAddRef(pSLP);
   return S_OK;
}

///////////////////////////////////////

tResult cVersionedParticipant::RemoveVersion(int version)
{
   if (version < 0)
   {
      return E_INVALIDARG;
   }

   tVersionMap::iterator f = m_versionMap.find(version);
   if (f == m_versionMap.end())
   {
      return S_FALSE;
   }

   f->second->Release();
   m_versionMap.erase(f);
   return S_OK;
}

///////////////////////////////////////

tResult cVersionedParticipant::GetMostRecentVersion(int * pVersion) const
{
   if (pVersion == NULL)
   {
      return E_POINTER;
   }

   int biggestVersion = -1;
   tVersionMap::const_iterator iter = m_versionMap.begin();
   tVersionMap::const_iterator end = m_versionMap.end();
   for (; iter != end; iter++)
   {
      if (iter->first > biggestVersion)
      {
         biggestVersion = iter->first;
      }
   }

   if (biggestVersion == -1)
   {
      return E_FAIL;
   }

   *pVersion = biggestVersion;
   return S_OK;
}

///////////////////////////////////////

tResult cVersionedParticipant::GetParticipant(int version, ISaveLoadParticipant * * ppSLP)
{
   if (version < 0)
   {
      return E_INVALIDARG;
   }

   if (ppSLP == NULL)
   {
      return E_POINTER;
   }

   tVersionMap::iterator f = m_versionMap.find(version);
   if (f == m_versionMap.end())
   {
      return E_FAIL;
   }

   *ppSLP = CTAddRef(f->second);
   return S_OK;
}


////////////////////////////////////////////////////////////////////////////////

template <>
class cReadWriteOps<GUID>
{
public:
   static tResult Read(IReader * pReader, GUID * pGUID);
   static tResult Write(IWriter * pWriter, const GUID & guid);
};

tResult cReadWriteOps<GUID>::Read(IReader * pReader, GUID * pGUID)
{
   if (pReader == NULL || pGUID == NULL)
   {
      return E_POINTER;
   }

   if (pReader->Read(&pGUID->Data1) == S_OK
      && pReader->Read(&pGUID->Data2) == S_OK
      && pReader->Read(&pGUID->Data3) == S_OK
      && pReader->Read(&pGUID->Data4[0], sizeof(pGUID->Data4)) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}

tResult cReadWriteOps<GUID>::Write(IWriter * pWriter, const GUID & guid)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (pWriter->Write(guid.Data1) == S_OK
      && pWriter->Write(guid.Data2) == S_OK
      && pWriter->Write(guid.Data3) == S_OK
      && pWriter->Write((void*)guid.Data4, sizeof(guid.Data4)) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}

////////////////////////////////////////////////////////////////////////////////

template <>
class cReadWriteOps<cSaveLoadManager::sFileEntry>
{
public:
   static tResult Read(IReader * pReader, cSaveLoadManager::sFileEntry *);
   static tResult Write(IWriter * pWriter, const cSaveLoadManager::sFileEntry &);
};

tResult cReadWriteOps<cSaveLoadManager::sFileEntry>::Read(IReader * pReader,
                                                          cSaveLoadManager::sFileEntry * pFileEntry)
{
   if (pReader == NULL || pFileEntry == NULL)
   {
      return E_POINTER;
   }

   if (pReader->Read(&pFileEntry->id) == S_OK
      && pReader->Read(&pFileEntry->version) == S_OK
      && pReader->Read(&pFileEntry->offset) == S_OK
      && pReader->Read(&pFileEntry->length) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}

tResult cReadWriteOps<cSaveLoadManager::sFileEntry>::Write(IWriter * pWriter,
                                                           const cSaveLoadManager::sFileEntry & fileEntry)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (pWriter->Write(fileEntry.id) == S_OK
      && pWriter->Write(fileEntry.version) == S_OK
      && pWriter->Write(fileEntry.offset) == S_OK
      && pWriter->Write(fileEntry.length) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSaveLoadManager
//

///////////////////////////////////////

tResult SaveLoadManagerCreate()
{
   cAutoIPtr<ISaveLoadManager> p(new cSaveLoadManager);
   return RegisterGlobalObject(IID_ISaveLoadManager, static_cast<ISaveLoadManager*>(p));
}

///////////////////////////////////////

cSaveLoadManager::cSaveLoadManager()
{
}

///////////////////////////////////////

cSaveLoadManager::~cSaveLoadManager()
{
}

///////////////////////////////////////

tResult cSaveLoadManager::Init()
{
   return S_OK;
}

///////////////////////////////////////

tResult cSaveLoadManager::Term()
{
   return S_OK;
}

///////////////////////////////////////

tResult cSaveLoadManager::RegisterSaveLoadParticipant(REFGUID id, int version, ISaveLoadParticipant * pSLP)
{
   if (CTIsEqualUnknown(id))
   {
      // Cannot register IID_IUnknown
      return E_INVALIDARG;
   }

   if (version < 0)
   {
      return E_INVALIDARG;
   }

   if (pSLP == NULL)
   {
      return E_POINTER;
   }

   cVersionedParticipant * pParticipant = NULL;
   tParticipantMap::iterator f = m_participantMap.find(&id);
   if (f == m_participantMap.end())
   {
      pParticipant = new cVersionedParticipant;
      if (pParticipant == NULL)
      {
         return E_OUTOFMEMORY;
      }

      m_participantMap[&id] = pParticipant;
   }
   else
   {
      pParticipant = f->second;
   }

   if (pParticipant == NULL)
   {
      return E_FAIL;
   }

   return pParticipant->AddVersion(version, pSLP);
}

///////////////////////////////////////

tResult cSaveLoadManager::RevokeSaveLoadParticipant(REFGUID id, int version)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cSaveLoadManager::Save(IWriter * pWriter)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   sFileEntry header;
   memcpy(&header.id, &SAVELOADID_SaveLoadFile, sizeof(header.id));
   header.version = g_saveLoadFileVer;
   header.offset = 0;
   header.length = 0;

   // Write the file header
   if (pWriter->Write(header) != S_OK)
   {
      ErrorMsg("Failed to write the file header\n");
      return E_FAIL;
   }

   std::vector<sFileEntry> entries;

   tParticipantMap::iterator iter = m_participantMap.begin();
   tParticipantMap::iterator end = m_participantMap.end();
   for (; iter != end; iter++)
   {
      int entryVersion = -1;
      if (iter->second->GetMostRecentVersion(&entryVersion) != S_OK)
      {
         continue;
      }

      cAutoIPtr<ISaveLoadParticipant> pSLP;
      if (iter->second->GetParticipant(entryVersion, &pSLP) != S_OK)
      {
         continue;
      }

      ulong begin;
      if (pWriter->Tell(&begin) != S_OK)
      {
         ErrorMsg("Failed to get the beginning offset of a save entry\n");
         return E_FAIL;
      }

      // S_OK: writing data succeeded
      // S_FALSE: skip this entry, but not error
      // Otherwise, failure
      tResult entrySaveResult = pSLP->Save(pWriter);
      if (FAILED(entrySaveResult))
      {
         ErrorMsg("Failed to write a save/load entry\n");
         return E_FAIL;
      }
      else if (entrySaveResult == S_OK)
      {
         ulong end;
         if (pWriter->Tell(&end) != S_OK)
         {
            ErrorMsg("Failed to get the end offset of a save entry\n");
            return E_FAIL;
         }

         sFileEntry entry;
         memcpy(&entry.id, iter->first, sizeof(GUID));
         entry.version = entryVersion;
         entry.offset = begin;
         entry.length = end - begin;
         entries.push_back(entry);
      }
   }

   // Determine the offset of the entry table
   if (pWriter->Tell(&header.offset) != S_OK)
   {
      ErrorMsg("Failed to get the offset of the entry table\n");
      return E_FAIL;
   }

   // Write the entry table at the end
   {
      std::vector<sFileEntry>::iterator iter = entries.begin();
      std::vector<sFileEntry>::iterator end = entries.end();
      for (; iter != end; iter++)
      {
         if (pWriter->Write(*iter) != S_OK)
         {
            ErrorMsg("Failed to write the entry table\n");
            return E_FAIL;
         }
      }
   }

   // Re-write the file header now that the table size and position are known
   Assert(header.offset > 0);
   header.length = entries.size();
   if (pWriter->Seek(0, kSO_Set) != S_OK
      || pWriter->Write(header) != S_OK)
   {
      ErrorMsg("Failed to re-write the file header\n");
      return E_FAIL;
   }

   return S_OK;
}

///////////////////////////////////////

tResult cSaveLoadManager::Load(IReader * pReader)
{
   if (pReader == NULL)
   {
      return E_POINTER;
   }

   // Read the file header
   sFileEntry header;
   if (pReader->Read(&header) != S_OK)
   {
      ErrorMsg("Failed to read the file header\n");
      return E_FAIL;
   }

   if (memcmp(&header.id, &SAVELOADID_SaveLoadFile, sizeof(GUID)) != 0)
   {
      return E_FAIL;
   }

   if (header.version != g_saveLoadFileVer)
   {
      return E_FAIL;
   }

   if (header.offset == 0 || header.length == 0)
   {
      ErrorMsg("Invalid save/load file\n");
      return E_FAIL;
   }

   // Read the entry table itself
   std::vector<sFileEntry> entries(header.length);
   if (pReader->Seek(header.offset, kSO_Set) != S_OK)
   {
      ErrorMsg("Failed to seek to the head of the file entry table\n");
      return E_FAIL;
   }

   for (uint i = 0; i < entries.size(); i++)
   {
      if (pReader->Read(&entries[i]) != S_OK)
      {
         ErrorMsg1("Failed to read the file table entry %d\n", i);
         return E_FAIL;
      }
   }

   // Read the individual entries
   std::vector<sFileEntry>::iterator iter = entries.begin();
   std::vector<sFileEntry>::iterator end = entries.end();
   for (; iter != end; iter++)
   {
      const sFileEntry & entry = *iter;

      tParticipantMap::iterator f = m_participantMap.find(&entry.id);
      if (f == m_participantMap.end())
      {
         WarnMsg("Unable to find reader for file entry\n");
         continue;
      }

      cAutoIPtr<ISaveLoadParticipant> pSLP;
      if (f->second->GetParticipant(entry.version, &pSLP) != S_OK)
      {
         ErrorMsg1("Unable to get reader for file entry version %d\n", entry.version);
         return E_FAIL;
      }

      if (pReader->Seek(entry.offset, kSO_Set) != S_OK)
      {
         ErrorMsg("Unable to seek to file entry while reading\n");
         return E_FAIL;
      }

      // Both S_OK and S_FALSE are valid return values
      if (FAILED(pSLP->Load(pReader, entry.version)))
      {
         ErrorMsg("Failed to read a file entry\n");
         return E_FAIL;
      }
   }

   return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
