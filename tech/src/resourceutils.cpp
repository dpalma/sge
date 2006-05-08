////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "resourceutils.h"
#include "fileenum.h"
#include "filepath.h"
#include "filespec.h"

#ifdef HAVE_CPPUNITLITE2
#include "CppUnitLite2.h"
#endif

#include "dbgalloc.h" // must be last header

////////////////////////////////////////////////////////////////////////////////

LOG_EXTERN_CHANNEL(ResourceManager);

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

// {93BA1F78-3FF1-415b-BA5B-56FED039E838}
const GUID IID_IResourceManagerDiagnostics = 
{ 0x93ba1f78, 0x3ff1, 0x415b, { 0xba, 0x5b, 0x56, 0xfe, 0xd0, 0x39, 0xe8, 0x38 } };


////////////////////////////////////////////////////////////////////////////////

tStrings::size_type ListDirs(const cFilePath & path, bool bSkipHidden, tStrings * pDirs)
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


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceCacheKey
//

////////////////////////////////////////

cResourceCacheKey::cResourceCacheKey(const tChar * pszName, tResourceType type)
 : m_name(pszName)
 , m_type(type)
{
}

////////////////////////////////////////

cResourceCacheKey::cResourceCacheKey(const cResourceCacheKey & other)
 : m_name(other.m_name)
 , m_type(other.m_type)
{
}

////////////////////////////////////////

cResourceCacheKey::~cResourceCacheKey()
{
}

////////////////////////////////////////

const cResourceCacheKey & cResourceCacheKey::operator =(const cResourceCacheKey & other)
{
   m_name = other.m_name;
   m_type = other.m_type;
   return *this;
}

////////////////////////////////////////

bool cResourceCacheKey::operator ==(const cResourceCacheKey & other) const
{
   return (m_name.compare(other.m_name) == 0) && SameType(m_type, other.m_type);
}

////////////////////////////////////////

bool cResourceCacheKey::operator <(const cResourceCacheKey & other) const
{
   int c = m_name.compare(other.m_name);
   return (c < 0) || ((c == 0) && (strcmp(m_type, other.m_type) < 0));
}


///////////////////////////////////////////////////////////////////////////////
//
// STRUCT: sResource
//

////////////////////////////////////////

sResource::sResource()
 : name() 
 , extensionId(kNoIndex)
 , formatId(kNoIndex)
 , pStore(NULL)
 , pData(NULL)
 , dataSize(0)
{
}

////////////////////////////////////////

sResource::sResource(const sResource & other)
 : name(other.name)
 , extensionId(other.extensionId)
 , formatId(other.formatId)
 , pStore(other.pStore)
 , pData(other.pData)
 , dataSize(other.dataSize)
{
}

////////////////////////////////////////

sResource::~sResource()
{
}

////////////////////////////////////////

const sResource & sResource::operator =(const sResource & other)
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


////////////////////////////////////////////////////////////////////////////////
