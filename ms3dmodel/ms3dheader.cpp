///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ms3dheader.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dHeader
//

///////////////////////////////////////

const cMs3dHeader cMs3dHeader::gm_ms3dHeader("MS3D000000", 4);

///////////////////////////////////////

cMs3dHeader::cMs3dHeader()
 : m_version(0)
{
}

///////////////////////////////////////

cMs3dHeader::cMs3dHeader(const char id[10], int version)
 : m_version(version)
{
   memcpy(m_id, id, sizeof(m_id));
}

///////////////////////////////////////

cMs3dHeader::cMs3dHeader(const cMs3dHeader & other)
 : m_version(other.m_version)
{
   memcpy(m_id, other.m_id, sizeof(m_id));
}

///////////////////////////////////////

cMs3dHeader::~cMs3dHeader()
{
}

///////////////////////////////////////

const cMs3dHeader & cMs3dHeader::operator =(const cMs3dHeader & other)
{
   Assign(other);
   return *this;
}

///////////////////////////////////////

void cMs3dHeader::Assign(const cMs3dHeader & other)
{
   m_version = other.m_version;
   memcpy(m_id, other.m_id, sizeof(m_id));
}

///////////////////////////////////////

bool cMs3dHeader::EqualTo(const cMs3dHeader & other) const
{
   return m_version == other.m_version
      && memcmp(m_id, other.m_id, sizeof(m_id)) == 0;
}

///////////////////////////////////////

tResult cReadWriteOps<cMs3dHeader>::Read(IReader * pReader, cMs3dHeader * pHeader)
{
   Assert(pReader != NULL);
   Assert(pHeader != NULL);

   tResult result = E_FAIL;

   do
   {
      if (pReader->Read(pHeader->m_id, sizeof(pHeader->m_id)) != S_OK)
         break;

      if (pReader->Read(&pHeader->m_version, sizeof(pHeader->m_version)) != S_OK)
         break;

      result = S_OK;
   }
   while (0);

   return result;
}


///////////////////////////////////////////////////////////////////////////////
