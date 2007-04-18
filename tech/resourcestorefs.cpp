////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "resourcestore.h"

#include "tech/fileenum.h"
#include "tech/filepath.h"
#include "tech/filespec.h"
#include "tech/readwriteapi.h"
#include "tech/techstring.h"

#include "tech/dbgalloc.h" // must be last header

////////////////////////////////////////////////////////////////////////////////

LOG_EXTERN_CHANNEL(ResourceStore);

#define LocalMsg(msg)            DebugMsgEx(ResourceStore,msg)
#define LocalMsg1(msg,a)         DebugMsgEx1(ResourceStore,msg,(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(ResourceStore,msg,(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(ResourceStore,msg,(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)   DebugMsgEx4(ResourceStore,msg,(a),(b),(c),(d))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(ResourceStore,(cond),msg)
#define LocalMsgIf1(cond,msg,a)        DebugMsgIfEx1(ResourceStore,(cond),msg,(a))
#define LocalMsgIf2(cond,msg,a,b)      DebugMsgIfEx2(ResourceStore,(cond),msg,(a),(b))
#define LocalMsgIf3(cond,msg,a,b,c)    DebugMsgIfEx3(ResourceStore,(cond),msg,(a),(b),(c))
#define LocalMsgIf4(cond,msg,a,b,c,d)  DebugMsgIfEx4(ResourceStore,(cond),msg,(a),(b),(c),(d))


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cDirectoryResourceStore
//

class cDirectoryResourceStore : public cComObject<IMPLEMENTS(IResourceStore)>
{
public:
   cDirectoryResourceStore(const tChar * pszDir);
   virtual ~cDirectoryResourceStore();

   virtual tResult CollectResourceNames(const tChar * pszMatch, std::vector<cStr> * pNames);
   virtual tResult OpenEntry(const tChar * pszName, IReader * * ppReader);

private:
   cStr m_dir;
};

////////////////////////////////////////

cDirectoryResourceStore::cDirectoryResourceStore(const tChar * pszDir)
 : m_dir((pszDir != NULL) ? pszDir : _T(""))
{
}

////////////////////////////////////////

cDirectoryResourceStore::~cDirectoryResourceStore()
{
}

////////////////////////////////////////

tResult cDirectoryResourceStore::CollectResourceNames(const tChar * pszMatch, std::vector<cStr> * pNames)
{
   if (pszMatch == NULL || pNames == NULL)
   {
      return E_POINTER;
   }

   if (m_dir.empty())
   {
      return E_FAIL;
   }

   cFileSpec wildcard(pszMatch);
   wildcard.SetPath(cFilePath(m_dir.c_str()));

   cAutoIPtr<IEnumFiles> pEnumFiles;
   if (EnumFiles(wildcard, &pEnumFiles) == S_OK)
   {
      static const int kThisManyAtOnce = 10;
      cFileSpec files[kThisManyAtOnce];
      uint attribs[kThisManyAtOnce];
      ulong nFiles = 0;
      while (SUCCEEDED(pEnumFiles->Next(_countof(files), files, attribs, &nFiles)))
      {
         for (ulong i = 0; i < nFiles; i++)
         {
            if ((attribs[i] & kFA_Directory) == kFA_Directory)
            {
               LocalMsg1("Directory: %s\n", files[i].CStr());
            }
            else if ((attribs[i] & kFA_Hidden) == kFA_Hidden)
            {
               InfoMsg1("Skipping hidden file \"%s\"\n", files[i].CStr());
            }
            else
            {
               LocalMsg1("File: %s\n", files[i].CStr());
               pNames->push_back(files[i].CStr());
            }
         }
      }
   }

   return S_OK;
}

////////////////////////////////////////

tResult cDirectoryResourceStore::OpenEntry(const tChar * pszName, IReader * * ppReader)
{
   if (pszName == NULL || ppReader == NULL)
   {
      return E_POINTER;
   }

   cFileSpec file(pszName);
   file.SetPath(cFilePath(m_dir.c_str()));

   tResult result = E_FAIL;
   cAutoIPtr<IReader> pReader;
   if ((result = FileReaderCreate(file, kFileModeBinary, &pReader)) != S_OK)
   {
      return result;
   }

   *ppReader = CTAddRef(pReader);
   return S_OK;
}

////////////////////////////////////////

tResult ResourceStoreCreateFileSystem(const tChar * pszDir, IResourceStore * * ppStore)
{
   if (pszDir == NULL || ppStore == NULL)
   {
      return E_POINTER;
   }

   IResourceStore * pStore = static_cast<IResourceStore *>(new cDirectoryResourceStore(pszDir));
   if (pStore == NULL)
   {
      return E_OUTOFMEMORY;
   }

   *ppStore = pStore;
   return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
