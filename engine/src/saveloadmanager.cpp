////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "saveloadmanager.h"

#include "toposort.h"

#include <vector>

#include "dbgalloc.h" // must be last header


////////////////////////////////////////////////////////////////////////////////

// {8542CD88-7986-450e-B3C3-5ED5BCC23F04}
static const GUID SAVELOADID_SaveLoadFile = {
   0x8542cd88, 0x7986, 0x450e, { 0xb3, 0xc3, 0x5e, 0xd5, 0xbc, 0xc2, 0x3f, 0x4 } };
//DEFINE_GUID(<<name>>, 
//0x8542cd88, 0x7986, 0x450e, 0xb3, 0xc3, 0x5e, 0xd5, 0xbc, 0xc2, 0x3f, 0x4);


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cVersionedParticipant
//

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


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMD5Writer
//

////////////////////////////////////////

cMD5Writer::cMD5Writer(IWriter * pWriter)
 : m_pWriter(CTAddRef(pWriter))
 , m_bUpdateMD5(true)
{
   Assert(pWriter != NULL);
   MD5Init(&m_context);
}

////////////////////////////////////////

cMD5Writer::~cMD5Writer()
{
}

////////////////////////////////////////

void cMD5Writer::FinalizeMD5(byte digest[16])
{
   if (m_bUpdateMD5)
   {
      m_bUpdateMD5 = false;
      MD5Final(digest, &m_context);
   }
}

////////////////////////////////////////

tResult cMD5Writer::Tell(ulong * pPos)
{
   return m_pWriter->Tell(pPos);
}

////////////////////////////////////////

tResult cMD5Writer::Seek(long pos, eSeekOrigin origin)
{
   WarnMsg("Attempting to seek with a MD5 IWriter\n");
   return E_FAIL;
}

////////////////////////////////////////

tResult cMD5Writer::Write(const char * value)
{
   if (m_bUpdateMD5)
   {
      MD5Update(&m_context, const_cast<byte*>(reinterpret_cast<const byte*>(value)), strlen(value));
   }
   return m_pWriter->Write(value);
}

////////////////////////////////////////

tResult cMD5Writer::Write(void * pValue, size_t cbValue, size_t * pcbWritten)
{
   if (m_bUpdateMD5)
   {
      MD5Update(&m_context, static_cast<byte*>(pValue), cbValue);
   }
   return m_pWriter->Write(pValue, cbValue, pcbWritten);
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
class cReadWriteOps<cSaveLoadManager::sFileHeader>
{
public:
   static tResult Read(IReader * pReader, cSaveLoadManager::sFileHeader *);
   static tResult Write(IWriter * pWriter, const cSaveLoadManager::sFileHeader &);
};

tResult cReadWriteOps<cSaveLoadManager::sFileHeader>::Read(IReader * pReader,
                                                           cSaveLoadManager::sFileHeader * pFileHeader)
{
   if (pReader == NULL || pFileHeader == NULL)
   {
      return E_POINTER;
   }

   if (pReader->Read(&pFileHeader->id) == S_OK
      && pReader->Read(&pFileHeader->version) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}

tResult cReadWriteOps<cSaveLoadManager::sFileHeader>::Write(IWriter * pWriter,
                                                            const cSaveLoadManager::sFileHeader & fileHeader)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (pWriter->Write(fileHeader.id) == S_OK
      && pWriter->Write(fileHeader.version) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}


////////////////////////////////////////////////////////////////////////////////

template <>
class cReadWriteOps<cSaveLoadManager::sFileFooter>
{
public:
   static tResult Read(IReader * pReader, cSaveLoadManager::sFileFooter *);
   static tResult Write(IWriter * pWriter, const cSaveLoadManager::sFileFooter &);
};

tResult cReadWriteOps<cSaveLoadManager::sFileFooter>::Read(IReader * pReader,
                                                           cSaveLoadManager::sFileFooter * pFileFooter)
{
   if (pReader == NULL || pFileFooter == NULL)
   {
      return E_POINTER;
   }

   if (pReader->Read(&pFileFooter->offset) == S_OK
      && pReader->Read(&pFileFooter->length) == S_OK
      && pReader->Read(pFileFooter->digest, sizeof(pFileFooter->digest)) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}

tResult cReadWriteOps<cSaveLoadManager::sFileFooter>::Write(IWriter * pWriter,
                                                            const cSaveLoadManager::sFileFooter & fileFooter)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (pWriter->Write(fileFooter.offset) == S_OK
      && pWriter->Write(fileFooter.length) == S_OK
      && pWriter->Write(const_cast<byte *>(fileFooter.digest),
      sizeof(fileFooter.digest)) == S_OK)
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
   tParticipantMap::iterator iter = m_participantMap.begin();
   tParticipantMap::iterator end = m_participantMap.end();
   for (; iter != end; iter++)
   {
      delete iter->second;
   }
   m_participantMap.clear();

   return S_OK;
}

///////////////////////////////////////

tResult cSaveLoadManager::RegisterSaveLoadParticipant(REFGUID id,
                                                      const cBeforeAfterConstraint * pConstraints,
                                                      size_t nConstraints,
                                                      int version,
                                                      ISaveLoadParticipant * pSLP)
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

   // All participants are put into the constraint graph. Only those with
   // constraints will have edges, though.
   Verify(m_saveOrderConstraintGraph.AddNode(&id));

   if (pConstraints != NULL && nConstraints > 0)
   {
      for (size_t i = 0; i < nConstraints; i++)
      {
         const cBeforeAfterConstraint & c = pConstraints[i];

         if (c.GetName() != NULL)
         {
            WarnMsg("Constraints by name not supported by save/load manager\n");
            continue;
         }

         if (c.GetGuid() != NULL)
         {
            // Don't check the existence of the constraint target in the map
            // because it could register after this one at start-up. The save
            // order will be built later when all participants have been registered.
            if (c.Before())
            {
               m_saveOrderConstraintGraph.AddEdge(&id, c.GetGuid());
            }
            else
            {
               m_saveOrderConstraintGraph.AddEdge(c.GetGuid(), &id);
            }
         }
      }
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

   cAutoIPtr<cMD5Writer> pMD5Writer(new cMD5Writer(pWriter));
   if (!pMD5Writer)
   {
      return E_OUTOFMEMORY;
   }

   // Don't use this pointer anymore--use pMD5Writer!
   pWriter = NULL;

   // Determine the save order
   std::vector<const GUID *> saveOrder;
   cTopoSorter<tConstraintGraph>().TopoSort(&m_saveOrderConstraintGraph, &saveOrder);
   AssertMsg(saveOrder.size() == m_participantMap.size(), "Size mismatch after determining constrained save order");

   sFileHeader header;
   memcpy(&header.id, &SAVELOADID_SaveLoadFile, sizeof(header.id));
   header.version = 2;

   // Write the file header
   if (pMD5Writer->Write(header) != S_OK)
   {
      ErrorMsg("Failed to write the file header\n");
      return E_FAIL;
   }

   std::vector<sFileEntry> entries;

   std::vector<const GUID *>::iterator iter = saveOrder.begin();
   for (; iter != saveOrder.end(); iter++)
   {
      cVersionedParticipant * pVP = NULL;
      {
         tParticipantMap::iterator f = m_participantMap.find(*iter);
         if (f == m_participantMap.end())
         {
            ErrorMsg("Save/load participant in the save order not found in the registered list\n");
            continue;
         }
         pVP = f->second;
      }

      int entryVersion = -1;
      if (pVP->GetMostRecentVersion(&entryVersion) != S_OK)
      {
         continue;
      }

      cAutoIPtr<ISaveLoadParticipant> pSLP;
      if (pVP->GetParticipant(entryVersion, &pSLP) != S_OK)
      {
         continue;
      }

      ulong begin;
      if (pMD5Writer->Tell(&begin) != S_OK)
      {
         ErrorMsg("Failed to get the beginning offset of a save entry\n");
         return E_FAIL;
      }

      // S_OK: writing data succeeded
      // S_FALSE: skip this entry, but not error
      // Otherwise, failure
      tResult entrySaveResult = pSLP->Save(pMD5Writer);
      if (FAILED(entrySaveResult))
      {
         ErrorMsg("Failed to write a save/load entry\n");
         return E_FAIL;
      }
      else if (entrySaveResult == S_OK)
      {
         ulong end;
         if (pMD5Writer->Tell(&end) != S_OK)
         {
            ErrorMsg("Failed to get the end offset of a save entry\n");
            return E_FAIL;
         }

         sFileEntry entry;
         memcpy(&entry.id, *iter, sizeof(GUID));
         entry.version = entryVersion;
         entry.offset = begin;
         entry.length = end - begin;
         entries.push_back(entry);
      }
   }

   ulong tableOffset = 0, tableLength = entries.size() * sizeof(sFileEntry);

   // Determine the offset of the entry table
   if (pMD5Writer->Tell(&tableOffset) != S_OK)
   {
      ErrorMsg("Failed to get the offset of the entry table\n");
      return E_FAIL;
   }

   // Write the entry table at the end
   {
      std::vector<sFileEntry>::iterator iter = entries.begin();
      for (; iter != entries.end(); iter++)
      {
         if (pMD5Writer->Write(*iter) != S_OK)
         {
            ErrorMsg("Failed to write the entry table\n");
            return E_FAIL;
         }
      }
   }

   sFileFooter footer = {0};
   footer.offset = tableOffset;
   footer.length = tableLength;

   pMD5Writer->FinalizeMD5(footer.digest);

   if (pMD5Writer->Write(footer) != S_OK)
   {
      ErrorMsg("Failed to write the file footer\n");
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

   // Read the entry table
   std::vector<sFileEntry> entries;
   if (LoadEntryTable(pReader, &entries) != S_OK)
   {
      return E_FAIL;
   }

   // Read the individual entries
   std::vector<sFileEntry>::iterator iter = entries.begin();
   for (; iter != entries.end(); iter++)
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

      // S_OK: loading succeeded
      // S_FALSE: loading refused--seek past this entry
      // Otherwise, failure
      tResult loadResult = pSLP->Load(pReader, entry.version);
      if (FAILED(loadResult))
      {
         ErrorMsg("Failed to read a file entry\n");
         return E_FAIL;
      }
      else if (loadResult == S_FALSE)
      {
         if (pReader->Seek(entry.length, kSO_Cur) != S_OK)
         {
            ErrorMsg("Unable to skip over a file entry refused by its registered loader\n");
            return E_FAIL;
         }
      }
   }

   return S_OK;
}

///////////////////////////////////////

tResult cSaveLoadManager::LoadEntryTable(IReader * pReader, std::vector<sFileEntry> * pEntries)
{
   if (pReader == NULL || pEntries == NULL)
   {
      return E_POINTER;
   }

   // Read the file header. The file header uses the same struct as for a table
   // entry, but the offset field is the position of the table and the length
   // field is the size of the table. That is, the # entries in the table is 
   // header.length / sizeof(sFileEntry).

   sFileHeader header;
   if (pReader->Read(&header) != S_OK)
   {
      ErrorMsg("Failed to read the file header\n");
      return E_FAIL;
   }

   if (memcmp(&header.id, &SAVELOADID_SaveLoadFile, sizeof(GUID)) != 0)
   {
      return E_FAIL;
   }

   ulong tableOffset = 0, tableLength = 0;

   if (header.version == 1)
   {
      // Version 1 had four fields in the header:
      //    GUID file id
      //    int file version
      //    ulong table offset
      //    ulong table size

      if (pReader->Read(&tableOffset) != S_OK)
      {
         ErrorMsg("Failed to read the table offset\n");
         return E_FAIL;
      }

      if (pReader->Read(&tableLength) != S_OK)
      {
         ErrorMsg("Failed to read the table length\n");
         return E_FAIL;
      }
   }
   else if (header.version == 2)
   {
      // Version 2 has two fields in the header:
      //    GUID file id
      //    int file version
      // and a footer:
      //    ulong table offset
      //    ulong table size
      //    16-byte MD5 digest

      ulong fileSize = 0;
      sFileFooter footer = {0};

      if (pReader->Seek(0, kSO_End) != S_OK
         || pReader->Tell(&fileSize) != S_OK
         || pReader->Seek(fileSize - sizeof(footer), kSO_Set) != S_OK
         || pReader->Read(&footer) != S_OK)
      {
         return E_FAIL;
      }

      // Compute the MD5 digest for the file being loaded (exclude the footer)
      byte digest[16];
      memset(digest, 0, sizeof(digest));
      if (pReader->Seek(0, kSO_Set) == S_OK)
      {
         MD5_CTX context;
         MD5Init(&context);
         ulong nReadTotal = 0;
         byte buffer[512];
         size_t nRead = sizeof(buffer);
         while (SUCCEEDED(pReader->Read(buffer, nRead, &nRead)))
         {
            ulong b4ftr = Min(nRead, fileSize - sizeof(sFileFooter) - nReadTotal);
            MD5Update(&context, buffer, b4ftr);
            nReadTotal += nRead;
            if (nReadTotal >= fileSize)
            {
               break;
            }
            nRead = sizeof(buffer);
         }
         MD5Final(digest, &context);
      }

      if (memcmp(digest, footer.digest, sizeof(digest)) != 0)
      {
         ErrorMsg("Invalid file\n");
         return E_FAIL;
      }

      tableOffset = footer.offset;
      tableLength = footer.length;
   }
   else
   {
      return E_FAIL;
   }

   if (tableOffset == 0 || tableLength == 0)
   {
      ErrorMsg("Invalid file\n");
      return E_FAIL;
   }

   // Seek to the entry table
   if (pReader->Seek(tableOffset, kSO_Set) != S_OK)
   {
      ErrorMsg("Failed to seek to the head of the file entry table\n");
      return E_FAIL;
   }

   // Read the entries
   size_t nEntries = tableLength / sizeof(sFileEntry);
   pEntries->clear();
   pEntries->reserve(nEntries);
   for (uint i = 0; i < nEntries; i++)
   {
      sFileEntry entry;
      if (pReader->Read(&entry) != S_OK)
      {
         ErrorMsg1("Failed to read the file table entry %d\n", i);
         return E_FAIL;
      }
      pEntries->push_back(entry);
   }

   return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
