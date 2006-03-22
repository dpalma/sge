////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "resourceformat.h"
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

LOG_DEFINE_CHANNEL(ResourceFormat);

#define LocalMsg(msg)            DebugMsgEx(ResourceFormat,msg)
#define LocalMsg1(msg,a)         DebugMsgEx1(ResourceFormat,msg,(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(ResourceFormat,msg,(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(ResourceFormat,msg,(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)   DebugMsgEx4(ResourceFormat,msg,(a),(b),(c),(d))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(ResourceFormat,(cond),msg)
#define LocalMsgIf1(cond,msg,a)        DebugMsgIfEx1(ResourceFormat,(cond),msg,(a))
#define LocalMsgIf2(cond,msg,a,b)      DebugMsgIfEx2(ResourceFormat,(cond),msg,(a),(b))
#define LocalMsgIf3(cond,msg,a,b,c)    DebugMsgIfEx3(ResourceFormat,(cond),msg,(a),(b),(c))
#define LocalMsgIf4(cond,msg,a,b,c,d)  DebugMsgIfEx4(ResourceFormat,(cond),msg,(a),(b),(c),(d))

////////////////////////////////////////////////////////////////////////////////

static const tChar kExtSep = _T('.');

static const uint kNoIndex = ~0u;


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


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceFormat
//

////////////////////////////////////////

void * cResourceFormat::Load(IReader * pReader) const
{
   return (pfnLoad != NULL) ? (*pfnLoad)(pReader) : NULL;
}

////////////////////////////////////////

void * cResourceFormat::Postload(void * pData, int dataLength, void * param) const
{
   return (pfnPostload != NULL) ? (*pfnPostload)(pData, dataLength, param) : pData;
}

////////////////////////////////////////

void cResourceFormat::Unload(void * pData) const
{
   (*pfnUnload)(pData);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceFormatTable
//

////////////////////////////////////////

cResourceFormatTable::cResourceFormatTable()
{
}

////////////////////////////////////////

cResourceFormatTable::~cResourceFormatTable()
{
}

////////////////////////////////////////

tResult cResourceFormatTable::RegisterFormat(tResourceType type,
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

      tResourceFormats::const_iterator iter = m_formats.begin();
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

   cResourceFormat format;
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

tResult cResourceFormatTable::RevokeFormat(tResourceType type, tResourceType typeDepend, const tChar * pszExtension)
{
   return E_NOTIMPL;
}

////////////////////////////////////////

uint cResourceFormatTable::DeduceFormats(const tChar * pszName, tResourceType type,
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
   // the format. Plus, include all formats that can generate the resource type from
   // a dependent type.
   if (extensionId != kNoIndex)
   {
      Assert(extensionId < m_extensions.size());
      tResourceFormats::const_iterator fIter = m_formats.begin();
      tResourceFormats::const_iterator fEnd = m_formats.end();
      for (uint index = 0; (fIter != fEnd) && (iFormat < nMaxFormats); fIter++, index++)
      {
         if (!SameType(fIter->type, type))
         {
            continue;
         }

         if ((fIter->extensionId == extensionId) || (fIter->extensionId == kNoIndex) || fIter->typeDepend)
         {
            pFormatIds[iFormat] = index;
            iFormat += 1;
         }
      }
   }

   // If no suitable formats found with the file extension, clear it so the block
   // below will try to with the given type alone.
   if (iFormat == 0)
   {
//      extensionId = kNoIndex;
   }

   // If no file extension, the resource type alone determines set of possible formats.
   if (extensionId == kNoIndex)
   {
      tResourceFormats::const_iterator fIter = m_formats.begin();
      tResourceFormats::const_iterator fEnd = m_formats.end();
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

uint cResourceFormatTable::GetFormatId(tResourceType type) const
{
   tResourceFormats::const_iterator iter = m_formats.begin();
   tResourceFormats::const_iterator end = m_formats.end();
   for (uint index = 0; iter != end; iter++, index++)
   {
      if (SameType(iter->type, type))
      {
         return index;
      }
   }
   return kNoIndex;
}

////////////////////////////////////////

void cResourceFormatTable::GetCompatibleFormats(tResourceType type, std::set<cStr> * pFormats) const
{
   pFormats->insert(type);

   tResourceFormats::const_iterator iter = m_formats.begin();
   for (; iter != m_formats.end(); iter++)
   {
      if ((pFormats->find(iter->type) != pFormats->end()) && iter->typeDepend)
      {
         pFormats->insert(iter->typeDepend);
      }
   }
}

////////////////////////////////////////

void cResourceFormatTable::GetExtensionsForType(tResourceType type, std::set<uint> * pExtensionIds) const
{
   tExtensionsForType::const_iterator iter = m_extsForType.lower_bound(type);
   tExtensionsForType::const_iterator end = m_extsForType.upper_bound(type);
   for (; iter != end; ++iter)
   {
      pExtensionIds->insert(iter->second);
   }
}

////////////////////////////////////////

uint cResourceFormatTable::GetExtensionId(const tChar * pszExtension)
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

uint cResourceFormatTable::GetExtensionIdForName(const tChar * pszName)
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

void cResourceFormatTable::DumpFormats() const
{
   techlog.Print(kInfo, "%d resource formats\n", m_formats.size());
   static const int kTypeWidth = -20;
   static const int kExtWidth = -5;
   static const tChar kRowFormat[] = "%*s | %*s | %*s\n";
   techlog.Print(kInfo, kRowFormat, kTypeWidth, "Type", kTypeWidth, "Dep. Type", kExtWidth, "Ext");
   techlog.Print(kInfo, "----------------------------------------------------------------------\n");
   tResourceFormats::const_iterator iter = m_formats.begin();
   tResourceFormats::const_iterator end = m_formats.end();
   for (uint index = 0; iter != end; iter++, index++)
   {
      techlog.Print(kInfo, kRowFormat,
         kTypeWidth, ResourceTypeName(iter->type),
         kTypeWidth, iter->typeDepend ? ResourceTypeName(iter->typeDepend) : "None",
         kExtWidth, iter->extensionId != kNoIndex ? m_extensions[iter->extensionId].c_str() : "None");
   }
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNITLITE2

static void * NopLoad(IReader * pReader)
{
   return NULL;
}

static void NopUnload(void * pData)
{
}

static void * NopPostload(void * pData, int dataLength, void * param)
{
   return pData;
}

TEST(ResourceFormatTableRegister)
{
   cResourceFormatTable rft;
   CHECK(rft.RegisterFormat(NULL, NULL, "dat", NopLoad, NULL, NopUnload) == E_INVALIDARG);
   CHECK(rft.RegisterFormat("data", NULL, "dat", NULL, NULL, NopUnload) == E_POINTER);
   CHECK(rft.RegisterFormat("data", NULL, "dat", NopLoad, NULL, NopUnload) == S_OK);
   CHECK(rft.RegisterFormat("data", NULL, "dat", NopLoad, NULL, NopUnload) == E_FAIL);
   CHECK(rft.RegisterFormat("bitmap", NULL, "bmp", NopLoad, NULL, NopUnload) == S_OK);
}

#endif // HAVE_CPPUNITLITE2

////////////////////////////////////////////////////////////////////////////////
