///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "readwriteapi.h"
#include "resourceapi.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

static char * GetContents(IReader * pReader)
{
   Assert(pReader != NULL);

   pReader->Seek(0, kSO_End);
   int length = pReader->Tell();
   pReader->Seek(0, kSO_Set);

   char * pszContents = new char[length + 1];

   if (pReader->Read(pszContents, length) != S_OK)
   {
      delete [] pszContents;
      return NULL;
   }

   pszContents[length] = 0;

   return pszContents;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTextResource
//

class cTextResource : public cComObject<IMPLEMENTS(IResource)>
{
public:
   cTextResource(IReader * pReader);

   virtual eResourceClass GetClass() const;

   virtual tResult GetData(void * * ppData);

private:
   cAutoIPtr<IReader> m_pReader;
};

////////////////////////////////////////

cTextResource::cTextResource(IReader * pReader)
 : m_pReader(CTAddRef(pReader))
{
}

////////////////////////////////////////

eResourceClass cTextResource::GetClass() const
{
   return kRC_Text;
}

////////////////////////////////////////

tResult cTextResource::GetData(void * * ppData)
{
   if (ppData == NULL)
   {
      return E_POINTER;
   }

   if (!m_pReader)
   {
      return E_FAIL;
   }

   // TODO: return something better than a pointer to allocated memory
   *ppData = GetContents(m_pReader);

   return (*ppData != NULL) ? S_OK : E_OUTOFMEMORY;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTextResourceFormat
//

class cTextResourceFormat : public cComObject<IMPLEMENTS(IResourceFormat)>
{
public:
   virtual tResult GetSupportedFileExtensions(std::vector<cStr> * pExtensions);

   virtual tResult Load(const tResKey & key, IReader * pReader, IResource * * ppResource);
};

////////////////////////////////////////

tResult cTextResourceFormat::GetSupportedFileExtensions(std::vector<cStr> * pExtensions)
{
   if (pExtensions == NULL)
   {
      return E_POINTER;
   }

   pExtensions->clear();
   pExtensions->push_back(cStr("txt"));
   pExtensions->push_back(cStr("lua"));
   pExtensions->push_back(cStr("xml"));

   return S_OK;
}

////////////////////////////////////////

tResult cTextResourceFormat::Load(const tResKey & key, IReader * pReader, IResource * * ppResource)
{
   if (pReader == NULL || ppResource == NULL)
   {
      return E_POINTER;
   }

   IResource * pResource = static_cast<IResource *>(new cTextResource(pReader));
   if (pResource == NULL)
   {
      return E_OUTOFMEMORY;
   }

   *ppResource = pResource;

   return S_OK;
}

////////////////////////////////////////

AUTOREGISTER_RESOURCEFORMAT(kRC_Text, cTextResourceFormat);

///////////////////////////////////////////////////////////////////////////////
