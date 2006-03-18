////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "resourcemanager.h"
#include "fileenum.h"
#include "filepath.h"
#include "filespec.h"
#include "readwriteapi.h"

#ifdef HAVE_CPPUNITLITE2
#include "CppUnitLite2.h"
#endif

#include <cstdio>
#include <vector>
#include <algorithm>
#include <set>

#include "dbgalloc.h" // must be last header

////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////

// REFERENCES
// "Game Developer Magazine", February 2005, "Inner Product" column

static const tChar kExtSep = _T('.');

static const uint kNoIndex = ~0u;
static const ulong kNoIndexL = ~0u;

// {93BA1F78-3FF1-415b-BA5B-56FED039E838}
const GUID IID_IResourceManagerDiagnostics = 
{ 0x93ba1f78, 0x3ff1, 0x415b, { 0xba, 0x5b, 0x56, 0xfe, 0xd0, 0x39, 0xe8, 0x38 } };


////////////////////////////////////////////////////////////////////////////////

typedef std::vector<cStr> tStrings;
static size_t ListDirs(const cFilePath & path, bool bSkipHidden, tStrings * pDirs)
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
      cFileSpec files[10];
      uint attribs[10];
      ulong nFiles = 0;
      while (SUCCEEDED(pEnumFiles->Next(_countof(files), files, attribs, &nFiles)))
      {
         for (ulong i = 0; i < nFiles; i++)
         {
            if ((attribs[i] & kFA_Directory) == kFA_Directory)
            {
               if (bSkipHidden && ((attribs[i] & kFA_Hidden) == kFA_Hidden))
               {
                  LocalMsg1("Skipping hidden directory \"%s\"\n", files[i].CStr());
                  continue;
               }
               pDirs->push_back(files[i].CStr());
            }
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
      return _tcscmp(lhs, rhs) == 0;
   }
   else if (lhs == NULL && rhs == NULL)
   {
      return true;
   }

   return false;
}

////////////////////////////////////////////////////////////////////////////////

inline const tChar * ResourceTypeName(tResourceType resourceType)
{
   return resourceType;
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

   {
      std::vector<cResourceStore *>::iterator iter = m_stores.begin();
      for (; iter != m_stores.end(); iter++)
      {
         delete *iter;
      }
      m_stores.clear();
   }

   return S_OK;
}

////////////////////////////////////////

tResult cResourceManager::AddDirectory(const tChar * pszDir)
{
   if (pszDir == NULL)
   {
      return E_POINTER;
   }

   cResourceStore * pStore = static_cast<cResourceStore *>(new cDirectoryResourceStore(pszDir));
   if (pStore == NULL)
   {
      return E_OUTOFMEMORY;
   }

   DebugMsg1("Adding directory store for \"%s\"\n", pszDir);
   
   return AddResourceStore(pStore);
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
   if (AddDirectory(root.CStr()) != S_OK)
   {
      return E_FAIL;
   }

   tStrings dirs;
   if (ListDirs(root, true, &dirs) > 0)
   {
      tStrings::const_iterator iter;
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
   if (pszArchive == NULL)
   {
      return E_POINTER;
   }

   cResourceStore * pStore = static_cast<cResourceStore *>(new cZipResourceStore(pszArchive));
   if (pStore == NULL)
   {
      return E_OUTOFMEMORY;
   }

   return AddResourceStore(pStore);
}

////////////////////////////////////////

tResult cResourceManager::AddResourceStore(cResourceStore * pStore)
{
   if (pStore == NULL)
   {
      return E_POINTER;
   }
   
   if (FAILED(pStore->FillCache(static_cast<cResourceCache*>(this))))
   {
      return E_FAIL;
   }

   m_stores.push_back(pStore);

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
   for (uint i = 0; i < nFormats; i++)
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

#ifdef _DEBUG
   static bDumpCache = false;
   if (bDumpCache)
   {
      DumpCache();
   }
#endif

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

      if (pRes->pStore != NULL)
      {
         cFileSpec file(pszName);
         file.SetFileExt(m_extensions[pRes->extensionId].c_str());

         cAutoIPtr<IReader> pReader;
         if (pRes->pStore->OpenEntry(file.CStr(), &pReader) == S_OK)
         {
            if (pReader->Seek(0, kSO_End) == S_OK
               && pReader->Tell(&dataSize) == S_OK
               && pReader->Seek(0, kSO_Set) == S_OK
               && DoLoadFromReader(pReader, pFormat, dataSize, param, &pData) == S_OK)
            {
               result = S_OK;
            }
         }
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

      m_extsForType.insert(std::make_pair(ResourceTypeName(type), extensionId));

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

tResult cResourceManager::ListResources(tResourceType type, std::vector<cStr> * pNames) const
{
   if (type == NULL)
   {
      return E_INVALIDARG;
   }

   if (pNames == NULL)
   {
      return E_POINTER;
   }

   std::set<cStr> formats;
   formats.insert(type);
   {
      tFormats::const_iterator iter = m_formats.begin();
      for (; iter != m_formats.end(); iter++)
      {
         if ((formats.find(iter->type) != formats.end()) && iter->typeDepend)
         {
            formats.insert(iter->typeDepend);
         }
      }
   }

   std::set<uint> formatIds;
   {
      tFormats::const_iterator iter = m_formats.begin();
      for (uint index = 0; iter != m_formats.end(); iter++, index++)
      {
         if (formats.find(iter->type) != formats.end())
         {
            formatIds.insert(index);
         }
      }
   }

   std::set<uint> extIds;
   {
      std::set<cStr>::const_iterator typeIter = formats.begin();
      for (; typeIter != formats.end(); typeIter++)
      {
         tExtsForType::const_iterator extIter = m_extsForType.lower_bound(*typeIter);
         tExtsForType::const_iterator extEnd = m_extsForType.upper_bound(*typeIter);
         for (; extIter != extEnd; ++extIter)
         {
            extIds.insert(extIter->second);
         }
      }
   }

   std::set<cStr> results;
   {
      tResources::const_iterator iter = m_resources.begin();
      for (; iter != m_resources.end(); iter++)
      {
         if (formatIds.find(iter->formatId) != formatIds.end()
            || extIds.find(iter->extensionId) != extIds.end())
         {
            cFileSpec name(iter->name.c_str());
            if (iter->extensionId != kNoIndex)
            {
               name.SetFileExt(m_extensions[iter->extensionId].c_str());
            }
            LocalMsg2("Resource '%s' is possibly of type '%s'\n", name.CStr(), type);
            results.insert(name.CStr());
         }
      }
   }

   {
      std::set<cStr>::const_iterator iter = results.begin();
      for (; iter != results.end(); iter++)
      {
         pNames->push_back(*iter);
      }
   }

   return results.empty() ? S_FALSE : S_OK;
}

////////////////////////////////////////

tResult cResourceManager::AddCacheEntry(const cResourceCacheEntryHeader & entry)
{
   cFileSpec file(entry.GetName());
   sResource res;
   Verify(file.GetFileNameNoExt(&res.name));
   const tChar * pszExt = file.GetFileExt();
   if (pszExt != NULL && _tcslen(pszExt) > 0)
   {
      res.extensionId = GetExtensionId(pszExt);
   }
   res.pStore = entry.GetStore();
   m_resources.push_back(res);
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

size_t cResourceManager::GetCacheSize() const
{
   return m_resources.size();
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

   {
      tResources::iterator iter = m_resources.begin();
      tResources::iterator end = m_resources.end();
      for (uint index = 0; iter != end; iter++, index++)
      {
         if (stricmp(iter->name.c_str(), name.c_str()) == 0)
         {
            LocalMsg4("Resource %d: base name \"%s\", extension %d is a potential match for \"%s\"\n",
               index, iter->name.c_str(), iter->extensionId, pszName);

            if (extensionId == kNoIndex
               && (extsPossible.find(iter->extensionId) != extsPossible.end()))
            {
               pPotentialMatch = &m_resources[index];
               nPotentialMatches++;
            }
            else if (extensionId == iter->extensionId)
            {
               if (iter->formatId == kNoIndex
                  && !m_formats[formatId].typeDepend
                  && iter->pStore != NULL)
               {
                  pPotentialMatch = &m_resources[index];
               }
               else if (iter->formatId == formatId)
               {
                  pPotentialMatch = &m_resources[index];
                  break;
               }
            }
         }
      }
   }

   LocalMsgIf3(nPotentialMatches > 0, "%d potential matches for %s, %s\n",
      nPotentialMatches, pszName, ResourceTypeName(type));

   return pPotentialMatch;
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

   // If the name has a file extension, the resource type plus extension determines
   // the format. Plus, include all formats that can generate the resource class from
   // a dependent type.
   if (extensionId != kNoIndex)
   {
      Assert(extensionId < m_extensions.size());
      tFormats::const_iterator fIter = m_formats.begin();
      tFormats::const_iterator fEnd = m_formats.end();
      for (uint index = 0; (fIter != fEnd) && (iFormat < nMaxFormats); fIter++, index++)
      {
         if (SameType(fIter->type, type))
         {
            if ((fIter->extensionId == extensionId) || (fIter->extensionId == kNoIndex) || fIter->typeDepend)
            {
               pFormatIds[iFormat] = index;
               iFormat += 1;
            }
         }
      }
   }

   // If no suitable formats found with the file extension, clear it so the block
   // below will try to with the given type alone.
   if (iFormat == 0)
   {
      extensionId = kNoIndex;
   }

   // If no file extension, the resource type alone determines set of possible formats.
   if (extensionId == kNoIndex)
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

tResult ResourceManagerCreate()
{
   cAutoIPtr<IResourceManager> p(new cResourceManager);
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IResourceManager, static_cast<IResourceManager*>(p));
}

////////////////////////////////////////

cResourceManager::sResource::sResource()
 : name() 
 , extensionId(kNoIndex)
 , formatId(kNoIndex)
 , pStore(NULL)
 , pData(NULL)
 , dataSize(0)
{
}

////////////////////////////////////////

cResourceManager::sResource::sResource(const sResource & other)
 : name(other.name)
 , extensionId(other.extensionId)
 , formatId(other.formatId)
 , pStore(other.pStore)
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
   pStore = other.pStore;
   pData = NULL;
   dataSize = 0;
   return *this;
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNITLITE2

typedef std::pair<cStr, cStr> tStrPair;

class cTestResourceStore : public cResourceStore
{
public:
   cTestResourceStore(const tStrPair * pTestData, size_t nTestData);
   virtual ~cTestResourceStore();

   virtual tResult FillCache(cResourceCache * pCache);
   virtual tResult OpenEntry(const tChar * pszName, IReader * * ppReader);

private:
   // Pairs of <file name, pseudo data>
   std::vector<tStrPair> m_testData;
};

cTestResourceStore::cTestResourceStore(const tStrPair * pTestData, size_t nTestData)
 : m_testData(nTestData)
{
   for (size_t i = 0; i < nTestData; i++, pTestData++)
   {
      m_testData[i] = *pTestData;
   }
}

cTestResourceStore::~cTestResourceStore()
{
}

tResult cTestResourceStore::FillCache(cResourceCache * pCache)
{
   std::vector<std::pair<cStr, cStr> >::const_iterator iter = m_testData.begin();
   for (; iter != m_testData.end(); iter++)
   {
      cResourceCacheEntryHeader header(iter->first.c_str(), static_cast<cResourceStore*>(this));
      pCache->AddCacheEntry(header);
   }
   return S_OK;
}

tResult cTestResourceStore::OpenEntry(const tChar * pszName, IReader * * ppReader)
{
   std::vector<std::pair<cStr, cStr> >::const_iterator iter = m_testData.begin();
   for (ulong index = 0; iter != m_testData.end(); iter++, index++)
   {
      if (stricmp(pszName, m_testData[index].first.c_str()) == 0)
      {
         cStr * pDataStr = &m_testData[index].second;
         return MemReaderCreate(reinterpret_cast<const byte *>(pDataStr->c_str()), pDataStr->length(), false, ppReader);
      }
   }
   return E_FAIL;
}

////////////////////////////////////////

class cResourceManagerTests
{
public:
   cResourceManagerTests(const tStrPair * pTestData, size_t nTestData);
   ~cResourceManagerTests();

   cAutoIPtr<cResourceManager> m_pResourceManager;
   cAutoIPtr<IResourceManagerDiagnostics> m_pDiagnostics;
};

////////////////////////////////////////

cResourceManagerTests::cResourceManagerTests(const tStrPair * pTestData, size_t nTestData)
{
   SafeRelease(m_pResourceManager);
   m_pResourceManager = new cResourceManager;
   m_pResourceManager->Init();

   cAutoIPtr<IResourceManagerDiagnostics> pResMgrDiag;
   Verify(m_pResourceManager->QueryInterface(IID_IResourceManagerDiagnostics, (void**)&m_pDiagnostics) == S_OK);

   if ((pTestData != NULL) && (nTestData > 0))
   {
      cTestResourceStore * pStore = new cTestResourceStore(pTestData, nTestData);
      m_pResourceManager->AddResourceStore(static_cast<cResourceStore*>(pStore));
   }
}

////////////////////////////////////////

cResourceManagerTests::~cResourceManagerTests()
{
   if (!!m_pResourceManager)
   {
      m_pResourceManager->Term();
      SafeRelease(m_pResourceManager);
   }
}

////////////////////////////////////////

void * TestDataLoad(IReader * pReader)
{
   ulong dataSize = 0;
   if (pReader != NULL
      && pReader->Seek(0, kSO_End) == S_OK
      && pReader->Tell(&dataSize) == S_OK
      && pReader->Seek(0, kSO_Set) == S_OK)
   {
      byte * pData = new byte[dataSize];
      if (pData != NULL)
      {
         if (pReader->Read(pData, dataSize) != S_OK)
         {
            delete [] pData;
            return NULL;
         }
         else
         {
            return pData;
         }
      }
   }
   return NULL;
}

void TestDataUnload(void * pData)
{
   delete [] (byte *)pData;
}

void * ReverseTestDataPostload(void * pData, int dataLength, void * param)
{
   if (dataLength == 0)
   {
      dataLength = strlen(static_cast<char*>(pData));
   }
   byte * pBytes = static_cast<byte*>(pData);
   for (int i = 0; i < (dataLength / 2); i++)
   {
      byte temp = pBytes[i];
      pBytes[i] = pBytes[dataLength - i - 1];
      pBytes[dataLength - i - 1] = temp;
   }
   return pBytes;
}

////////////////////////////////////////

#define kRT_Data _T("data")
#define kRT_ReverseData _T("RevData")
#define kRT_Bitmap _T("bitmap")

const tStrPair g_basicTestResources[] =
{
   std::make_pair(cStr("foo.dat"), cStr("foo_dat_foo_dat_foo_dat_foo_dat\0")),
   std::make_pair(cStr("foo.bmp"), cStr("foo_bmp_foo_bmp_foo_bmp_foo_bmp\0")),
   std::make_pair(cStr("bar.dat"), cStr("bar_dat_bar_dat_bar_dat_bar_dat\0")),
};

////////////////////////////////////////

TEST_FP(cResourceManagerTests,
        cResourceManagerTests(&g_basicTestResources[0], _countof(g_basicTestResources)),
        ResourceManagerLoadSameNameDifferentType)
{
   CHECK(m_pResourceManager->RegisterFormat(kRT_Data, NULL, "dat", TestDataLoad, NULL, TestDataUnload) == S_OK);
   CHECK(m_pResourceManager->RegisterFormat(kRT_Bitmap, NULL, "bmp", TestDataLoad, NULL, TestDataUnload) == S_OK);

   size_t cacheSizeBefore = m_pDiagnostics->GetCacheSize();

   {
      byte * pFooDat = NULL;
      CHECK(m_pResourceManager->Load("foo", kRT_Data, (void*)NULL, (void**)&pFooDat) == S_OK);
      CHECK(memcmp(pFooDat, g_basicTestResources[0].second.c_str(), g_basicTestResources[0].second.length()) == 0);
   }

   {
      byte * pFooBmp = NULL;
      CHECK(m_pResourceManager->Load("foo", kRT_Bitmap, (void*)NULL, (void**)&pFooBmp) == S_OK);
      CHECK(memcmp(pFooBmp, g_basicTestResources[1].second.c_str(), g_basicTestResources[1].second.length()) == 0);
   }

   {
      byte * pFooDat2 = NULL;
      CHECK(m_pResourceManager->Load("foo", kRT_Data, (void*)NULL, (void**)&pFooDat2) == S_OK);
      CHECK(memcmp(pFooDat2, g_basicTestResources[0].second.c_str(), g_basicTestResources[0].second.length()) == 0);
   }

   CHECK_EQUAL(cacheSizeBefore, m_pDiagnostics->GetCacheSize());
}

////////////////////////////////////////

#if 0
TEST_FP(cResourceManagerTests,
        cResourceManagerTests(&g_basicTestResources[0], _countof(g_basicTestResources)),
        ResourceManagerDerivedType)
{
   CHECK(m_pResourceManager->RegisterFormat(kRT_Data, NULL, "dat", TestDataLoad, NULL, TestDataUnload) == S_OK);
   CHECK(m_pResourceManager->RegisterFormat(kRT_ReverseData, kRT_Data, NULL, NULL, ReverseTestDataPostload, TestDataUnload) == S_OK);

   size_t cacheSizeBefore = m_pDiagnostics->GetCacheSize();

   // TODO: test loading derived types

   CHECK_EQUAL(cacheSizeBefore, m_pDiagnostics->GetCacheSize());
}
#endif

////////////////////////////////////////

TEST_FP(cResourceManagerTests,
        cResourceManagerTests(&g_basicTestResources[0], _countof(g_basicTestResources)),
        ResourceManagerLoadCaseSensitivity)
{
   CHECK(m_pResourceManager->RegisterFormat(kRT_Data, NULL, "dat", TestDataLoad, NULL, TestDataUnload) == S_OK);

   {
      byte * pFooDat1 = NULL;
      CHECK(m_pResourceManager->Load("foo", kRT_Data, (void*)NULL, (void**)&pFooDat1) == S_OK);
      CHECK(memcmp(pFooDat1, g_basicTestResources[0].second.c_str(), g_basicTestResources[0].second.length()) == 0);
   }

   {
      byte * pFooDat2 = NULL;
      CHECK(m_pResourceManager->Load("FOO", kRT_Data, (void*)NULL, (void**)&pFooDat2) == S_OK);
      CHECK(memcmp(pFooDat2, g_basicTestResources[0].second.c_str(), g_basicTestResources[0].second.length()) == 0);
   }
}

////////////////////////////////////////

TEST_FP(cResourceManagerTests,
        cResourceManagerTests(&g_basicTestResources[0], _countof(g_basicTestResources)),
        ResourceManagerListResources)
{
   CHECK(m_pResourceManager->RegisterFormat(kRT_Data, NULL, "dat", TestDataLoad, NULL, TestDataUnload) == S_OK);
   CHECK(m_pResourceManager->RegisterFormat(kRT_Bitmap, NULL, "bmp", TestDataLoad, NULL, TestDataUnload) == S_OK);

   {
      std::vector<cStr> dataResNames;
      CHECK(m_pResourceManager->ListResources(kRT_Data, &dataResNames) == S_OK);
      CHECK_EQUAL(dataResNames.size(), 2);
   }

   {
      std::vector<cStr> bitmapResNames;
      CHECK(m_pResourceManager->ListResources(kRT_Bitmap, &bitmapResNames) == S_OK);
      CHECK_EQUAL(bitmapResNames.size(), 1);
   }

   {
      std::vector<cStr> allResNames;
      CHECK(m_pResourceManager->ListResources(kRT_Data, &allResNames) == S_OK);
      CHECK(m_pResourceManager->ListResources(kRT_Bitmap, &allResNames) == S_OK);
      CHECK_EQUAL(allResNames.size(), 3);
   }
}

////////////////////////////////////////

TEST_FP(cResourceManagerTests,
        cResourceManagerTests(NULL, 0),
        ResourceManagerRegisterFormat)
{
   CHECK(m_pResourceManager->RegisterFormat(NULL, NULL, "dat", TestDataLoad, NULL, TestDataUnload) == E_INVALIDARG);
   CHECK(m_pResourceManager->RegisterFormat(kRT_Data, NULL, "dat", NULL, NULL, TestDataUnload) == E_POINTER);
   CHECK(m_pResourceManager->RegisterFormat(kRT_Data, NULL, "dat", TestDataLoad, NULL, TestDataUnload) == S_OK);
   CHECK(m_pResourceManager->RegisterFormat(kRT_Data, NULL, "dat", TestDataLoad, NULL, TestDataUnload) == E_FAIL);
   CHECK(m_pResourceManager->RegisterFormat(kRT_Bitmap, NULL, "bmp", TestDataLoad, NULL, TestDataUnload) == S_OK);
}

////////////////////////////////////////

// This unit test is for a case that the resource manager doesn't actually handle yet
#if 0
const tStrPair g_multExtTestResources[] =
{
   std::make_pair(cStr("foo.ms3d"), cStr("foo_ms3d_foo_ms3d_foo_ms3d_foo_ms3d\0")),
   std::make_pair(cStr("foo.ms3d.xml"), cStr("foo_ms3d_xml_foo_ms3d_xml_foo_ms3d_xml\0")),
};

TEST_FP(cResourceManagerTests,
        cResourceManagerTests(&g_multExtTestResources[0], _countof(g_multExtTestResources)),
        ResourceManagerMultipleExtensionConfusion)
{
   CHECK(m_pResourceManager->RegisterFormat("fooms3d", NULL, "ms3d", TestDataLoad, TestDataPostload, TestDataUnload) == S_OK);
   CHECK(m_pResourceManager->RegisterFormat("fooxml", NULL, "xml", TestDataLoad, TestDataPostload, TestDataUnload) == S_OK);

   {
      byte * pFooXml = NULL;
      CHECK(m_pResourceManager->Load("foo.ms3d", "fooxml", (void*)NULL, (void**)&pFooXml) == S_OK);
      if (pFooXml != NULL)
      {
         const cStr & expected = g_multExtTestResources[1].second;
         CHECK(memcmp(pFooXml, expected.c_str(), expected.length()) == 0);
      }
   }
}
#endif

#endif // HAVE_CPPUNITLITE2

////////////////////////////////////////////////////////////////////////////////
