////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "resourcemanager.h"
#include "fileenum.h"
#include "filespec.h"
#include "readwriteapi.h"

#include <cstdio>
#include <vector>
#include <algorithm>
#include <set>

#include "dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(ResourceManager);

#define LocalMsg(msg)            DebugMsgEx(ResourceManager,msg)
#define LocalMsg1(msg,a)         DebugMsgEx1(ResourceManager,msg,(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(ResourceManager,msg,(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(ResourceManager,msg,(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)   DebugMsgEx4(ResourceManager,msg,(a),(b),(c),(d))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(ResourceManager,(cond),msg)
#define LocalMsgIf1(cond,msg,a)        DebugMsgIfEx1(ResourceManager,(cond),msg,(a))
#define LocalMsgIf2(cond,msg,a,b)      DebugMsgIfEx2(ResourceManager,(cond),msg,(a),(b))
#define LocalMsgIf3(cond,msg,a,b,c)    DebugMsgIfEx3(ResourceManager,(cond),msg,(a),(b),(c))
#define LocalMsgIf4(cond,msg,a,b,c,d)  DebugMsgIfEx4(ResourceManager,(cond),msg,(a),(b),(c),(d))

static const int kUnzMaxPath = 260;

static const tChar kExtSep = _T('.');

// {93BA1F78-3FF1-415b-BA5B-56FED039E838}
static const GUID IID_IResourceManagerDiagnostics = 
{ 0x93ba1f78, 0x3ff1, 0x415b, { 0xba, 0x5b, 0x56, 0xfe, 0xd0, 0x39, 0xe8, 0x38 } };


// REFERENCES
// "Game Developer Magazine", February 2005, "Inner Product" column

////////////////////////////////////////////////////////////////////////////////

typedef std::vector<cStr> tStrings;
static size_t ListDirs(const cFilePath & path, tStrings * pDirs)
{
   Assert(pDirs != NULL);
   if (pDirs == NULL)
      return 0;

   pDirs->clear();

   cFileSpec wildcard(_T("*"));
   wildcard.SetPath(path);
   cAutoIPtr<IEnumFiles> pEnumFiles;
   if (EnumFiles(wildcard, &pEnumFiles) == S_OK)
   {
      cFileSpec file;
      uint attribs;
      ulong nFiles;
      while (pEnumFiles->Next(1, &file, &attribs, &nFiles) == S_OK)
      {
         if ((attribs & kFA_Directory) == kFA_Directory)
         {
            pDirs->push_back(file);
         }
      }
   }

   return pDirs->size();
}

////////////////////////////////////////////////////////////////////////////////

static bool SameType(tResourceType lhs, tResourceType rhs)
{
   if (lhs != NULL && rhs != NULL)
   {
      uint16 lhsHiWord = (uint16)(((ulong)lhs >> 16) & 0xFFFF);
      uint16 rhsHiWord = (uint16)(((ulong)rhs >> 16) & 0xFFFF);
      if (lhsHiWord == 0 && rhsHiWord == 0)
      {
         return lhs == rhs;
      }
      else if (lhsHiWord != 0 && rhsHiWord != 0)
      {
         return _tcscmp(lhs, rhs) == 0;
      }
   }
   else if (lhs == NULL && rhs == NULL)
   {
      return true;
   }

   return false;
}

////////////////////////////////////////////////////////////////////////////////

static const tChar * ResourceTypeName(tResourceType rt)
{
   uint16 hiWord = (uint16)(((ulong)rt >> 16) & 0xFFFF);
   if (hiWord == 0)
   {
      static const struct
      {
         eResourceClass rc;
         const tChar * pszName;
      }
      builtInTypeNames[] =
      {
         { kRC_Unknown,    _T("Unknown") },
         { kRC_Text,       _T("Text") },
      };
      if ((uint)rt >= _countof(builtInTypeNames))
      {
         rt = 0;
      }
      return builtInTypeNames[(uint)rt].pszName;
   }
   else
   {
      return rt;
   }
}


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceManager
//

////////////////////////////////////////

cResourceManager::cResourceManager()
{
}

////////////////////////////////////////

cResourceManager::~cResourceManager()
{
}

////////////////////////////////////////

tResult cResourceManager::Init()
{
   return S_OK;
}

////////////////////////////////////////

tResult cResourceManager::Term()
{
   tResources::iterator resIter = m_resources.begin();
   tResources::iterator resEnd = m_resources.end();
   for (; resIter != resEnd; resIter++)
   {
      if (resIter->pData != NULL)
      {
         if (resIter->formatId != kNoIndex)
         {
            const sFormat & format = m_formats[resIter->formatId];
            if (format.pfnUnload != NULL)
            {
               LocalMsg2("Unloading \"%s\" (%s)\n", resIter->name.c_str(), ResourceTypeName(format.type));
               (*format.pfnUnload)(resIter->pData);
               resIter->pData = NULL;
               resIter->dataSize = 0;
            }
         }
      }
   }
   m_resources.clear();

   tArchives::iterator arIter = m_archives.begin();
   tArchives::iterator arEnd = m_archives.end();
   for (; arIter != arEnd; arIter++)
   {
      if (arIter->handle != NULL)
      {
         unzClose(arIter->handle);
         arIter->handle = NULL;
      }
   }
   m_archives.clear();

   return S_OK;
}

////////////////////////////////////////

tResult cResourceManager::AddDirectory(const tChar * pszDir)
{
   if (pszDir == NULL)
   {
      return E_POINTER;
   }

   cFileSpec wildcard(_T("*.*"));
   wildcard.SetPath(cFilePath(pszDir));
   cAutoIPtr<IEnumFiles> pEnumFiles;
   if (EnumFiles(wildcard, &pEnumFiles) == S_OK)
   {
      cFileSpec file;
      uint attribs;
      ulong nFiles;
      while (pEnumFiles->Next(1, &file, &attribs, &nFiles) == S_OK)
      {
         if ((attribs & kFA_Directory) == kFA_Directory)
         {
            LocalMsg1("Dir: %s\n", file.c_str());
         }
         else
         {
            LocalMsg1("File: %s\n", file.c_str());
            sResource res;
            Verify(file.GetFileNameNoExt(&res.name));
            const tChar * pszExt = file.GetFileExt();
            if (pszExt != NULL && _tcslen(pszExt) > 0)
            {
               res.extensionId = GetExtensionId(pszExt);
            }
            res.dirId = GetDirectoryId(pszDir);
            m_resources.push_back(res);
         }
      }
   }

   return S_OK;
}

////////////////////////////////////////

tResult cResourceManager::AddDirectoryTreeFlattened(const tChar * pszDir)
{
   if (pszDir == NULL)
   {
      return E_POINTER;
   }

   cFilePath root(pszDir);
   root.MakeFullPath();
   if (AddDirectory(root.c_str()) != S_OK)
   {
      return E_FAIL;
   }

   tStrings dirs;
   if (ListDirs(root, &dirs) > 0)
   {
      tStrings::iterator iter;
      for (iter = dirs.begin(); iter != dirs.end(); iter++)
      {
         if (AddDirectoryTreeFlattened(iter->c_str()) != S_OK)
         {
            return E_FAIL;
         }
      }
   }

   return S_OK;
}

////////////////////////////////////////

tResult cResourceManager::AddArchive(const tChar * pszArchive)
{
   uint archiveId = GetArchiveId(pszArchive);

   unzFile uf = NULL;
   if (m_archives[archiveId].handle != NULL)
   {
      uf = m_archives[archiveId].handle;
   }
   else
   {
#ifdef _UNICODE
      size_t size = wcstombs(NULL, pszArchive, 0);
      char * pszTemp = reinterpret_cast<char*>(alloca(size));
      wcstombs(pszTemp, pszArchive, size);
      uf = m_archives[archiveId].handle = unzOpen(pszTemp);
#else
      uf = m_archives[archiveId].handle = unzOpen(pszArchive);
#endif
      if (uf == NULL)
      {
         return E_FAIL;
      }
   }

   do
   {
      unz_file_pos filePos;
      unz_file_info fileInfo;
      char szFile[kUnzMaxPath];
      if (unzGetFilePos(uf, &filePos) == UNZ_OK &&
         unzGetCurrentFileInfo(uf, &fileInfo, szFile, _countof(szFile), NULL, 0, NULL, 0) == UNZ_OK)
      {
         LocalMsg3("%s(%d): %s\n", pszArchive, filePos.num_of_file, szFile);
#ifdef _UNICODE
         size_t size = mbstowcs(NULL, szFile, 0);
         wchar_t * pszTemp = reinterpret_cast<wchar_t*>(alloca(size));
         mbstowcs(pszTemp, szFile, size);
         cFileSpec file(pszTemp);
#else
         cFileSpec file(szFile);
#endif
         sResource res;
         Verify(file.GetFileNameNoExt(&res.name));
         const tChar * pszExt = file.GetFileExt();
         if (pszExt != NULL && _tcslen(pszExt) > 0)
         {
            res.extensionId = GetExtensionId(pszExt);
         }
         res.archiveId = archiveId;
         res.offset = filePos.pos_in_zip_directory;
         res.index = filePos.num_of_file;
         m_resources.push_back(res);
      }
   }
   while (unzGoToNextFile(uf) == UNZ_OK);

   return S_OK;
}

////////////////////////////////////////

tResult cResourceManager::Load(const tChar * pszName, tResourceType type,
                               void * param, void * * ppData)
{
   if (pszName == NULL || ppData == NULL)
   {
      return E_POINTER;
   }

   if (!type)
   {
      return E_INVALIDARG;
   }

   uint formatIds[10];
   uint nFormats = DeduceFormats(pszName, type, formatIds, _countof(formatIds));
   for (uint i = 0; i <  nFormats; i++)
   {
      if (LoadWithFormat(pszName, type, formatIds[i], param, ppData) == S_OK)
      {
         return S_OK;
      }
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cResourceManager::LoadWithFormat(const tChar * pszName, tResourceType type,
                                         uint formatId, void * param, void * * ppData)
{
   Assert(pszName != NULL);
   Assert(type != NULL);
   Assert(formatId != kNoIndex);
   Assert(ppData != NULL);

   sFormat * pFormat = &m_formats[formatId];

   sResource * pRes = FindResourceWithFormat(pszName, type, formatId);

   // If no resource and format specifies a dependent type then it
   // would not have been pre-loaded in AddDirectory or AddArchive.
   if (pRes == NULL)
   {
      if (pFormat->typeDepend)
      {
         LocalMsg3("Request for (\"%s\", %s) will be converted from type %s\n",
            pszName, ResourceTypeName(type), ResourceTypeName(pFormat->typeDepend));

         // TODO: load the dependent data before adding a new resource
         void * pDependData = NULL;
         if (Load(pszName, pFormat->typeDepend, param, &pDependData) == S_OK)
         {
            void * pData = (*pFormat->pfnPostload)(pDependData, 0, param);
            if (pData != NULL)
            {
               sResource res;
               cFileSpec(pszName).GetFileNameNoExt(&res.name);
               res.extensionId = GetExtensionIdForName(pszName);
               res.formatId = formatId;
               res.pData = pData;
               m_resources.push_back(res);
               *ppData = pData;
               return S_OK;
            }
         }
      }

      // Loading via a dependent type failed above--don't bother continuing
      // (Or, had no dependent type to load from in the first place.)
      return E_FAIL;
   }

   if (!pFormat->typeDepend && pRes->pData == NULL)
   {
      tResult result = E_FAIL;
      ulong dataSize = 0;
      void * pData = NULL;

      if (pRes->dirId != kNoIndex)
      {
         cFileSpec file(pszName);
         file.SetFileExt(m_extensions[pRes->extensionId].c_str());
         file.SetPath(m_dirs[pRes->dirId]);
         result = DoLoadFromFile(file, pFormat, param, &dataSize, &pData);
      }
      else if (pRes->archiveId != kNoIndex)
      {
         result = DoLoadFromArchive(pRes->archiveId, pRes->offset, pRes->index, pFormat, param, &dataSize, &pData);
      }

      if (result == S_OK)
      {
         // Cache the resource data
         pRes->formatId = formatId;
         pRes->pData = pData;
         pRes->dataSize = dataSize;
      }
   }

   if (pRes->pData != NULL)
   {
      *ppData = pRes->pData;
      return S_OK;
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cResourceManager::Unload(const tChar * pszName, tResourceType type)
{
   // TODO: For now resources unloaded only on exit
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cResourceManager::Lock(const tChar * pszName, tResourceType type)
{
   // TODO
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cResourceManager::Unlock(const tChar * pszName, tResourceType type)
{
   // TODO
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cResourceManager::RegisterFormat(tResourceType type,
                                         tResourceType typeDepend,
                                         const tChar * pszExtension,
                                         tResourceLoad pfnLoad,
                                         tResourcePostload pfnPostload,
                                         tResourceUnload pfnUnload)
{
   if (!type)
   {
      return E_INVALIDARG;
   }

   if (typeDepend)
   {
      if (pfnLoad != NULL)
      {
         WarnMsg("Dependent resource type loader specifies a load \
            function which will never be called\n");
      }
   }

   if (pfnLoad == NULL && !typeDepend)
   {
      // Must have at least a load function
      WarnMsg1("No load function specified when registering \"%s\" resource format\n",
            pszExtension != NULL ? pszExtension : _T("<NONE>"));
      return E_POINTER;
   }

   if (pfnUnload == NULL)
   {
      WarnMsg1("No unload function specified for \"%s\" resource format\n",
            pszExtension != NULL ? pszExtension : _T("<NONE>"));
   }

   uint extensionId = kNoIndex;
   if (pszExtension != NULL)
   {
      extensionId = GetExtensionId(pszExtension);

      if (!typeDepend)
      {
         m_extsForType.insert(std::make_pair(ResourceTypeName(type), extensionId));
      }

      tFormats::const_iterator iter = m_formats.begin();
      for (; iter != m_formats.end(); iter++)
      {
         if (iter->extensionId == extensionId && SameType(iter->type, type))
         {
            WarnMsg1("Resource format with file extension \"%s\" already registered\n",
               pszExtension != NULL ? pszExtension : _T("<NONE>"));
            return E_FAIL;
         }
      }
   }

   sFormat format;
   format.type = type;
   format.typeDepend = typeDepend;
   format.extensionId = extensionId;
   format.pfnLoad = pfnLoad;
   format.pfnPostload = pfnPostload;
   format.pfnUnload = pfnUnload;
   m_formats.push_back(format);

   return S_OK;
}

////////////////////////////////////////

void cResourceManager::DumpFormats() const
{
   techlog.Print(kInfo, "%d resource formats\n", m_formats.size());
   static const int kTypeWidth = -20;
   static const int kExtWidth = -5;
   static const tChar kRowFormat[] = "%*s | %*s | %*s\n";
   techlog.Print(kInfo, kRowFormat, kTypeWidth, "Type", kTypeWidth, "Dep. Type", kExtWidth, "Ext");
   techlog.Print(kInfo, "----------------------------------------------------------------------\n");
   tFormats::const_iterator iter = m_formats.begin();
   tFormats::const_iterator end = m_formats.end();
   for (uint index = 0; iter != end; iter++, index++)
   {
      techlog.Print(kInfo, kRowFormat,
         kTypeWidth, ResourceTypeName(iter->type),
         kTypeWidth, iter->typeDepend ? ResourceTypeName(iter->typeDepend) : "None",
         kExtWidth, iter->extensionId != kNoIndex ? m_extensions[iter->extensionId].c_str() : "None");
   }
}

////////////////////////////////////////

void cResourceManager::DumpCache() const
{
   techlog.Print(kInfo, "%d resource cache entries\n", m_resources.size());
   static const int kNameWidth = -30;
   static const int kExtWidth = -5;
   static const int kTypeWidth = -20;
   static const tChar kRowFormat[] = "%*s | %*s | %*s\n";
   techlog.Print(kInfo, kRowFormat, kNameWidth, "Name", kExtWidth, "Ext", kTypeWidth, "Type");
   techlog.Print(kInfo, "----------------------------------------------------------------------\n");
   tResources::const_iterator iter = m_resources.begin();
   tResources::const_iterator end = m_resources.end();
   for (uint index = 0; iter != end; iter++, index++)
   {
      const sFormat * pFormat = (iter->formatId != kNoIndex) ? &m_formats[iter->formatId] : NULL;
      techlog.Print(kInfo, kRowFormat,
         kNameWidth, !iter->name.empty() ? iter->name.c_str() : "Empty",
         kExtWidth, iter->extensionId != kNoIndex ? m_extensions[iter->extensionId].c_str() : "None",
         kTypeWidth, pFormat ? ResourceTypeName(pFormat->type) : "Undetermined");
   }
}

////////////////////////////////////////

void DumpLoadedResources()
{
   cAutoIPtr<IResourceManagerDiagnostics> pResMgrDiag;
   UseGlobal(ResourceManager);
   if (pResourceManager->QueryInterface(IID_IResourceManagerDiagnostics, (void**)&pResMgrDiag) == S_OK)
   {
      pResMgrDiag->DumpCache();
   }
}

////////////////////////////////////////

void DumpResourceFormats()
{
   cAutoIPtr<IResourceManagerDiagnostics> pResMgrDiag;
   UseGlobal(ResourceManager);
   if (pResourceManager->QueryInterface(IID_IResourceManagerDiagnostics, (void**)&pResMgrDiag) == S_OK)
   {
      pResMgrDiag->DumpFormats();
   }
}

////////////////////////////////////////

cResourceManager::sResource * cResourceManager::FindResourceWithFormat(
   const tChar * pszName, tResourceType type, uint formatId)
{
   if (pszName == NULL || formatId == kNoIndex)
   {
      return NULL;
   }

   std::set<uint> extsPossible;
   uint extensionId = GetExtensionIdForName(pszName);
   if (extensionId == kNoIndex)
   {
      // Deduce possible extensions using the type
      tExtsForType::iterator iter = m_extsForType.lower_bound(type);
      tExtsForType::iterator end = m_extsForType.upper_bound(type);
      for (; iter != end; ++iter)
      {
         extsPossible.insert(iter->second);
      }
   }

   cStr name;
   cFileSpec(pszName).GetFileNameNoExt(&name);

   // Will point to an unloaded resource at the end of the loop
   sResource * pPotentialMatch = NULL;
   int nPotentialMatches = 0;

   tResources::iterator resIter = m_resources.begin();
   tResources::iterator resEnd = m_resources.end();
   for (uint index = 0; resIter != resEnd; resIter++, index++)
   {
      if (resIter->name == name)
      {
         if (extensionId == kNoIndex
            && (extsPossible.find(resIter->extensionId) != extsPossible.end()))
         {
            pPotentialMatch = &m_resources[index];
            nPotentialMatches++;
         }
         else if (extensionId == resIter->extensionId)
         {
            if (resIter->formatId == kNoIndex && !m_formats[formatId].typeDepend
               && (resIter->archiveId != kNoIndex || resIter->dirId != kNoIndex))
            {
               pPotentialMatch = &m_resources[index];
            }
            else if (resIter->formatId == formatId)
            {
               pPotentialMatch = &m_resources[index];
               break;
            }
         }
      }
   }

   LocalMsgIf3(nPotentialMatches > 0, "%d potential matches for %s, %s\n",
      nPotentialMatches, pszName, ResourceTypeName(type));

   return pPotentialMatch;
}

////////////////////////////////////////

tResult cResourceManager::DoLoadFromFile(const cFileSpec & file, const sFormat * pFormat,
                                         void * param, ulong * pDataSize, void * * ppData)
{
   if (pDataSize == NULL || ppData == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IReader> pReader(FileCreateReader(file));
   if (!pReader)
   {
      return E_OUTOFMEMORY;
   }

   pReader->Seek(0, kSO_End);
   ulong dataSize;
   if (pReader->Tell(&dataSize) != S_OK)
   {
      return E_FAIL;
   }
   pReader->Seek(0, kSO_Set);

   if (DoLoadFromReader(pReader, pFormat, dataSize, param, ppData) == S_OK)
   {
      *pDataSize = dataSize;
      return S_OK;
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cResourceManager::DoLoadFromArchive(uint archiveId, ulong offset, ulong index,
                                            const sFormat * pFormat, void * param,
                                            ulong * pDataSize, void * * ppData)
{
   if (archiveId == kNoIndex || offset == kNoIndexL || index == kNoIndexL)
   {
      return E_INVALIDARG;
   }

   if (pDataSize == NULL || ppData == NULL)
   {
      return E_POINTER;
   }

   unzFile uf = m_archives[archiveId].handle;
   if (uf == NULL)
   {
      // Could actually reload the zip file on the fly using unzOpen(m_archives[...].archive)
      return E_FAIL;
   }

   unz_file_pos filePos;
   filePos.pos_in_zip_directory = offset;
   filePos.num_of_file = index;
   if (unzGoToFilePos(uf, &filePos) != UNZ_OK)
   {
      return E_FAIL;
   }

   tResult result = E_FAIL;

   unz_file_info fileInfo;
   char szFile[kUnzMaxPath];
   if (unzGetCurrentFileInfo(uf, &fileInfo, szFile, _countof(szFile), NULL, 0, NULL, 0) == UNZ_OK)
   {
      byte * pBuffer = new byte[fileInfo.uncompressed_size];
      if (pBuffer == NULL)
      {
         return E_OUTOFMEMORY;
      }

      if (unzOpenCurrentFile(uf) == UNZ_OK)
      {
         cAutoIPtr<IReader> pReader;
         if (unzReadCurrentFile(uf, pBuffer, fileInfo.uncompressed_size) >= 0
            && ReaderCreateMem(pBuffer, fileInfo.uncompressed_size, false, &pReader) == S_OK)
         {
            result = DoLoadFromReader(pReader, pFormat, fileInfo.uncompressed_size, param, ppData);
            if (result == S_OK)
            {
               *pDataSize = fileInfo.uncompressed_size;
            }
         }

         unzCloseCurrentFile(uf);
      }

      delete [] pBuffer;
   }

   return result;
}

////////////////////////////////////////

tResult cResourceManager::DoLoadFromReader(IReader * pReader, const sFormat * pFormat, ulong dataSize,
                                           void * param, void * * ppData)
{
   if (pReader == NULL || ppData == NULL)
   {
      return E_POINTER;
   }

   if (pFormat->pfnLoad != NULL)
   {
      void * pData = (*pFormat->pfnLoad)(pReader);
      if (pData != NULL)
      {
         if (pFormat->pfnPostload != NULL)
         {
            // Assume the postload function cleans up pData or passes
            // it through (or returns NULL)
            pData = (*pFormat->pfnPostload)(pData, dataSize, param);
         }

         if (pData != NULL)
         {
            *ppData = pData;
            return S_OK;
         }
      }
   }

   return E_FAIL;
}

////////////////////////////////////////

uint cResourceManager::DeduceFormats(const tChar * pszName, tResourceType type,
                                     uint * pFormatIds, uint nMaxFormats)
{
   if (pszName == NULL || !type || pFormatIds == NULL || nMaxFormats == 0)
   {
      return 0;
   }

   LocalMsg2("Deducing resource formats for (\"%s\", %s)...\n", pszName, ResourceTypeName(type));

   uint extensionId = GetExtensionIdForName(pszName);

   uint iFormat = 0;

   // if name has file extension, resource class plus extension determines format
   // plus, include all formats that can generate the resource class from a dependent type
   if (extensionId != kNoIndex)
   {
      Assert(extensionId < m_extensions.size());
      tFormats::const_iterator fIter = m_formats.begin();
      tFormats::const_iterator fEnd = m_formats.end();
      for (uint index = 0; (fIter != fEnd) && (iFormat < nMaxFormats); fIter++, index++)
      {
         if (SameType(fIter->type, type))
         {
            if ((fIter->extensionId == extensionId) || fIter->typeDepend)
            {
               pFormatIds[iFormat] = index;
               iFormat += 1;
            }
         }
      }
   }
   // else resource class alone determines set of possible formats
   else
   {
      tFormats::const_iterator fIter = m_formats.begin();
      tFormats::const_iterator fEnd = m_formats.end();
      for (uint index = 0; (fIter != fEnd) && (iFormat < nMaxFormats); fIter++, index++)
      {
         if (SameType(fIter->type, type))
         {
            pFormatIds[iFormat] = index;
            iFormat += 1;
         }
      }
   }

   LocalMsgIf(iFormat == 0, "   No compatible formats\n");
   LocalMsgIf1(iFormat == 1, "   Single compatible format: %s\n", ResourceTypeName(m_formats[pFormatIds[0]].type));
   LocalMsgIf(iFormat > 1, "   Multiple compatible formats\n");
   return iFormat;
}

////////////////////////////////////////

uint cResourceManager::GetExtensionId(const tChar * pszExtension)
{
   Assert(pszExtension != NULL);

   std::vector<cStr>::const_iterator f = std::find(m_extensions.begin(), m_extensions.end(), pszExtension);
   if (f == m_extensions.end())
   {
      m_extensions.push_back(pszExtension);
      uint index = m_extensions.size() - 1;
      LocalMsg2("File extension %s has id %d\n", pszExtension, index);
      return index;
   }

   return f - m_extensions.begin();
}

////////////////////////////////////////

uint cResourceManager::GetExtensionIdForName(const tChar * pszName)
{
   if (pszName == NULL)
   {
      return kNoIndex;
   }
   const tChar * pszExt = _tcsrchr(pszName, kExtSep);
   if (pszExt != NULL)
   {
      return GetExtensionId(_tcsinc(pszExt));
   }
   return kNoIndex;
}

////////////////////////////////////////

uint cResourceManager::GetDirectoryId(const tChar * pszDir)
{
   Assert(pszDir != NULL);

   cFilePath dir(pszDir);

   std::vector<cFilePath>::iterator f = std::find(m_dirs.begin(), m_dirs.end(), dir);
   if (f == m_dirs.end())
   {
      m_dirs.push_back(dir);
      return m_dirs.size() - 1;
   }

   return f - m_dirs.begin();
}

////////////////////////////////////////

uint cResourceManager::GetArchiveId(const tChar * pszArchive)
{
   Assert(pszArchive != NULL);

   tArchives::iterator iter = m_archives.begin();
   tArchives::iterator end = m_archives.end();
   for (uint index = 0; iter != end; iter++)
   {
      if (_tcsicmp(pszArchive, iter->archive.c_str()) == 0)
      {
         return index;
      }
   }

   sArchiveInfo archiveInfo;
   archiveInfo.archive = pszArchive;
   archiveInfo.handle = NULL;
   m_archives.push_back(archiveInfo);
   return m_archives.size() - 1;
}

////////////////////////////////////////

void ResourceManagerCreate()
{
   cAutoIPtr<IResourceManager> p(new cResourceManager);
   RegisterGlobalObject(IID_IResourceManager, static_cast<IResourceManager*>(p));
}

////////////////////////////////////////

cResourceManager::sResource::sResource()
 : name() 
 , extensionId(kNoIndex)
 , formatId(kNoIndex)
 , dirId(kNoIndex)
 , archiveId(kNoIndex)
 , offset(kNoIndexL)
 , index(kNoIndexL)
 , lockCount(0)
 , pData(NULL)
 , dataSize(0)
{
}

////////////////////////////////////////

cResourceManager::sResource::sResource(const sResource & other)
 : name(other.name)
 , extensionId(other.extensionId)
 , formatId(other.formatId)
 , dirId(other.dirId)
 , archiveId(other.archiveId)
 , offset(other.offset)
 , index(other.index)
 , lockCount(other.lockCount)
 , pData(other.pData)
 , dataSize(other.dataSize)
{
}

////////////////////////////////////////

cResourceManager::sResource::~sResource()
{
}

////////////////////////////////////////

const cResourceManager::sResource & cResourceManager::sResource::operator =(const sResource & other)
{
   Assert(other.pData == NULL);
   name = other.name;
   extensionId = other.extensionId;
   formatId = other.formatId;
   dirId = other.dirId;
   archiveId = other.archiveId;
   offset = other.offset;
   index = other.index;
   lockCount = 0;
   pData = NULL;
   dataSize = 0;
   return *this;
}


////////////////////////////////////////////////////////////////////////////////
